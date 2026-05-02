#include <chrono>
#include <future>
#include <thread>
#include <memory.h>
//
#include "AUILib.h"

bool need_delay_exit = 1;

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

  ta->AddRows(20);

  ta->AddColumns(30);
  
  ta->Move(0, 0);
  
  ta->Resize(400, 250);
    
  std::future<void> handle;
  if(need_delay_exit) {
    handle = std::async(std::launch::async, [=]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
      au->ExitAUI();
    });
  }

  au->ProcessMessages();

  delete au;
  
  if(need_delay_exit) {
    handle.get();
  }
  
  au = 0;

  return 0;
}


