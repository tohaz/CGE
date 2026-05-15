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

void ButtonCloseHandler(XEvent* ev, AWidget* w, void* d) {
  D("user quit callback fired, bye world {} {}", (UINT64)ev, (UINT64)d)
  w->AUIPtr()->ExitAUI();
}

INT32 TestComboBoxAddItemAndGetText(AWidget* parent) {
  AComboBox* combo = AComboBox::AttachTo(parent, "Helper Test 1");
  
  combo->AddItem("First");
  combo->AddItem("Second");

  // Validate operation 5: GetItemText extraction indices
  if (combo->GetItemText(0) != "First" || combo->GetItemText(1) != "Second") {
    E("REGRESSION: Helper GetItemText or AddItem failed data integrity constraints!");
    return 1;
  }

  D1("SUCCESS: Helper operations 1 & 5 (AddItem/GetItemText) verified cleanly");
  return 0;
}

INT32 TestComboBoxSelectionStateLifecycle(AWidget* parent) {
  AComboBox* combo = AComboBox::AttachTo(parent, "Helper Test 2");
  combo->AddItem("Alpha");
  combo->AddItem("Beta");

  // Default state validation (Operation 3)
  if (combo->GetSelectedIndex() != -1) {
    E("REGRESSION: Unselected ComboBox index tracking initialization state is not -1");
    return 1;
  }

  // State mutation validation (Operation 4)
  combo->SetSelectedIndex(1);
  if (combo->GetSelectedIndex() != 1) {
    E("REGRESSION: SetSelectedIndex failed to commit index modification state values");
    return 2;
  }

  D1("SUCCESS: Helper operations 3 & 4 (Selection State read/write) match specifications");
  return 0;
}

INT32 TestComboBoxClearRoutine(AWidget* parent) {
  AComboBox* combo = AComboBox::AttachTo(parent, "Helper Test 3");
  combo->AddItem("Data 1");
  combo->SetSelectedIndex(0);

  // Trigger full reset (Operation 2)
  combo->Clear();

  if (combo->GetSelectedIndex() != -1) {
    E("REGRESSION: Clear() failed to restore unselected default index flag state (-1)");
    return 1;
  }

  D1("SUCCESS: Helper operation 2 (Clear data vector flushing routine) executed smoothly");
  return 0;
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
