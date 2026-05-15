#include <chrono>
#include <future>
#include <thread>
#include <memory.h>
#include <iostream>
#include <vector>

#include "AUILib.h"
#include "AWindow.h"
#include "AButton.h"

using namespace aui;

// Global tracking flags for the regression simulation
static bool g_BackgroundButtonTriggered = false;
// Mock callback handler registered to the underlying backdrop button element
static void MockBackgroundButtonHandler(UNUSED XEvent* ev, UNUSED AWidget* w, UNUSED void* data) {
  g_BackgroundButtonTriggered = true; 
}
// System close handler that will break the blocking ProcessMessages loop
void SystemCloseHandler(UNUSED XEvent* ev, AWidget* w, UNUSED void* d) {
  w->AUIPtr()->ExitAUI();
}
// Test Suite: Validates that top-level floating windows insulate background widgets from coordinate theft
INT32 TestChildWindowClickShielding(AWidget* parent) {
  AUI* ui = parent->AUIPtr();
  Display* d = ui->Disp();
  g_BackgroundButtonTriggered = false; 

  D("TEST TRACE: Step 1 - Registering underlying background button inside main window...")
  AButton* backgroundBtn = AButton::AttachTo(parent, "Underground");
  if (!backgroundBtn) [[unlikely]] return 1;
  backgroundBtn->Move(50, 50);
  backgroundBtn->Resize(100, 40);
  backgroundBtn->SetOnButtonPressCB(MockBackgroundButtonHandler, nullptr);

  D("TEST TRACE: Step 2 - Detecting absolute screen coordinates of the main window...")
  XFlush(d);
  int mainAbsX = 0;
  int mainAbsY = 0;
  Window dummyWin;
  XTranslateCoordinates(d, parent->Wnd(), XRootWindow(d, 0), 0, 0, &mainAbsX, &mainAbsY, &dummyWin);
  if (mainAbsX == 0 && mainAbsY == 0) {
      mainAbsX = 100;
      mainAbsY = 100;
  }
  D("TEST TRACE: Step 3 - Creating a FREE FLOATING top-level window directly via AUI pointer...")
  // Exact replication of your production bug environment layout code
  AWindow* wProcess = AWindow::AttachTo(ui, "Open process");
  if (!wProcess) [[unlikely]] {
    ui->RemoveWidget(backgroundBtn->Wnd());
    return 1;
  }
  // Position the floating window precisely over the main window's button coordinates on screen
  int targetX = mainAbsX + 40;
  int targetY = mainAbsY + 40;
  wProcess->Move(static_cast<UINT32>(targetX), static_cast<UINT32>(targetY));
  wProcess->EnableResize();
  wProcess->Resize(150, 100);
  XMapWindow(d, wProcess->Wnd());
  XSelectInput(d, wProcess->Wnd(), ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
  XFlush(d);
  // Give Valgrind and Window Manager enough time to map the new top-level window client state
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  D("TEST TRACE: Step 4 - Formulating mock ButtonPress event payload structure...")
  XEvent mockClick;
  memset(&mockClick, 0, sizeof(mockClick));
  mockClick.type = ButtonPress;
  mockClick.xbutton.button = Button1;
  mockClick.xbutton.window = wProcess->Wnd(); // Target is natively the floating window
  mockClick.xbutton.x_root = mainAbsX + 70;
  mockClick.xbutton.y_root = mainAbsY + 70;
  mockClick.xbutton.x = 30; 
  mockClick.xbutton.y = 30;
  mockClick.xbutton.same_screen = True;
  D("TEST TRACE: Step 5 - Dispatching events directly into the X-Server pipeline queue...")
  XSendEvent(d, wProcess->Wnd(), False, ButtonPressMask, &mockClick);
  XEvent closeClick;
  memset(&closeClick, 0, sizeof(closeClick));
  closeClick.type = ClientMessage;
  closeClick.xclient.window = parent->Wnd();
  closeClick.xclient.message_type = XInternAtom(d, "WM_DELETE_WINDOW", False);
  closeClick.xclient.format = 32;
  XSendEvent(d, parent->Wnd(), False, NoEventMask, &closeClick);
  XFlush(d);
  return 0;
}

int main() {
  INT32 testsfailed = 0;
  AUI* au = AUI::Create("AWindow click shielding test suite");

  AWindow* mainFrame = au->MainWnd();

//this is a complex test below involving highend hackery and changing code of ProcessMessages to trigger the
// false result. So all other tests shoud be above this comment
  AButton* exitBtn = AButton::AttachTo(mainFrame, "Close");
  exitBtn->SetOnButtonReleaseCB(SystemCloseHandler, nullptr);
  TestChildWindowClickShielding(mainFrame);
  // Real framework dispatch execution loop
  au->ProcessMessages();

  if (g_BackgroundButtonTriggered) {
    D("test failed: Background button intercepted click event through the floating AWindow!")
    testsfailed = 1; 
  } else {
    D("test passed: Click isolation shielding validated perfectly. Floating window insulated the background.")
  }

  delete au;
  au = nullptr;
  exitBtn = nullptr;

  return testsfailed; 
}
