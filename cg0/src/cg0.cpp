#include <string>
#include <cassert>
#include <random>
#include <AUILib.h>
#include "ProcessList.h"

using namespace aui;
using namespace cg;
using namespace std::chrono;

ProcessList gPR;
INT32 gPID = 0;
std::string gPath = "";
std::string gPIDStr = "";
ALabel* gLB = nullptr;
AButton* bSearch = nullptr;
AInputBox* iVal = nullptr;

void StopTimer(time_point<high_resolution_clock> start) {
  time_point<high_resolution_clock> end = high_resolution_clock::now();
  duration<double, std::milli> duration_ms1 = end - start; // @suppress("Invalid arguments")
  D1("measured time: {} ms", duration_ms1.count()); // @suppress("Function cannot be instantiated")
}

void ShowSearchUI(UNUSED AWidget *w) {
  D("widget is {}", (UINT64)w)
  if(!bSearch) {
    bSearch = AButton::AttachTo(w, "Search");
    bSearch->Hide();
    bSearch->Show();
    bSearch->Move(10, 730);
  }
  if(!iVal) {
    iVal = AInputBox::AttachTo(w, "");
    iVal->Move(10, 40);
    iVal->Resize(100, 25);
  }
//  bOpenProc->Resize(200, 50);

}

void ButtonSelectHandler(UNUSED XEvent* ev, AWidget* w, UNUSED void* d) {
  D4()
  AButton* b = (AButton*) w;
  ATable* ta = (ATable*)d;
  gPath = ta->DataAt(ta->CursorRow(), 1);
  gPIDStr = ta->DataAt(ta->CursorRow(), 0);
  try {
      gPID = std::stoi(gPIDStr.c_str());
  } catch (const std::invalid_argument& e) {
      D1("Invalid input: No conversion could be performed.")
  } catch (const std::out_of_range& e) {
      D1("Result out of range for unsigned long.")
  }
  if(gPID > 0) {
    std::string ps = gPath;
    size_t firstSpace = gPath.find(' ');
    if (firstSpace != std::string::npos) {
        ps.erase(firstSpace);
    }
    D2("chosen PID {}", gPID);
    *gLB = "";
    *gLB += gPIDStr + " " + ps;
  }
  else {
    D2("not opening PID")
    gPID = 0;
  }
  D1("cursor PID data '{}', row is {}", gPath.c_str(), ta->CursorRow())
  if(ta->CursorRow() != -1) {
    ShowSearchUI(b->AUIPtr()->MainWnd());
    b->ParentWidget()->Close();
  }
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
  for (const auto& [id, value] : gPR) {
    pd = value;
    if(pd->Path().contains(filter) || pd->PidStr().contains(filter)) {
      UNUSED INT64 realIdx = ta->AddRow();
      D2("Inserting row: realIdx={}, manualRow={}", realIdx, row);
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
  AButton* bSelect = AButton::AttachTo(wProcess, "Select");
  bSelect->Resize(80, 20);
  bSelect->Move(530, 10);
  bSelect->SetOnButtonReleaseCB(ButtonSelectHandler, ta);
  AInputBox* ib = AInputBox::AttachTo(wProcess, "");
  ib->SetTitle("Search");
  ib->Move(10, 10);
  ib->Resize(100,25);
  ib->SetOnValueChangedCB(InputBoxValueChangedHandler, ta);
  ib->SetStyle(AUIWidgetStyle::Simple3D);
  ib->SetPressDepth(3);
  UpdateProcTable(ta, "");
}

void PopulateTableWithTrash(UNUSED ATable* table, UNUSED UINT32 numRows, UNUSED UINT32 numCols, UNUSED size_t stringLength = 16) {
  if (!table) return;
  D1("PopulateTableWithTrash() -> Populating table via public API: {} rows x {} cols", numRows, numCols);
  // 1. Initialize random number generator using the BaseAlphabet declared in defaults.h
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<size_t> dist(0, BaseAlphabet.length() - 1);
  // 2. Expand table bounds using official public methods
  UNUSED INT64 startRowIdx = static_cast<INT64>(table->Rows());
  table->AddColumns(numCols);
  table->AddRows(numRows);
  std::string trashBuffer;
  trashBuffer.resize(stringLength);
  for (UINT32 r = 0; r < numRows; ++r) {
    INT64 row = startRowIdx + static_cast<INT64>(r);
    for (UINT32 c = 0; c < numCols; ++c) {
      INT64 col = static_cast<INT64>(c);
      // Generate randomized characters into buffer string
      for (size_t i = 0; i < stringLength; ++i) {
        trashBuffer[i] = BaseAlphabet[dist(gen)];
      }
      // 3. Allocate clean cell data on the heap to satisfy the AUICellData* signature
      auto* cell = new AUICellData();
      cell->data = trashBuffer;
      cell->hAlign = AUIHAlign::center;
      cell->vAlign = AUIVAlign::center;
      // 4. Pass the pointer downstream into the widget context via public interface
      table->Insert(row, col, cell);
    }
  }
  table->Draw();
  D1("PopulateTableWithTrash() -> Dataset injected successfully via public interface.");
}


int main() {
  time_point<high_resolution_clock> start = high_resolution_clock::now();
  AUI* cg = AUI::Create("cg0 main");
  UNUSED AWindow* w = cg->MainWnd();
//  AComboBox* myCombo = AComboBox::AttachTo(w, "Select configuration...");
//  myCombo->Move(20, 60);
//  myCombo->Resize(180, 28);
//
//  // Load the data payload vectors
//  myCombo->AddItem("Debug Mode");
//  myCombo->AddItem("Release Optimization");
//  myCombo->AddItem("Testing Core Environment");
  UNUSED UINT32 r = 10000, c = 100;
  ATable* ta = ATable::AttachTo(w);
//  ta->AddRows(r);
//  ta->AddColumns(c);
  PopulateTableWithTrash(ta, r, c, 10);
  StopTimer(start);
  start = high_resolution_clock::now();
  ta->ClearMT();
  StopTimer(start);

//  UNUSED AButton* bOp = AButton::AttachTo(w, "Processes");
//  bOp->Resize(100,30);
//  AModalWindow::Message(w, "test title", "test message");
//  w->EnableResize();
//  w->Resize(1024, 768);
//  w->DisableResize();
//  AButton* bOpenProc = AButton::AttachTo(w, "Processes");
//  bOpenProc->Resize(100, 26);
//  bOpenProc->SetOnButtonReleaseCB(ButtonProcessesHandler, w);
//  bOpenProc->SetStyle(AUIWidgetStyle::Simple3D);
//  bOpenProc->SetPressDepth(0);
//  gLB = ALabel::AttachTo(w, "Select process");
//  gLB->Move(120, 8);
//  gLB->Resize(1024, 30);
//  gLB->SetHAlign(AUIHAlign::left);
//  gLB->SetVAlign(AUIVAlign::center);
//  //ButtonProcessesHandler(0, w, 0);
  cg->ProcessMessages();
  delete cg;
  cg = 0;

  return 0;
}


