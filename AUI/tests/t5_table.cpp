#include <chrono>
#include <future>
#include <thread>
#include <memory.h>
//
#include "AUILib.h"

using namespace aui;

int main() {
	//char *qqq = new char[1]; // generate error
  UINT32 delay_ms = 50; // delay before thead calls window to close
  AUI* au = AUI::Create("table");
  AWindow* w = au->MainWnd();
  
  ATable* ta = ATable::AttachTo(w);
  ta->AddRow();
  for(int i = 0; i < 10; i++) {
    ta->AddColumn();
  }
  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    au->ExitAUI();
  });

  au->ProcessMessages();

  delete au;
  handle.get();

  return 0;
}


