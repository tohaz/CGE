#include <cassert>
#include <chrono>
#include <future>
#include <thread>
#include <memory.h>
#include <X11/Xlib.h> // Required for XInitThreads
#include "AUILib.h"

using namespace aui;

bool need_delay_exit = 1;

INT32 TestInputBoxSunkenEffect(AInputBox* ib) {
  Display* d = ib->AUIPtr()->Disp();
  Window w = ib->Wnd();
  // 1. Force sync and grab the image from the window
  XSync(d, False);
  XImage* img = XGetImage(d, w, 0, 0, ib->SizeXUI32(), ib->SizeYUI32(), AllPlanes, ZPixmap);
  if (!img) {
    E("Test failed: Could not grab XImage from widget");
    return 1;
  }
  // 2. Define test points
  // Use SafeINT32 to convert coordinates for XGetPixel
  INT32 centerX = SafeINT32(ib->SizeXUI32() / 2);
  INT32 centerY = SafeINT32(ib->SizeYUI32() / 2);
  // Point A: Top-left corner (should be shadow)
  unsigned long shadowPixel = XGetPixel(img, 1, 1);
  // Point B: Center of the field (should be white)
  unsigned long fieldPixel = XGetPixel(img, centerX, centerY);
  RGBAColor shadowColor, fieldColor, bgColor;
  shadowColor.value = (UINT32)shadowPixel;
  fieldColor.value = (UINT32)fieldPixel;
  bgColor.value = ib->BGColor();
  // 3. Assertions
  // Check if shadow is darker than background (Sunken effect check)
  bool isSunken = (shadowColor.rgba.r < bgColor.rgba.r) || 
                  (shadowColor.rgba.g < bgColor.rgba.g) || 
                  (shadowColor.rgba.b < bgColor.rgba.b);
  // Check if the center is white/light grey (Field check)
  bool isFieldWhite = (fieldColor.rgba.r > 200);
  XDestroyImage(img);
  if(isSunken && isFieldWhite) {
    D("SUCCESS: Sunken effect regression test passed!");
  } else {
    E("REGRESSION FILLED: Sunken effect is broken!");
      if (!isSunken) E("  - Reason: Shadow at (1,1) is not darker than BG");
      if (!isFieldWhite) E("  - Reason: Field center is not white");
    }
  return 0;
}

INT32 TestSunkenEffect(aui::AInputBox* ib) {
  Display* d = ib->AUIPtr()->Disp();
  XSync(d, False);
  XImage* img = XGetImage(d, ib->Wnd(), 0, 0, ib->SizeXUI32(), ib->SizeYUI32(), AllPlanes, ZPixmap);
  if (!img) return 1;
  INT32 depth = ib->PressDepth();
  // Test point in the middle of the top slope
  INT32 checkCoord = depth > 1 ? depth / 2 : 1;
  unsigned long shadowPixel = XGetPixel(img, checkCoord, checkCoord);
  RGBAColor shadow, bg;
  shadow.value = static_cast<UINT32>(shadowPixel);
  bg.value = ib->BGColor();
  XDestroyImage(img);
  // Verify that the slope is darker than the widget background
  bool isDarker = (shadow.rgba.r < bg.rgba.r) && 
                    (shadow.rgba.g < bg.rgba.g) && 
                    (shadow.rgba.b < bg.rgba.b);
  if (isDarker) {
    D("SUCCESS: Sunken shadow detected at ({}, {})", checkCoord, checkCoord);
  } else {
    E("REGRESSION: Sunken effect missing! Shadow pixel is not darker than BG.");
  }
  return 0;
}

INT32 TestFieldDepth(aui::AInputBox* ib) {
  Display* d = ib->AUIPtr()->Disp();
  XSync(d, False);
  XImage* img = XGetImage(d, ib->Wnd(), 0, 0, ib->SizeXUI32(), ib->SizeYUI32(), AllPlanes, ZPixmap);
  if (!img) return 1;
  INT32 depth = ib->PressDepth();
  // This point should be exactly at the start of the white input field
  INT32 fieldX = depth + 1;
  INT32 fieldY = depth + 1;
  unsigned long fieldPixel = XGetPixel(img, fieldX, fieldY);
  RGBAColor field;
  field.value = static_cast<UINT32>(fieldPixel);
  XDestroyImage(img);
  // The field should be white or very light (gradient starts here)
  // We check if red channel is > 200 (assuming white/light field)
  bool isFieldStartCorrect = (field.rgba.r >= 200);
  if (isFieldStartCorrect) {
    D("SUCCESS: Input field correctly starts at depth {}", depth);
  } else {
    E("REGRESSION: Field offset mismatch! Pixel at ({}, {}) is not light enough.", fieldX, fieldY);
    E("Color found: R:{} G:{} B:{}", field.rgba.r, field.rgba.g, field.rgba.b);
  }
  return 0;
}

int main() {
  UINT32 delay_ms = 100; 
  INT32 testsfailed = 0;
  AUI* au = AUI::Create("inputbox");
  AWindow* w = au->MainWnd();
  AInputBox* ib = AInputBox::AttachTo(w, (char *)"testing");
  ib->Resize(100, 25);
  ib->SetStyle(AUIWidgetStyle::Simple3D);

  ib->SetPressDepth(2);
  testsfailed += TestSunkenEffect(ib);

  TestFieldDepth(ib);
  ib->SetPressDepth(10);

  testsfailed += TestSunkenEffect(ib); // Should still find shadow at depth/2
  TestFieldDepth(ib); 

  ib->SetPressDepth(3);
  testsfailed += TestInputBoxSunkenEffect(ib);

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
  
  return testsfailed;
}


