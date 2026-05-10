#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"

bool need_delay_exit = 1;

using namespace aui;

INT32 GeneralTest(ATable *ta) {
  AUICellData di;
  ta->Clear();
  ta->Resize(400, 250);
  ta->AddColumn();
  ta->AddColumn();
  ta->AddRow();
  ta->AddRows(5);
  ta->AddColumns(5);
  if(ta->Rows() != 6) E("number of rows is wrong{}", ta->Rows())
  if(ta->Columns() != 7) E("number of columnts is wrong {}", ta->Rows())
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
  return 0;
}

INT32 AutowidenTest(ATable *ta) {
  AUICellData di;
	ta->Clear();
  ta->AddColumns(5);
  ta->AddRows(5);
  ta->SetColumnWidth(0, 1);
  ta->SetAutoWiden(true);
  di.data = "some decently long string";
  ta->Insert(0, 0, &di);
  if(ta->GetColumnWidth(0) > 10) {
    D("Column Autowiden test passed({})\n", ta->GetColumnWidth(0))
    return 0;
  }
	D("Column autowiden test failed")
	return 1;
}

int main() {
	//char *qqq = new char[1]; // generate error
  UINT32 delay_ms = 50; // delay before thead calls window to close
  AUI* au = AUI::Create("table");
  AWindow* w = au->MainWnd();
  ATable* ta = ATable::AttachTo(w);
  INT32 testsfailed = 0;
  
  testsfailed += GeneralTest(ta);
  testsfailed += AutowidenTest(ta);
  
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
  ta = 0;
  
  return testsfailed;
}


