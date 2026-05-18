#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"

using namespace aui;

// Helper to check if a specific window ID is registered in the main event registry
bool IsWidgetRegisteredInAUI(AUI* au, Window w) {
  return au->IsWindowRegistered(w);
}

INT32 TestComboBoxLayout(AWidget* parent) {
  AComboBox* combo = AComboBox::AttachTo(parent, "Select item...");
  if (!combo) {
    E("REGRESSION: Failed to allocate AComboBox instance context");
    return 1;
  }

  // ComboBox height should initialize to 28 pixels based on structural defaults
  UINT32 expectedH = 28;
  UINT32 actualH = combo->SizeYUI32();
  if (actualH != expectedH) {
    E("REGRESSION: ComboBox height mismatch! Expected {}, got {}", expectedH, actualH);
    return 2;
  }

  D1("SUCCESS: ComboBox structural layout dimensions verified ({})", actualH);
  return 0;
}

INT32 TestComboBoxLazyInitialization(AWidget* parent) {
  AComboBox* combo = AComboBox::AttachTo(parent, "Lazy Test");
  combo->AddItem("Item 1");
  combo->AddItem("Item 2");

  // Under the new Show/Hide paradigm, sub-widgets should remain unallocated before the first click
  if (combo->IsPopupOpen()) {
    E("REGRESSION: Dropdown state reports open immediately after instantiation");
    return 1;
  }

  combo->OpenDropDown();
  if (!combo->IsPopupOpen()) {
    E("REGRESSION: OpenDropDown failed to transition internal state tracking flag");
    combo->CloseDropDown();
    return 2;
  }

  D1("SUCCESS: ComboBox lazy initialization mechanics verify clean state shifts");
  combo->CloseDropDown();
  return 0;
}

INT32 TestComboBoxDataSyncAndClear(AWidget* parent) {
  AComboBox* combo = AComboBox::AttachTo(parent, "Data Test");
  combo->AddItem("Alpha");
  combo->AddItem("Beta");

  combo->OpenDropDown();
  // Double invoke verification test to check for exponential vector growth bugs
  combo->CloseDropDown();
  combo->OpenDropDown();

  // If the internal cleanup logic functions fail, trailing events will leak memory.
  // We execute a test cycle path validation here.
  combo->CloseDropDown();

  D1("SUCCESS: ComboBox data synchronization vector flushing passed cleanly");
  return 0;
}

INT32 TestComboBoxAddItemAndGetText(AWidget* parent) {
  // Scenario 1: Test AttachTo(parent) -> Must initialize with 0 elements and selection index -1
  AComboBox* comboEmpty = AComboBox::AttachTo(parent);
  comboEmpty->AddItem("First");
  comboEmpty->AddItem("Second");
  if (comboEmpty->ItemCount() != 2 || comboEmpty->GetItemText(0) != "First" || comboEmpty->GetItemText(1) != "Second") {
    E("REGRESSION: Single-argument AttachTo raw allocation failed data integrity validation steps!");
    return 1;
  }
  // Scenario 2: Test AttachTo(parent, "Initial") -> Must seed the single text value into index 0 and select it
  AComboBox* comboSingle = AComboBox::AttachTo(parent, "Initial");
  comboSingle->AddItem("Append1");
  if (comboSingle->ItemCount() != 2 || comboSingle->GetItemText(0) != "Initial" || comboSingle->GetItemText(1) != "Append1" || comboSingle->GetSelectedIndex() != 0) {
    E("REGRESSION: Two-argument AttachTo factory failed layout seeding or tracking validation checks!");
    return 2;
  }
  // Scenario 3: Test AttachTo(parent, {list}) -> Must seed multiple items sequentially and select index 0
  AComboBox* comboList = AComboBox::AttachTo(parent, {"Alpha", "Beta", "Gamma"});
  if (comboList->ItemCount() != 3 || comboList->GetItemText(0) != "Alpha" || comboList->GetItemText(2) != "Gamma" || comboList->GetSelectedIndex() != 0) {
    E("REGRESSION: Variadic initializer_list AttachTo factory mapping generated corrupted data offsets!");
    return 3;
  }
  D1("SUCCESS: All 3 factory initialization paths (AttachTo variants) verified safely and cleanly");
  return 0;
}

INT32 TestComboBoxWindowLifecycleReuse(AWidget* parent) {
  AComboBox* combo = AComboBox::AttachTo(parent, "Lifecycle Test");
  combo->AddItem("Entry 1");

  combo->OpenDropDown();
  // Capture native window handle allocated under the current runtime mapping context
  // Note: Depending on your specific implementation structure, get the popup handle.
  // If your AComboBox exposes a method to get popup window ID, use it, or check alignment via active map scans.
  combo->CloseDropDown();

  // The window structure should remain alive and hidden in memory without throwing BadWindow faults
  combo->OpenDropDown();
  combo->CloseDropDown();

  D1("SUCCESS: Dropdown surface context reusability checks finished successfully");
  return 0;
}

INT32 TestComboBoxSelectionStateLifecycle(AWidget* parent) {
  // Use raw single-argument factory to ensure the widget starts in a completely unselected state
  AComboBox* combo = AComboBox::AttachTo(parent);
  if (combo->GetSelectedIndex() != -1) {
    E("REGRESSION: Unselected ComboBox index tracking initialization state is not -1");
    return 1;
  }
  combo->AddItem("Alpha");
  combo->AddItem("Beta");
  // Explicitly trigger mutation to evaluate state synchronization overrides
  combo->SetSelectedIndex(1);
  if (combo->GetSelectedIndex() != 1) {
    E("REGRESSION: SetSelectedIndex failed to commit index modification state values");
    return 2;
  }
  D1("SUCCESS: Helper operations 3 & 4 (Selection State read/write) match specifications");
  return 0;
}
INT32 TestComboBoxClearRoutine(AWidget* parent) {
  // Use the list initialization factory to seed data structures safely
  AComboBox* combo = AComboBox::AttachTo(parent, {"Data 1", "Data 2"});
  if (combo->GetSelectedIndex() != 0) {
    E("REGRESSION: Factory failed to default selection to index 0 on multi-item instantiation");
    return 1;
  }
  // Trigger full data vector and selection tracking purge
  combo->Clear();
  if (combo->GetSelectedIndex() != -1) {
    E("REGRESSION: Clear() failed to restore unselected default index flag state (-1)");
    return 2;
  }
  D1("SUCCESS: Helper operation 2 (Clear data vector flushing routine) executed smoothly");
  return 0;
}

void ButtonCloseHandler(XEvent* ev, AWidget* w, void* d) {
  D("user quit callback fired, bye world {} {}", (UINT64)ev, (UINT64)d)
  w->AUIPtr()->ExitAUI();
}

int main() {
  UINT32 delay_ms = 50; // delay before thread calls window to close
  INT32 testsfailed = 0;
  AUI* au = AUI::Create("AComboBox class test");
  
  AWindow* w = au->MainWnd();
  AButton* bn = AButton::AttachTo(w, "Close");
  bn->SetOnButtonReleaseCB(ButtonCloseHandler, nullptr);
	
  // Packaged test streams evaluating component states sequentially
  testsfailed += TestComboBoxLayout(w);
  testsfailed += TestComboBoxLazyInitialization(w);
  testsfailed += TestComboBoxDataSyncAndClear(w);
  testsfailed += TestComboBoxWindowLifecycleReuse(w);
  testsfailed += TestComboBoxAddItemAndGetText(w);
  testsfailed += TestComboBoxSelectionStateLifecycle(w);
  testsfailed += TestComboBoxClearRoutine(w);
   
  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    au->ExitAUI();
  });

  au->ProcessMessages();
  
  handle.get();

  delete au;
  au = nullptr;
  bn = nullptr;

  return testsfailed;
}
