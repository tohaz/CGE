#include <cassert>
#include <AUILib.h>

using namespace aui;
using namespace std::chrono;

void StopTimer(time_point<high_resolution_clock> start) {
  time_point<high_resolution_clock> end = high_resolution_clock::now();
  duration<double, std::milli> duration_ms1 = end - start;
  D("time: %f ms", duration_ms1.count());
}

void AddRowHandler(UNUSED XEvent* ev, UNUSED AWidget* w, void* d) {
  D1("Add Row");
  ATable* t = (ATable*)d;
  t->AddRow();
  t->Draw();
}

void RemoveRowHandler(UNUSED XEvent* ev, UNUSED AWidget* w, void* d) {
  D1("Remove Row");
  ATable* t = (ATable*)d;
  t->RemoveLastRow();
}

void AddColumnHandler(UNUSED XEvent* ev, UNUSED AWidget* w, void* d) {
  D1("Add Column");
  ATable* t = (ATable*)d;
  t->AddColumn();
  t->Draw();
}

void RemoveColumnHandler(UNUSED XEvent* ev, UNUSED AWidget* w, void* d) {
  D1("Remove Column");
  ATable* t = (ATable*)d;
  t->RemoveLastColumn();
}

void MoveUpHandler(UNUSED XEvent* ev, UNUSED AWidget* w, void* d) {
  D2("Up");
  ATable* t = (ATable*)d;
  t->ScrollUpPx(9);
}

void MoveDownHandler(UNUSED XEvent* ev, UNUSED AWidget* w, void* d) {
  D2("Down");
  ATable* t = (ATable*)d;
  t->ScrollDownPx(9);
}

void MoveLeftHandler(UNUSED XEvent* ev, UNUSED AWidget* w, void* d) {
  D2("Left");
  ATable* t = (ATable*)d;
  t->ScrollLeftPx(9);
}

void MoveRightHandler(UNUSED XEvent* ev, UNUSED AWidget* w, void* d) {
  D2("Right");
  ATable* t = (ATable*)d;
  t->ScrollRightPx(9);
}

int main() {
  D3("starting main()")
  time_point<high_resolution_clock> start = high_resolution_clock::now();
  AUI* cg = AUI::Create("cg0 main");
  AWindow* w = cg->MainWnd();
  w->EnableResize();
  w->Resize(540, 380);
  w->DisableResize();

  ATable* ta = ATable::AttachTo(w);
  ta->Resize(300,340);
  for(int i = 0; i < 50; i++) ta->AddRow();
  for(int i = 0; i < 50; i++) ta->AddColumn();
  AUICellData di;
  di.hal = AUIHAlign::center;
  di.val = AUIVAlign::center;
  di.data = "sta";
  ta->Insert(0, 0, &di);
  di.data = "ZZZ";
  ta->Insert(0, 3, &di);
  di.data = "TTTTT";
  ta->Insert(0, 4, &di);
  di.data = "HHH";
  ta->Insert(0, 6, &di);
  di.data = "123";
  ta->Insert(1, 1, &di);
  di.data = "aaa";
  ta->Insert(2, 2, &di);
  di.data = "qqq";
  ta->Insert(2, 0, &di);

  AButton *bRC = AButton::AttachTo(w, "Remv. Col");
  bRC->Move(340, 20);
  bRC->SetOnButtonReleaseCB(RemoveColumnHandler, ta);
  AButton *bAC = AButton::AttachTo(w, "Add Col");
  bAC->Move(430, 20);
  bAC->SetOnButtonReleaseCB(AddColumnHandler, ta);
  AButton *bRR = AButton::AttachTo(w, "Remv. Row");
  bRR->Move(340, 70);
  bRR->SetOnButtonReleaseCB(RemoveRowHandler, ta);
  AButton *bAR = AButton::AttachTo(w, "Add Row");
  bAR->Move(340, 110);
  bAR->SetOnButtonReleaseCB(AddRowHandler, ta);

  AButton *bL = AButton::AttachTo(w, "Left");
  bL->SetOnButtonReleaseCB(MoveLeftHandler, ta);
  bL->Move(330, 250);
  AButton *bR = AButton::AttachTo(w, "Right");
  bR->SetOnButtonReleaseCB(MoveRightHandler, ta);
  bR->Move(420, 250);
  AButton *bU = AButton::AttachTo(w, "Up");
  bU->SetOnButtonReleaseCB(MoveUpHandler, ta);
  bU->Move(373, 210);
  AButton *bD = AButton::AttachTo(w, "Down");
  bD->SetOnButtonReleaseCB(MoveDownHandler, ta);
  bD->Move(373, 290);

  D1("table size [%lu, %lu]", ta->Rows(), ta->Columns())

  StopTimer(start);
  cg->ProcessMessages();
  delete cg;
  cg = 0;

  return 0;
}
