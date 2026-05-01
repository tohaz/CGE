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
  AUI* cg = AUI::Create("cg0 main");
  AWindow* w = cg->MainWnd();
  w->EnableResize();
  w->Resize(540, 380);
  w->DisableResize();


//  ATable* ta = ATable::AttachTo(w);
  AInputBox* ib = AInputBox::AttachTo(w, "edit me");

//  D1("table size [%lu, %lu]", ta->Rows(), ta->Columns())

  StopTimer(start);
  cg->ProcessMessages();
  delete cg;
  cg = 0;

  return 0;
}
