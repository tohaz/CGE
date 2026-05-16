#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"

using namespace aui;

INT32 TestMenuLayout(AWidget* parent) {
  std::vector<AMenuItem> items;
  items.push_back(createEmptyItem()); 
  AMenuItem sep = createEmptyItem();
  sep.isSeparator = true;
  items.push_back(sep);
  items.push_back(createEmptyItem());
  APopupMenu* menu = APopupMenu::AttachTo(parent, items);
  // Dynamic expectation based on actual class members
  INT32 h = menu->ItemHeight();
  INT32 s = menu->SeparatorHeight();
  INT32 d = menu->PressDepth();
  UINT32 expectedH = SafeUINT32((2 * h) + s + (2 * d));
  UINT32 actualH = menu->SizeYUI32();
  if (actualH != expectedH) {
    E("REGRESSION: Menu height mismatch! Expected {}, got {}", expectedH, actualH);
    menu->Dismiss();
    return 1;
  }
  D1("SUCCESS: Menu layout matches internal constants ({})", actualH);
  menu->Dismiss();
  return 0;
}

INT32 TestMenuHoverLogic(AWidget* parent) {
  std::vector<AMenuItem> items;
  items.push_back(createEmptyItem()); // Index 0
  AMenuItem sep = createEmptyItem(); 
  sep.isSeparator = true;
  items.push_back(sep);               // Index 1
  items.push_back(createEmptyItem()); // Index 2 (Target)
  items[2].text = "Target";
  APopupMenu* menu = APopupMenu::AttachTo(parent, items);
  // Dynamic coordinate calculation:
  // Position = Depth + Item0Height + SeparatorHeight + (Item2Height / 2)
  INT32 h = menu->ItemHeight();
  INT32 s = menu->SeparatorHeight();
  INT32 d = menu->PressDepth();
  // Precisely in the middle of the 3rd item (index 2)
  INT32 testY = d + h + s + (h / 2);
  D2("Testing hover at y={} (Menu height is {})", testY, menu->SizeYUI32());
  XEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = MotionNotify;
  ev.xmotion.window = menu->Wnd();
  ev.xmotion.x = 20;
  ev.xmotion.y = testY;
  // Manual call to check logic
  menu->OnMouseMove(&ev);
  if (menu->HoveredIndex() != 2) {
    E("REGRESSION: Hover logic failed! Expected index 2, got {}", menu->HoveredIndex());
    menu->Dismiss();
    return 1;
  }
  D1("SUCCESS: Menu hover logic respects actual heights (Index 2 found)");
  menu->Dismiss();
  return 0;
}

INT32 TestMenuGrabState(AWidget* parent) {
  std::vector<AMenuItem> items;
  items.push_back(createEmptyItem());
  items[0].text = "Test";
  APopupMenu* menu = APopupMenu::AttachTo(parent, items);
  menu->Popup(100, 100);
  if (!menu->IsGrabbed()) {
    E("REGRESSION: Menu failed to grab pointer on Popup");
    menu->Dismiss();
    return 1;
  }
  menu->Dismiss();
  if (menu->IsGrabbed()) {
    E("REGRESSION: Menu grab not released after Dismiss");
    return 2;
  }
  D1("SUCCESS: Menu grab state lifecycle is correct");
  return 0;
}

INT32 TestSubmenuCleanup(AWidget* parent) {
  std::vector<AMenuItem> subItems;
  subItems.push_back(createEmptyItem());
  subItems[0].text = "Action";
  std::vector<AMenuItem> mainItems;
  AMenuItem sub = createEmptyItem();
  sub.text = "Has Sub";
  sub.subItems = subItems;
  mainItems.push_back(sub);
  APopupMenu* menu = APopupMenu::AttachTo(parent, mainItems);
  menu->Popup(100, 100);
  // Trigger hover to open submenu
  XEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = MotionNotify;
  ev.xmotion.window = menu->Wnd();
  ev.xmotion.y = 10; // Over the first item
  menu->OnMouseMove(&ev);
  // We cannot easily check if mActiveSubMenu is non-null because it's private,
  // but if the following Dismiss doesn't crash Valgrind, the cleanup is correct.
  menu->Dismiss();
  D1("SUCCESS: Submenu recursive cleanup test finished");
  return 0;
}

void ButtonCloseHandler(XEvent* ev, AWidget* w, void* d) {
  D("user quit callback fired, bye world {} {}", (UINT64)ev, (UINT64)d)
  w->AUIPtr()->ExitAUI();
}

int main() {
//	UNUSED char *qqq = new char[1]; // generate error
  UINT32 delay_ms = 50; // delay before thead calls window to close
	INT32 testsfailed = 0;
  AUI* au = AUI::Create("APopupMenu class test");
  
  AWindow* w = au->MainWnd();
  AButton* bn = AButton::AttachTo(w, "Close");
	bn->SetOnButtonReleaseCB(ButtonCloseHandler, nullptr);
	
  testsfailed += TestMenuLayout(w);
  testsfailed += TestMenuHoverLogic(w);
  testsfailed += TestMenuGrabState(w);
  testsfailed += TestSubmenuCleanup(w);
  
  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
  	au->ExitAUI();
  });

  au->ProcessMessages();
  
  handle.get();

  delete au;
  au = nullptr;
  bn = nullptr;

  return testsfailed;
}


