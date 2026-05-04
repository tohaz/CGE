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
  AUI* au = AUI::Create("table");
  AWindow* w = au->MainWnd();

  ATable* ta = ATable::AttachTo(w);
  
  ta->AddRow();


  for(int i = 0; i < 10; i++) {
    ta->AddColumn();
  }

  ta->AddRows(20);

  ta->AddColumns(30);
  
  AUICellData di;
  di.data = "sta";
  ta->Insert(0, 0, &di);
  di.data = "ZZZ";
  ta->Insert(0, 3, &di);
  di.data = "TTTTT";
  ta->Insert(0, 4, &di);
  di.data = "HHH";
  ta->Insert(0, 6, &di);
  di.data = "123";
  ta->Insert(1, 1, &di);
  di.data = "aaa";
  ta->Insert(2, 2, &di);
  di.data = "qqq";
  ta->Insert(2, 0, &di);


  
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
  ta = 0;

  return 0;
}


