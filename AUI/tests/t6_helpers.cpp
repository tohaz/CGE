#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"

bool need_delay_exit = 1;

using namespace aui;

int main() {
	//char *qqq = new char[1]; // generate error
  UINT32 delay_ms = 50; // delay before thead calls window to close
  AUI* au = AUI::Create("helpers");
//  AWindow* w = au->MainWnd();
  
  UNUSED UINT16 ui16 = SafeUINT16((UINT32)1);

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


