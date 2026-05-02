#include <chrono>
#include <future>
#include <thread>
#include <memory.h>
#include <X11/Xlib.h> // Required for XInitThreads
#include "AUILib.h"

using namespace aui;

int main() {
  // 1. CRITICAL: Initialize X11 thread support first
  XInitThreads();

  UINT32 delay_ms = 50; 
  AUI* au = AUI::Create("inputbox");
  AWindow* w = au->MainWnd();

  AInputBox* ib = AInputBox::AttachTo(w, "testing");
  ib->Resize(100, 25);

  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    // Check if au is still valid or use a signal 
    au->ExitAUI();
  });

  au->ProcessMessages();

  // 2. Wait for the thread to finish BEFORE deleting the AUI object
  handle.wait(); 

  // 3. Now it is safe to delete
  delete au;
  
  return 0;
}


