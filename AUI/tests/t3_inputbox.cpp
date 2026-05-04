#include <chrono>
#include <future>
#include <thread>
#include <memory.h>
#include <X11/Xlib.h> // Required for XInitThreads
#include "AUILib.h"

using namespace aui;

bool need_delay_exit = 0;

int main() {
  UINT32 delay_ms = 50; 
  AUI* au = AUI::Create("inputbox");
  AWindow* w = au->MainWnd();

  AInputBox* ib = AInputBox::AttachTo(w, (char *)"testing");
  ib->Resize(100, 25);

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
  
  return 0;
}


