#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"

using namespace aui;

int main() {
	//char *qqq = new char[1]; // generate error
	// delay of 3 is minimum for this test on my PC, otherwise X11 crashes with BadWindow error
	// IMHO it depends on how messages are sent and if our message for closure gets in earlier than standard
	// XLib messages for a window the crash occurs
  UINT32 delay_ms = 50; // delay before thead calls window to close
  AUI* au = AUI::Create("label");
  AWindow* w = au->MainWnd();
  ALabel* lb = ALabel::AttachTo(w, "test");
  lb->Resize(250, 20);
  *lb = "test2";
  *lb += "_1";

  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    au->ExitAUI();
  });

  au->ProcessMessages();

  handle.get();
  
  delete au;
  au = nullptr;
  lb = nullptr;

  return 0;
}


