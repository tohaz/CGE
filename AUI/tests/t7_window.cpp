#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"

bool need_delay_exit = 1;

using namespace aui;

void ButtonProcessesHandler(UNUSED XEvent* ev, UNUSED AWidget* w, UNUSED void* d) {
	D()
}

int main() {
	//char *qqq = new char[1]; // generate error
  UINT32 delay_ms = 0; // delay before thead calls window to close
  AUI* au = AUI::Create("helpers");
  UNUSED AWindow* w = au->MainWnd();
  // Test 1. Resource allocation - freeing for ATable in child window
  UNUSED AWindow* wProcess = AWindow::AttachTo(au, "Open process");
  AButton* bOpenProc = AButton::AttachTo(w, "Processes");
  bOpenProc->Resize(100, 20);
  bOpenProc->SetOnButtonReleaseCB(ButtonProcessesHandler, w);
  wProcess->Move(1000, 500);
  wProcess->EnableResize();
  wProcess->Resize(640, 450);
  wProcess->DisableResize();
  UNUSED ATable* ta = ATable::AttachTo(wProcess);
  // Test 1 end.

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

  delete au;
  au = nullptr;

  return 0;
}


