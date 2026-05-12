#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"

using namespace aui;

INT32 TestAWidgetResizeBuffer(AButton* wid) {
  D("TestAWidgetResizeBuffer()");
  if (!wid) {
    D("Test failed: Widget pointer is null");
    return 1;
  }
  UINT64 oldSizeX = wid->SizeX();
  UINT64 oldSizeY = wid->SizeY();
  Pixmap oldBB = wid->BB();
  // Test 1: Resize the widget
  wid->Resize(SafeUINT32(oldSizeX + 20), SafeUINT32(oldSizeY + 20));
  // Test 2: Check if Pixmap was replaced
  if (wid->BB() == oldBB) {
    D("Test failed: BackBuffer Pixmap was not updated after Resize");
    return 2;
  }
  // Test 3: Check internal dimensions
  if (wid->SizeX() != oldSizeX + 20 || wid->SizeY() != oldSizeY + 20) {
    D("Test failed: Internal dimensions not updated. Expected {}, got {}",
      oldSizeX + 20, wid->SizeX());
    return 3;
  }
  // Test 4: Verify Pixmap existence on X Server
  if (wid->BB() == 0) {
    D("Test failed: BackBuffer is null after resize");
      return 4;
  }
    return 0;
}

INT32 TestCoordinateCorrection(AButton* wid) {
  D("TestCoordinateCorrection()");
  if (!wid) return 1;
  XEvent ev;
  ev.type = ButtonPress;
  // Test 1: Negative coordinates
  ev.xbutton.x = -50;
  ev.xbutton.y = -10;
  wid->CorrectCoordinates(ev);
  if (ev.xbutton.x != 0 || ev.xbutton.y != 0) {
    D("Test failed: Negative coordinates not clamped to 0");
    return 2;
  }
  // Test 2: Overflow coordinates
  ev.xbutton.x = (INT32)(wid->SizeX() + 500);
  ev.xbutton.y = (INT32)(wid->SizeY() + 500);
  wid->CorrectCoordinates(ev);
  if ((UINT64)ev.xbutton.x != wid->SizeX() || (UINT64)ev.xbutton.y != wid->SizeY()) {
    D("Test failed: Overflow coordinates not clamped to SizeX/SizeY");
    return 3;
  }
  return 0;
}

INT32 TestAWidgetProperties(AButton* wid) {
    if (!wid) return 1;
    // Test 1: GC Initialization
    if (wid->GCPtr() == 0) {
        D("Test failed: Graphics Context (GC) is null");
        return 2;
    }
    // Test 2: BGColor storage
    UINT32 testColor = 0xFF55AA;
    wid->SetBGColor(testColor);
    if (wid->BGColor() != testColor) {
        D("Test failed: BGColor mismatch. Expected 0x%X, got 0x{}", testColor, wid->BGColor());
        return 3;
    }
    // 3. Test Simple3D Style
    // This internally triggers UpdateBuffer() and creates mRenderPicture
    wid->SetStyle(AUIWidgetStyle::Flat);
    wid->SetStyle(AUIWidgetStyle::Simple3D);
    if (wid->Style() != AUIWidgetStyle::Simple3D) {
        D("Test failed: Failed to set Simple3D style");
        return 3;
    }
    return 0;
}

INT32 TestPixmapLifecycle(AButton* wid) {
    if (!wid) {
        D("Test failed: Widget pointer is NULL");
        return 1;
    }
    // Ensure we have a buffer to start with
    wid->Draw();
    Pixmap initialBuffer = wid->BB();
    if (initialBuffer == 0) {
        D("Test failed: Initial Pixmap allocation failed");
        return 2;
    }
    // Perform resize (which should free old Pixmap and create new one)
    UINT32 newW = (UINT32)wid->SizeX() + 10;
    UINT32 newH = (UINT32)wid->SizeY() + 10;
    wid->Resize(newW, newH);
    Pixmap resizedBuffer = wid->BB();
    if (resizedBuffer == 0) {
        D("Test failed: Pixmap was lost after Resize");
        return 3;
    }
    if (resizedBuffer == initialBuffer) {
        D("Test failed: Pixmap was not re-allocated after Resize");
        return 4;
    }
    return 0;
}

INT32 TestAWidgetHierarchy(AWidget* expectedParent, AWidget* child) {
  if (!child || !expectedParent) {
    D("Test failed: Input pointers are null");
    return 1;
  }
  if (child->ParentWidget() != expectedParent) {
    D("Test failed: ParentWidget() does not match the expected parent object");
    return 2;
  }
  if (child->AUIPtr() != expectedParent->AUIPtr()) {
    D("Test failed: Child and Parent do not share the same AUI pointer");
    return 3;
  }
  return 0;
}

int main() {
	//char *qqq = new char[1]; // generate error
  UINT32 delay_ms = 50; // delay before thead calls window to close
	INT32 testsfailed = 0;
  AUI* au = AUI::Create("AWidget class test");
  
  AWindow* w = au->MainWnd();
  AButton* bn = AButton::AttachTo(w, "Close");
  
  testsfailed += TestAWidgetResizeBuffer(bn);
  testsfailed += TestCoordinateCorrection(bn);
  testsfailed += TestAWidgetProperties(bn);
  testsfailed += TestPixmapLifecycle(bn);
  testsfailed += TestAWidgetHierarchy(w, bn);
  
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


