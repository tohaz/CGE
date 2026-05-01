#include "AUILib.h"

using namespace aui;

void ButtonCloseHandler(XEvent* ev, AWidget* w, void* d) {
  D("user quit callback fired, bye world %lu %lu", (UINT64)ev, (UINT64)d)
	w->AUIPtr()->ExitAUI();
}

void ButtonTestHandler(XEvent* ev, AWidget* w, void* d) {
  D("user callback fired, congrats %lu %lu %lu", (UINT64)ev, (UINT64)w, (UINT64)d)
}

int main() {
  AUI* au = AUI::Create("example4");
  AWindow* w = au->MainWnd();

  AButton* bn = AButton::AttachTo(w, "Close");
  bn->SetOnButtonReleaseCB(ButtonCloseHandler, nullptr);

  AButton* bnt = AButton::AttachTo(w, "Test");
	bnt->Move(100, 10);
  bnt->SetOnButtonReleaseCB(ButtonTestHandler, nullptr);

  au->ProcessMessages();

  D1("destruction phase")
 	delete au;
	w = 0; 
  bn = 0;
  bnt = 0;
  au = 0;
  return 0;
}

