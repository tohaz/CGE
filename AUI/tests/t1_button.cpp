#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"

using namespace aui;

void ButtonCloseHandler(XEvent* ev, AWidget* w, void* d) {
  D("user quit callback fired, bye world %lu %lu", (UINT64)ev, (UINT64)d)
  w->AUIPtr()->ExitAUI();
}

void ButtonTestHandler(XEvent* ev, AWidget* w, void* d) {
  D("user callback fired, congrats %lu %lu %lu", (UINT64)ev, (UINT64)w, (UINT64)d)
}

int main() {
	//char *qqq = new char[1]; // generate error
  UINT32 delay_ms = 50; // delay before thead calls window to close
  AUI* au = AUI::Create("button");
  AWindow* w = au->MainWnd();
  
  AButton* bn = AButton::AttachTo(w, "Close");
  bn->SetOnButtonReleaseCB(ButtonCloseHandler, nullptr);

  AButton* bnt = AButton::AttachTo(w, "Test");
  bnt->Move(100, 10);
  bnt->Resize(100, 30);
  bnt->SetOnButtonReleaseCB(ButtonTestHandler, nullptr);

  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
  	au->ExitAUI();
  });

  au->ProcessMessages();
  
  handle.get();

  delete au;
  au = nullptr;
  bn = nullptr;
  bnt = nullptr;

  return 0;
}


