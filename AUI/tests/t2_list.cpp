#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"

using namespace aui;

int main() {
	//char *qqq = new char[1]; // generate error
  UINT32 delay_ms = 50; // delay before thead calls window to close
  AUI* au = AUI::Create("listview");
  AWindow* w = au->MainWnd();

  AList* lv = AList::AttachTo(w);

  lv->Resize(300, 250);
  lv->EnableScrollbars();

  lv->AddItem("1ABCDEFGHIJKLMNOPQRSTUVWXYX");
  lv->AddItem("2wabcdefdhijklmnopqrstuvwxyz");

  std::string z;
  for(UINT32 i = 0; i < 10000; i++) {
    z = std::to_string(i + 13) + "zzz";
    lv->AddItem(z.c_str());
  }

  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    au->ExitAUI();
  });

  au->ProcessMessages();

  delete au;
	lv = 0;
  au = 0;

  handle.get();
  return 0;
}


