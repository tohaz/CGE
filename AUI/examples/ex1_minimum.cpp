#include "AUILib.h"

using namespace aui;

int main() {
  AUI* au = AUI::Create("example1");
  au->ProcessMessages();

  delete au;
  return 0;
}


