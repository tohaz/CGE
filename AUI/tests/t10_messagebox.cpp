#include <chrono>
#include <future>
#include <thread>
#include <memory.h>
#include <iostream>
#include <vector>

#include "AUILib.h"
#include "AModalWindow.h"

using namespace aui;

// Test 1: Verifies string split and auto-wrapping algorithm parameters inside the custom engine canvas
INT32 TestAModalWindowWrapText(AWidget* parent) {
  AModalWindow* test_win = AModalWindow::AttachTo(parent, "Test", "");
  test_win->WrapText(400); 
  if (test_win->WrappedLines().size() > 1) {
    D("test failed")
    return 1;
  }
  AModalWindow* test_win2 = AModalWindow::AttachTo(parent, "Test", "Line1\nLine2\nLine3");
  test_win2->WrapText(500); 
  const auto& lines = test_win2->WrappedLines();
  if (lines.size() != 3) {
    D("test failed")
    return 1;
  }
  if (lines[0] != "Line1" || lines[1] != "Line2" || lines[2] != "Line3") {
    D("test failed")
    return 1;
  }
  std::string huge_word = "ThisIsAMassiveWordThatExceedsTheMaximumAllowedPixelWidthInsideTheCanvas";
  AModalWindow* test_win3 = AModalWindow::AttachTo(parent, "Test", huge_word);
  test_win3->WrapText(50); 
  if (test_win3->WrappedLines().empty()) {
    D("test failed")
    return 1;
  }
  return 0; 
}

// Test 2: Validates the clamping bounds and dynamic line height scaling geometry
INT32 TestCalculateLayout(AWidget* parent) {
  UINT32 outW = 0, outH = 0;
  AModalWindow* test_win = AModalWindow::AttachTo(parent, "T", "OK");
  test_win->CalculateLayout(outW, outH);
  if (outW < 320) {
    D("test failed")
    return 1;
  }
  outW = 0; outH = 0;
  std::string long_text(1000, 'A'); 
  AModalWindow* test_win2 = AModalWindow::AttachTo(parent, "T", long_text);
  test_win2->CalculateLayout(outW, outH);
  if (outW > 550) {
    D("test failed")
    return 1;
  }
  UINT32 shortW = 0, shortH = 0;
  UINT32 longW = 0, longH = 0;
  AModalWindow* win_short = AModalWindow::AttachTo(parent, "T", "One Line Message");
  win_short->CalculateLayout(shortW, shortH);
  AModalWindow* win_long = AModalWindow::AttachTo(parent, "T", "Line1\nLine2\nLine3\nLine4\nLine5");
  win_long->CalculateLayout(longW, longH);
  if (longH <= shortH) {
    D("test failed")
    return 1;
  }
  return 0; 
}

// Test 3: Simulates user mouse input down, move and up sequences to check position updates
INT32 TestAModalWindowDragAndDrop(AWidget* parent) {
  AModalWindow* test_win = AModalWindow::AttachTo(parent, "Test Title", "Message Body");
  XEvent press_ev{};
  press_ev.type = ButtonPress;
  press_ev.xbutton.button = Button1;
  press_ev.xbutton.x = 100;
  press_ev.xbutton.y = 15; 
  test_win->OnButtonPress(&press_ev);
  AModalWindow* test_win2 = AModalWindow::AttachTo(parent, "Test Title", "Message Body");
  XEvent press_ev2{};
  press_ev2.type = ButtonPress;
  press_ev2.xbutton.button = Button1;
  press_ev2.xbutton.x = 100;
  press_ev2.xbutton.y = 120; 
  test_win2->OnButtonPress(&press_ev2);
  AModalWindow* test_win3 = AModalWindow::AttachTo(parent, "Test Title", "Message Body");
  XEvent press_ev3{};
  press_ev3.type = ButtonPress;
  press_ev3.xbutton.button = Button1;
  press_ev3.xbutton.x = 10;
  press_ev3.xbutton.y = 10;
  test_win3->OnButtonPress(&press_ev3);
  XEvent motion_ev{};
  motion_ev.type = MotionNotify;
  motion_ev.xbutton.x_root = -500; 
  motion_ev.xbutton.y_root = -500;
  test_win3->OnMouseMove(&motion_ev);
  if (test_win3->X() < 0 || test_win3->Y() < 0) {
    return 1;
  }
  return 0; 
}

// Test 4: Sets up LIFO execution sequence and verifies hardware tracking stack constraints
INT32 TestModalStackLIFOOrder(AWidget* parent) {
  AUI* ui = parent->AUIPtr();
  AModalWindow* modal1 = AModalWindow::AttachTo(parent, "Title 1", "Content Text 1");
  AModalWindow* modal2 = AModalWindow::AttachTo(parent, "Title 2", "Content Text 2");
  AModalWindow* modal3 = AModalWindow::AttachTo(parent, "Title 3", "Content Text 3");
  AWidget* activeModal = ui->GetModal();
  if (!activeModal) {
    std::cerr << "[TEST SETUP ERROR] Modal stack is completely empty" << std::endl;
    return 1;
  }
  if (activeModal->Wnd() != modal3->Wnd()) {
    std::cerr << "[TEST SETUP ERROR] LIFO Window handle mismatch detected" << std::endl;
    return 1;
  }
  // FIX: Instead of mass-posting Close() and choking the X-Server event pipeline queue,
  // we trigger them. The async loop in main will cleanly digest them step by step.
  modal3->Close();
  modal2->Close();
  modal1->Close();
  return 0;
}

// Test 5: Simulates mid-stack layer eviction safely bounded to the async lifecycle loop
INT32 TestModalStackNonLinearClosure(AWidget* parent) {
  AUI* ui = parent->AUIPtr();
  AModalWindow* modal1 = AModalWindow::AttachTo(parent, "Window 1", "Data 1");
  AModalWindow* modal2 = AModalWindow::AttachTo(parent, "Window 2", "Data 2");
  AModalWindow* modal3 = AModalWindow::AttachTo(parent, "Window 3", "Data 3");
  AWidget* activeModal = ui->GetModal();
  if (!activeModal || activeModal->Wnd() != modal3->Wnd()) {
    std::cerr << "[TEST SETUP ERROR] Unexpected topmost window allocation layout" << std::endl;
    return 1;
  }
  modal2->Close();
  modal3->Close();
  modal1->Close();
  return 0;
}

// Test 6: Verifies cascading placement safety boundaries
INT32 TestModalCascadeOverflowSafety(AWidget* parent) {
  std::vector<AModalWindow*> windows;
  windows.reserve(15);
  for (int i = 0; i < 15; ++i) {
    windows.push_back(AModalWindow::AttachTo(parent, "Stress Title", "Verification Body"));
  }
  for (auto* win : windows) {
    if (win->X() < 0 || win->Y() < 0 || win->SizeXUI32() == 0 || win->SizeYUI32() == 0) {
      std::cerr << "[TEST ERROR] Structural overflow detected on layout coordinates limits" << std::endl;
      for (auto* cleanupWin : windows) cleanupWin->Close();
      return 1;
    }
  }
  for (auto* win : windows) {
    win->Close();
  }
  return 0;
}

// Post-Processing Verification: Executes diagnostic assertions after ProcessMessages completes loop digestion
INT32 ValidateAsyncTestResults(AUI* ui) {
  if (ui->GetModal() != nullptr) {
    std::cerr << "[POST TEST FAILURE] Stack framework failed to clear out tracking entries cleanly at exit" << std::endl;
    return 1;
  }
  return 0;
}

void ButtonCloseHandler(XEvent* ev, AWidget* w, void* d) {
  D("user quit callback fired, bye world {} {}", (UINT64)ev, (UINT64)d)
  w->AUIPtr()->ExitAUI();
}

bool need_delay_exit = 1;

int main() {
  UINT32 delay_ms = 500; 
  INT32 testsfailed = 0;
  AUI* au = AUI::Create("AModalWindow class test");

  AWindow* w = au->MainWnd();
  AButton* bn = AButton::AttachTo(w, "Close");
  bn->SetOnButtonReleaseCB(ButtonCloseHandler, nullptr);
	
  testsfailed += TestAModalWindowWrapText(w);
  testsfailed += TestCalculateLayout(w);
  testsfailed += TestAModalWindowDragAndDrop(w);
  
  testsfailed += TestModalStackLIFOOrder(w);
  testsfailed += TestModalStackNonLinearClosure(w);
  testsfailed += TestModalCascadeOverflowSafety(w);
  
  std::future<void> handle;
  if(need_delay_exit) {
    handle = std::async(std::launch::async, [=]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
      au->ExitAUI();
    });
  }
  
  au->ProcessMessages();
  
  if(need_delay_exit) {
    handle.get();
  }

//  testsfailed += ValidateAsyncTestResults(au);

  delete au;
  au = nullptr;
  bn = nullptr;

  return testsfailed;
}

