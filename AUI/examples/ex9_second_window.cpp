#include <cassert>
#include <AUILib.h>

using namespace aui;
using namespace std::chrono;

void StopTimer(time_point<high_resolution_clock> start) {
  time_point<high_resolution_clock> end = high_resolution_clock::now();
  duration<double, std::milli> duration_ms1 = end - start;
  D("time: %f ms", duration_ms1.count());
}

int main() {
  D3("starting main()")
  time_point<high_resolution_clock> start = high_resolution_clock::now();
  AUI* au = AUI::Create("window example (main window)");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(540, 380);
  w->DisableResize();
	UNUSED AWindow* wS = AWindow::AttachTo(au, "Open process");
  UNUSED ALabel* lb = ALabel::AttachTo(wS, "Close main window while not ");
  lb->Move(10, 10);
  lb->Resize(300, 20);
  UNUSED ALabel* lb2 = ALabel::AttachTo(wS, "closing this one to test");
  lb2->Move(10, 30);
  lb2->Resize(300, 20);
  UNUSED ALabel* lb3 = ALabel::AttachTo(wS, "correct resource deallocation");
  lb3->Move(10, 50);
  lb3->Resize(300, 20);
  UNUSED ALabel* lb4 = ALabel::AttachTo(wS, "(run with Valgrind)");
  lb4->Move(10, 70);
  lb4->Resize(300, 20);

  StopTimer(start);
  
  au->ProcessMessages();
  
  delete au;
  au  = nullptr;
  lb = nullptr;
  lb2 = nullptr;
  lb3 = nullptr;

  return 0;
}
