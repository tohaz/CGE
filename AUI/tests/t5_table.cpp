#include <chrono>
#include <future>
#include <thread>
#include <memory.h>
#include <random>

#include "AUILib.h"

bool need_delay_exit = 1;

using namespace aui;

void PopulateTableWithTrash(ATable* table, UINT32 numRows, UINT32 numCols, size_t stringLength = 16) {
  if (!table) return;
  D1("PopulateTableWithTrash() -> Populating table via public API: {} rows x {} cols", numRows, numCols);
  
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<size_t> dist(0, BaseAlphabet.length() - 1);
  INT64 startRowIdx = static_cast<INT64>(table->Rows());
  table->AddColumns(numCols);
  table->AddRows(numRows);
  std::string trashBuffer;
  trashBuffer.resize(stringLength);
  AUICellData cell;
  cell.hAlign = AUIHAlign::center;
  cell.vAlign = AUIVAlign::center;
  for (UINT32 r = 0; r < numRows; ++r) {
    INT64 row = startRowIdx + static_cast<INT64>(r);
    for (UINT32 c = 0; c < numCols; ++c) {
      INT64 col = static_cast<INT64>(c);
      for (size_t i = 0; i < stringLength; ++i) {
        trashBuffer[i] = BaseAlphabet[dist(gen)];
      }
      // Assign the new string data to the reusable loop cell
      cell.data = trashBuffer;
      // Pass the address of the stack object safely.
      // Inside Insert(), it will move the string out, but cell.data 
      // will be cleanly re-assigned on the next loop iteration.
      table->Insert(row, col, &cell);
    }
  }
  table->Draw();
  D1("PopulateTableWithTrash() -> Dataset injected successfully via public interface.");
}


INT32 TestGeneral(ATable *ta) {
  AUICellData di;
  ta->Clear();
  ta->Resize(400, 250);
  ta->AddColumn();
  ta->AddColumn();
  ta->AddRow();
  ta->AddRows(5);
  ta->AddColumns(5);
  if(ta->Rows() != 6) E("number of rows is wrong{}", ta->Rows())
  if(ta->Columns() != 7) E("number of columnts is wrong {}", ta->Rows())
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
  return 0;
}

INT32 AutowidenTest(ATable *ta) {
  AUICellData di;
	ta->Clear();
  ta->AddColumns(5);
  ta->AddRows(5);
  ta->SetColumnWidth(0, 1);
  ta->SetAutoWiden(true);
  di.data = "some decently long string";
  ta->Insert(0, 0, &di);
  if(ta->ColumnWidth(0) > 10) {
    D("Column Autowiden test passed({})\n", ta->ColumnWidth(0))
    return 0;
  }
	D("Column autowiden test failed")
	return 1;
}

INT32 AddRowTest(ATable *ta) {
	ta->Clear();
	ta->AddRow();
	D()
	return 0;
}

INT32 TestTableMemoryStressFlush(AWidget* parent) {
  ATable* table = ATable::AttachTo(parent);
  if (!table) return 1;
  D1("--------------------------------------------------");
  D1("ATable Benchmark -> Generating dataset from the outside (Public API)...");
  // Populate 20,000 rows x 10 columns = 200,000 cells via the external helper function
  PopulateTableWithTrash(table, 2000, 10, 24);
  if (table->Rows() != 2000) {
    E("REGRESSION: Public API data injection check failed!");
    return 2;
  }
  D1("ATable Benchmark -> Triggering parallel map memory-purge sweep...");
  auto start = std::chrono::high_resolution_clock::now();
  // Execute our multi-threaded mutex-free table clearing function
  table->Clear();
  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  D1("ATable Benchmark -> SUCCESS: 20,000 cells cleared concurrently in {} ms", elapsed);
  D1("--------------------------------------------------");
  return 0;
}

int main() {
	//char *qqq = new char[1]; // generate error
  UINT32 delay_ms = 50; // delay before thead calls window to close
  AUI* au = AUI::Create("table");
  AWindow* w = au->MainWnd();
  ATable* ta = ATable::AttachTo(w);
  INT32 testsfailed = 0;
  
  testsfailed += TestGeneral(ta);
  testsfailed += AutowidenTest(ta);
  testsfailed += AddRowTest(ta);
  testsfailed += TestTableMemoryStressFlush(w);
  
  std::future<void> handle;
  if(need_delay_exit) {
    handle = std::async(std::launch::async, [=]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
      au->ExitAUI();
    });
  }

  au->ProcessMessages();
  
  if(need_delay_exit) {
    handle.get();
  }
  delete au;
  au = nullptr;
  ta = 0;
  
  return testsfailed;
}


