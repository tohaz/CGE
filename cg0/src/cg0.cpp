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
  D1("init time: %f ms", duration_ms1.count());
}

void UpdateProcTable(ATable *ta, UNUSED std::string filter) {
  D("filter:%s", filter.c_str())
  INT64 row = 0;
  UNUSED AUICellData cell;
  ta->Clear();
  ta->DisableRowHeader();
  ta->AddColumns(2);
//  ta->AddRows(SafeUINT32((UINT64)pr.Size()));
  ta->SetColumnName(0, "PID");
  ta->SetColumnName(1, "Path");
  ta->SetColumnWidth(0, 75);
  ta->SetColumnWidth(1, 423);
  ProcessDescr *pd;
  for (UNUSED const auto& [id, value] : pr) {
    pd = value;
    if(pd->Path().contains(filter) || pd->PidStr().contains(filter)) {
      ta->AddRow();
      cell.data = pd->PidStr();
      cell.hAlign = AUIHAlign::center;
      ta->Insert(row, 0, &cell);
      cell.data = pd->Path();
      cell.hAlign = AUIHAlign::left;
      ta->Insert(row++, 1, &cell);
    }
  }
}

void InputBoxValueChangedHandler(UNUSED AWidget* w, void* d) {
  ATable* ta = (ATable*) d;
  UNUSED AInputBox* ib = (AInputBox*) w;
  UpdateProcTable(ta, ib->Text());
  ta->Draw();
}

void ButtonProcessesHandler(UNUSED XEvent* ev, AWidget* w, UNUSED void* d) {
  D3()
  AUI* au = w->AUIPtr();
  UNUSED AWindow* wProcess = AWindow::AttachTo(au, "Open process");
  wProcess->Move(1000, 500);
  wProcess->EnableResize();
  wProcess->Resize(640, 450);
  wProcess->DisableResize();
  ATable* ta = ATable::AttachTo(wProcess);
  ta->Resize(500, 400);
  ta->Move(10, 40);
  UNUSED AButton* bProcOK = AButton::AttachTo(wProcess, "Choose");
  bProcOK->Resize(80, 20);
  bProcOK->Move(530, 10);
  UNUSED AInputBox* ib = AInputBox::AttachTo(wProcess, "");
  ib->SetTitle("Search");
  ib->Move(10, 10);
  ib->SetOnValueChangedCB(InputBoxValueChangedHandler, ta);
  UpdateProcTable(ta, "");
}

int main() {
  time_point<high_resolution_clock> start = high_resolution_clock::now();
  UNUSED AUI* cg = AUI::Create("cg0 main");
  AWindow* w = cg->MainWnd();
  AButton* bOpenProc = AButton::AttachTo(w, "Processes");
  bOpenProc->Resize(100, 20);
  bOpenProc->SetOnButtonReleaseCB(ButtonProcessesHandler, w);
  //ButtonProcessesHandler(0, w, 0);
  StopTimer(start);
  cg->ProcessMessages();
  delete cg;
  cg = 0;

  return 0;
}


