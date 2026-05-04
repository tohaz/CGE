#include <cassert>
#include <AUILib.h>

using namespace aui;
using namespace std::chrono;

void StopTimer(time_point<high_resolution_clock> start) {
  time_point<high_resolution_clock> end = high_resolution_clock::now();
  duration<double, std::milli> duration_ms1 = end - start; // @suppress("Invalid arguments")
  D("time: %f ms", duration_ms1.count());
}

void ButtonProcessesHandler(UNUSED XEvent* ev, AWidget* w, UNUSED void* d) {
  D()
  AUI* au = w->AUIPtr();
  AWindow* wProcess = AWindow::AttachTo(au, "Open process");
  wProcess->Move(1000, 500);
  wProcess->EnableResize();
  wProcess->Resize(500, 450);
  wProcess->DisableResize();
}

int main() {
  time_point<high_resolution_clock> start = high_resolution_clock::now();
  AUI* cg = AUI::Create("cg0 main");
  AWindow* w = cg->MainWnd();
  w->EnableResize();
  w->Resize(540, 380);
  w->DisableResize();
  w->PrintDimensions();

//  AButton* bProc = AButton::AttachTo(w, "Processes");
//  bProc->ResizeX(120);
//  bProc->SetOnButtonReleaseCB(ButtonProcessesHandler, w);

//  ATable* ta = ATable::AttachTo(w);
//  for(int i = 0; i < 5; i++) ta->AddRow();
//  for(int i = 0; i < 5; i++) ta->AddColumn();
//  ta->AddRows(10);
//  ta->AddColumns(10);
//  AUICellData di;
//  di.data = "sta";
//  ta->Insert(0, 0, &di);
//  di.data = "ZZZ";
//  ta->Insert(0, 3, &di);
//  di.data = "TTTTT";
//  ta->Insert(0, 4, &di);
//  di.data = "HHH";
//  ta->Insert(0, 6, &di);
//  di.data = "123";
//  ta->Insert(1, 1, &di);
//  di.data = "aaa";
//  ta->Insert(2, 2, &di);
//  di.data = "qqq";
//  ta->Insert(2, 0, &di);


  StopTimer(start);
  cg->ProcessMessages();
  delete cg;
  cg = 0;

  return 0;
}


//AInputBox* ib = AInputBox::AttachTo(w, "");


