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
  AUI* au = AUI::Create("ex6");
  AWindow* w = au->MainWnd();

  ALabel* lb = ALabel::AttachTo(w, "Helloworld, but with input box.");
  lb->Resize(350, 20);
  lb->Move(15, 50);

  ALabel* lbo1 = ALabel::AttachTo(w, "1. Hover mouse to be above the box.");
  lbo1->Resize(350, 20);
  lbo1->Move(15, 80);
  lbo1->SetHAlign(AUIHAlign::left);

  ALabel* lbo2 = ALabel::AttachTo(w, "2. Edit text and press Enter");
  lbo2->Resize(350, 20);
  lbo2->Move(15, 110);
  lbo2->SetHAlign(AUIHAlign::left);

  AInputBox* ib = AInputBox::AttachTo(w, "edit me");
  ib->SetOnSubmitCB(InputSubmitHandler, lbo2);

  au->ProcessMessages();
  delete au;
  lb = 0;
  lbo1 = 0;
  lbo2 = 0;
  ib = 0;
  au = 0;

  return 0;
}
