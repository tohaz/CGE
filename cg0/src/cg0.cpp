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
  D1("init time: {} ms", duration_ms1.count());
}

void UpdateProcTable(ATable *ta, UNUSED std::string filter) {
  D("filter:{}", filter.c_str())
  INT64 row = 0;
  UNUSED AUICellData cell;
  ta->Clear();
  ta->DisableRowHeader();
  ta->AddColumns(2);
  ta->SetColumnName(0, "PID");
  ta->SetColumnName(1, "Path");
  ta->SetColumnWidth(0, 75);
  ta->SetColumnWidth(1, 423);
  ProcessDescr *pd;
  for (const auto& [id, value] : pr) {
    pd = value;
    if(pd->Path().contains(filter) || pd->PidStr().contains(filter)) {
      INT64 realIdx = ta->AddRow();
      D("Inserting row: realIdx={}, manualRow={}", realIdx, row);
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

void ButtonSelectHandler(UNUSED XEvent* ev, AWidget* w, UNUSED void* d) {
  D1()
  AButton* b = (AButton*) w;
  ATable* ta = (ATable*)d;
  std::string zzz = ta->CursorData();
  D1("cursor data '{}'", zzz.c_str())
  b->ParentWidget()->Close();
}

void ButtonProcessesHandler(UNUSED XEvent* ev, AWidget* w, UNUSED void* d) {
  D3()
  AUI* au = w->AUIPtr();
  AWindow* wProcess = AWindow::AttachTo(au, "Open process");
  wProcess->Move(1000, 500);
  wProcess->EnableResize();
  wProcess->Resize(640, 450);
  wProcess->DisableResize();
  ATable* ta = ATable::AttachTo(wProcess);
  ta->Resize(500, 400);
  ta->Move(10, 40);
  AButton* bSelect = AButton::AttachTo(wProcess, "Select");
  bSelect->Resize(80, 20);
  bSelect->Move(530, 10);
  bSelect->SetOnButtonReleaseCB(ButtonSelectHandler, ta);
  AInputBox* ib = AInputBox::AttachTo(wProcess, "");
  ib->SetTitle("Search");
  ib->Move(10, 10);
  ib->Resize(100,25);
  ib->SetOnValueChangedCB(InputBoxValueChangedHandler, ta);
  ib->SetStyle(AUIWidgetStyle::Flat);
  ib->SetPressDepth(1);
  UpdateProcTable(ta, "");
}

int main() {
  time_point<high_resolution_clock> start = high_resolution_clock::now();
  AUI* cg = AUI::Create("cg0 main");
  AWindow* w = cg->MainWnd();
  w->EnableResize();
  w->Resize(1024, 768);
  w->DisableResize();
  AButton* bOpenProc = AButton::AttachTo(w, "Processes");
  bOpenProc->Resize(100, 20);
  bOpenProc->SetOnButtonReleaseCB(ButtonProcessesHandler, w);
  bOpenProc->SetStyle(AUIWidgetStyle::Simple3D);
  bOpenProc->SetPressDepth(0);
  ALabel* lb = ALabel::AttachTo(w, "Select process");
  lb->Move(120, 8);
  lb->Resize(1024, 30);
  lb->SetHAlign(AUIHAlign::left);
  lb->SetVAlign(AUIVAlign::center);
  //ButtonProcessesHandler(0, w, 0);
  StopTimer(start);
  cg->ProcessMessages();
  delete cg;
  cg = 0;

  return 0;
}


