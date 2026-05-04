#include <cassert>
#include <AUILib.h>

#include "ProcessList.h"

using namespace aui;
using namespace cg;
using namespace std::chrono;

ProcessList pr;

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
  wProcess->Resize(640, 450);
  wProcess->DisableResize();
  ATable* ta = ATable::AttachTo(wProcess);
  ta->AddRows(20);
  ta->AddColumns(20);
  ta->DisableRowHeader();
  ta->Resize(500, 400);
  ta->Move(10, 40);
  AButton* bProcOK = AButton::AttachTo(wProcess, "Choose");
  bProcOK->Resize(80, 20);
  bProcOK->Move(530, 10);
//  bProc->SetOnButtonReleaseCB(ButtonProcessesHandler, w);
  AInputBox* ib = AInputBox::AttachTo(wProcess, "123");
  ib->Move(10, 10);
//  ib->X();
//    ib->SetOnSubmitCB(InputSubmitHandler, lbo2);

}

int main() {
  time_point<high_resolution_clock> start = high_resolution_clock::now();
  AUI* cg = AUI::Create("cg0 main");
  AWindow* w = cg->MainWnd();
  w->EnableResize();
  w->Resize(540, 380);
  w->DisableResize();
  w->PrintDimensions();

  AButton* bOpenProc = AButton::AttachTo(w, "Processes");
  bOpenProc->Resize(100, 20);
  bOpenProc->SetOnButtonReleaseCB(ButtonProcessesHandler, w);


  StopTimer(start);
  cg->ProcessMessages();
  delete cg;
  cg = 0;

  return 0;
}


//AInputBox* ib = AInputBox::AttachTo(w, "");


