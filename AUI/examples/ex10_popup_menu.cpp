#include "AUILib.h"
#include "APopupMenu.h"

using namespace aui;

// Helper to create an empty item to satisfy strict compiler
AMenuItem createEmptyItem() {
  return AMenuItem{
    .text = "",
    .hotkey = "",
    .action = nullptr,
    .subItems = {},
    .isSeparator = false,
    .isCheckable = false,
    .isChecked = false,
    .isEnabled = true,
    .icon = None
  };
}

std::vector<AMenuItem> createExampleItems(AUI* au) {
  std::vector<AMenuItem> items;
  // Item 1
  AMenuItem hello = createEmptyItem();
  hello.text = "Hello World";
  hello.action = []() { D("Menu Action: Hello clicked!"); };
  items.push_back(hello);

  // Submenu
  std::vector<AMenuItem> subItems;
  AMenuItem sub1 = createEmptyItem();
  sub1.text = "Sub Item 1";
  sub1.action = []() { D("Sub 1"); };
  subItems.push_back(sub1);

  AMenuItem tools = createEmptyItem();
  tools.text = "Tools";
  tools.subItems = subItems;
  items.push_back(tools);
  // Separator
  AMenuItem sep = createEmptyItem();
  sep.isSeparator = true;
  items.push_back(sep);
  // Exit
  AMenuItem quit = createEmptyItem();
  quit.text = "Exit Application";
  quit.action = [au]() { au->ExitAUI(); };
  items.push_back(quit);

  return items;
}

void MenuButtonHandler(XEvent* ev, [[maybe_unused]] AWidget* w, void* data) {
  APopupMenu* menu = static_cast<APopupMenu*>(data);
  menu->Popup(ev->xbutton.x_root, ev->xbutton.y_root);
}

void WindowRightClickHandler(XEvent* ev, [[maybe_unused]] AWidget* w, void* data) {
  if (ev->xbutton.button == Button3) { // Right Click
    APopupMenu* menu = static_cast<APopupMenu*>(data);
    menu->Popup(ev->xbutton.x_root, ev->xbutton.y_root);
  }
}

int main() {
  AUI* au = AUI::Create("Menu Example");
  AWindow* w = au->MainWnd();

  std::vector<AMenuItem> items = createExampleItems(au);
  APopupMenu* mainMenu = APopupMenu::AttachTo(w, items);

  AButton* btnMenu = AButton::AttachTo(w, "Open Menu");
  btnMenu->SetXY(50, 50);
  btnMenu->SetOnButtonReleaseCB(MenuButtonHandler, mainMenu);
  // Window now needs to handle ButtonPress to catch right-click
  w->SetOnButtonPressCB(WindowRightClickHandler, mainMenu);

  au->ProcessMessages();

  delete au;
  return 0;
}

