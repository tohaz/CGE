#include "AUILib.h"

using namespace aui;

int main() {

	AUI* au = AUI::Create("example2");
	AWindow* w = au->MainWnd();
	AButton* bn = AButton::AttachTo(w, "hello world");
	bn->Resize(200, 20);
  au->ProcessMessages();

  delete au;
	w = 0;
  au = 0;

  return 0;
}


