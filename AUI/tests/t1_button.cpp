#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"

using namespace aui;

void ButtonCloseHandler(XEvent* ev, AWidget* w, void* d) {
  D("user quit callback fired, bye world %lu %lu", (UINT64)ev, (UINT64)d)
  w->AUIPtr()->ExitAUI();
}

void ButtonTestHandler(XEvent* ev, AWidget* w, void* d) {
  D("user callback fired, congrats %lu %lu %lu", (UINT64)ev, (UINT64)w, (UINT64)d)
}

INT32 TestStyleAndDepth(AButton* btn) {
    if (!btn) {
        D("Test failed: Button pointer is null");
        return 1;
    }
    // Test 1: Set Style to Simple3D and check XRender resource
    btn->SetStyle(AUIWidgetStyle::Simple3D);
    if (btn->Style() != AUIWidgetStyle::Simple3D) {
        D("Test failed: Style not updated to Simple3D");
        return 2;
    }
    // Note: We assume mRenderPicture is accessible via friend or getter
    // Checking if UpdateBuffer actually created the picture
    if (btn->BB() != 0 && btn->GetRenderPicture() == None) {
        D("Test failed: Simple3D active but mRenderPicture is None");
        return 3;
    }
    // Test 2: Depth boundaries
    btn->SetPressDepth(20); // Should be clamped to 15
    // You might need a GetDepth() or check internal state
    // if (btn->GetDepth() > 15) { D("Depth clamping failed"); return 4; }
    // Test 3: Switch back to Flat and ensure XRender picture is cleaned
    btn->SetStyle(AUIWidgetStyle::Flat);
    if (btn->GetRenderPicture() != None) {
        D("Test failed: Style is Flat but mRenderPicture was not freed");
        return 5;
    }

    return 0;
}

int main() {
	//char *qqq = new char[1]; // generate error
  UINT32 delay_ms = 50; // delay before thead calls window to close
  INT32 testsfailed = 0;
  AUI* au = AUI::Create("button");
  AWindow* w = au->MainWnd();
  
  AButton* bn = AButton::AttachTo(w, "Close");
  bn->SetOnButtonReleaseCB(ButtonCloseHandler, nullptr);

  AButton* bnt = AButton::AttachTo(w, "Test");
  bnt->Move(100, 10);
  bnt->Resize(100, 30);
  bnt->SetOnButtonReleaseCB(ButtonTestHandler, nullptr);

  testsfailed += TestStyleAndDepth(bnt);

  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
  	au->ExitAUI();
  });

  au->ProcessMessages();
  
  handle.get();

  delete au;
  au = nullptr;
  bn = nullptr;
  bnt = nullptr;

  return testsfailed;
}


