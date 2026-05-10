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
  w->EnableResize();
  w->Resize(390, 220);
  w->DisableResize();

  ALabel* lb = ALabel::AttachTo(w, "Helloworld, but with an input box.");
  lb->Resize(350, 20);
  lb->Move(15, 50);

  ALabel* lbo1 = ALabel::AttachTo(w, "1. Hover mouse to be above the box and click on it.");
  lbo1->Resize(350, 20);
  lbo1->Move(15, 75);
  lbo1->SetHAlign(AUIHAlign::left);

  ALabel* lbo2 = ALabel::AttachTo(w, "and click on it.");
  lbo2->Resize(350, 20);
  lbo2->Move(15, 100);
  lbo2->SetHAlign(AUIHAlign::left);

  ALabel* lbo3 = ALabel::AttachTo(w, "2. Input text and press Enter");
  lbo3->Resize(350, 20);
  lbo3->Move(15, 125);
  lbo3->SetHAlign(AUIHAlign::left);

  AInputBox* ib = AInputBox::AttachTo(w, (char*)"");
  ib->SetTitle((char *) "Edit Me");
  ib->SetOnSubmitCB(InputSubmitHandler, lbo3);

  ALabel* lbo4 = ALabel::AttachTo(w, "PS. This is a new style inputbox VVV");
  lbo4->Resize(350, 20);
  lbo4->Move(15, 155);
  lbo4->SetHAlign(AUIHAlign::left);

  AInputBox* ib2 = AInputBox::AttachTo(w, (char*)"test");
  ib2->SetTitle((char *) "Edit Me2");
  ib2->Move(290, 180);
  ib2->SetStyle(AUIWidgetStyle::Simple3D);

  au->ProcessMessages();
  delete au;
  lb = 0;
  lbo1 = 0;
  lbo2 = 0;
  ib = 0;
  au = 0;

  return 0;
}
