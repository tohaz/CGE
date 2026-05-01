#include <AUILib.h>

using namespace aui;

int main() {
  AUI* au = AUI::Create("ex5");
  AWindow* w = au->MainWnd();

  ALabel* lb = ALabel::AttachTo(w, "Helloworld, but with label");
  lb->Resize(250, 20);

  au->ProcessMessages();
  delete au;
  lb = 0;
  au = 0;
  return 0;
}
