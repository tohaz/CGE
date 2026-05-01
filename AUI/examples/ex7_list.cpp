#include <AUILib.h>

using namespace aui;

void InputSubmitHandler(AWidget* w, void* d) {
  D("input submit handler")
  ALabel* l = (ALabel*) d;
  l->SetText(w->Text());
  l->Draw();
}

int main() {
  D3("starting main()")
  AUI* au = AUI::Create("ex7 list");
  AWindow* w = au->MainWnd();
  
  AList* lv = AList::AttachTo(w);

  lv->Resize(300, 250);
  lv->EnableScrollbars();

  lv->AddItem("1ABCDEFGHIJKLMNOPQRSTUVWXYX");
  lv->AddItem("2wabcdefdhijklmnopqrstuvwxyz");

  std::string z;
  for(UINT32 i = 0; i < 100; i++) {
    z = std::to_string(i + 13) + "zzz";
    lv->AddItem(z.c_str());
  }

  au->ProcessMessages();
  delete au;
  lv = 0;
  au = 0;

  return 0;
}
