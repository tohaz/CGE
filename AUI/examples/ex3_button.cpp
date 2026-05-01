#include "AUILib.h"

using namespace aui;

void ButtonCloseHandler(XEvent* ev, AWidget* w, void* d) {
  D("user callback fired congrats %lu %lu", (UINT64)ev, (UINT64)d)
	w->AUIPtr()->ExitAUI();
}

int main() {

	AUI* au = AUI::Create("example3");
	AWindow* w = au->MainWnd();

	AButton* bn = AButton::AttachTo(w, "Close");
  bn->SetOnButtonReleaseCB(ButtonCloseHandler, nullptr);
	au->ProcessMessages();

  delete au;
 	w = 0;
	bn = 0;
  au = 0;
  return 0;
}


