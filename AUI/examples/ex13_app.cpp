#include <string>
#include <cassert>
#include <random>
#include <charconv>
#include "AUILib.h"

using namespace aui;

volatile UINT32 gU32 = 0;
volatile UINT64 gU64 = 0;
volatile float gFl = 0;
std::atomic<std::string*> gStrPtr{new std::string("")};

struct TestControls {
  ALabel* lb1;
  ALabel* lb2;
  ALabel* lb3;
  ALabel* lb4;
  AInputBox* i1;
  AInputBox* i2;
  AInputBox* i3;
  AInputBox* i4;
};

void ButtonResetHandler(UNUSED XEvent* ev, UNUSED AWidget* wi, UNUSED void* d) {
  UNUSED TestControls* t = (TestControls*) d;
  gU32 = 0;
  gU64 = 0;
  gFl  = 0.0;
  std::string* newStr = new std::string("");
  std::string* oldStr = gStrPtr.exchange(newStr);
  delete oldStr;
  t->lb1->SetText("None");
  t->lb2->SetText("None");
  t->lb3->SetText("None");
  t->lb4->SetText("None");
}

void ButtonReadHandler(UNUSED XEvent* ev, UNUSED AWidget* wi, UNUSED void* d) {
  D2()
  std::string* currentPtr = gStrPtr.load();
  TestControls* t = (TestControls*) d;
  *(t->lb1) = std::to_string(gU32);
  *(t->lb2) = std::to_string(gU64);
  *(t->lb3) = std::to_string(gFl);
  if(currentPtr) {
    *(t->lb4) = *currentPtr;
  }
  else {
    D("error loading string")
  }
}

void ButtonSetHandler(UNUSED XEvent* ev, UNUSED AWidget* wi, void* d) {
  TestControls* t = (TestControls*) d;
  UNUSED ALabel* lU32 = t->lb1;
  UNUSED ALabel* lU64 = t->lb2;
  UNUSED ALabel* lFl = t->lb3;
  UNUSED ALabel* lStr = t->lb4;
  UNUSED AInputBox* iU32 = t->i1;
  UNUSED AInputBox* iU64 = t->i2;
  UNUSED AInputBox* iFl = t->i3;
  UNUSED AInputBox* iStr = t->i4;
  std::string sU32 = iU32->Text();
  UINT32 tU32;
  UNUSED auto [ptr, ec] = std::from_chars(sU32.data(), sU32.data() + sU32.size(), tU32);
  if (ec == std::errc()) {
    D1("success U32")
    gU32 = tU32;
    *(t->lb1) = std::to_string(gU32);
  }
  else {
    if (ec == std::errc::invalid_argument) {
      *lU32 = "Error_IA";
    }
    else {
      if (ec == std::errc::result_out_of_range) {
        *lU32 = "Error_OR";
      }
      else {
        *lU32 = "UNKNOWN";
      }
    }
  }
  std::string sU64 = iU64->Text();
  UINT64 tU64;
  UNUSED auto [ptr64, ec64] = std::from_chars(sU64.data(), sU64.data() + sU64.size(), tU64);
  if(ec64 == std::errc()) {
    D1("success U64")
    gU64 = tU64;
    *(t->lb2) = std::to_string(gU64);
  }
  else {
    if(ec64 == std::errc::invalid_argument) {
      D()
      *lU64 = "Error_IA";
    }
    else {
      if(ec64 == std::errc::result_out_of_range) {
        D()
        *lU64 = "Error_OR";
      }
    }
  }
  std::string sFl = iFl->Text();
  float tFl;
  UNUSED auto [ptrFl, ecFl] = std::from_chars(sFl.data(), sFl.data() + sFl.size(), tFl);
  if(ecFl == std::errc()) {
    D1("success float")
    gFl = tFl;
    *(t->lb3) = std::to_string(gFl);
  }
  else {
    if(ecFl == std::errc::invalid_argument) {
      D()
      *lFl = "Error_IA";
    }
    else {
      if(ecFl == std::errc::result_out_of_range) {
        D()
        *lFl = "Error_OR";
      }
      else {
        D("float UNKNOWN")
      }
    }
  }
  std::string* newStr = new std::string(iStr->Text());
  std::string* oldStr = gStrPtr.exchange(newStr);
  delete oldStr;
  *lStr = iStr->Text();
}

int main() {
  AUI* cg = AUI::Create("some app's main");
  UNUSED AWindow* w = cg->MainWnd();
  w->EnableResize();
  w->Resize(520, 165);
  w->DisableResize();
  ALabel* lPID = ALabel::AttachTo(w, "PID " + std::to_string(getpid()));
  lPID->MoveResize(10, 130, 500, 25);
  ALabel* lU32 = ALabel::AttachTo(w, "UINT32");
  lU32->MoveResize(10, 10, 80, 25);
  ALabel* lU64 = ALabel::AttachTo(w, "UINT64");
  lU64->MoveResize(10, 40, 80, 25);
  ALabel* lFl = ALabel::AttachTo(w, "float");
  lFl->MoveResize(10, 70, 80, 25);
  ALabel* lStr = ALabel::AttachTo(w, "string");
  lStr->MoveResize(10, 100, 80, 25);
  AInputBox* iU32 = AInputBox::AttachTo(w, "0");
  iU32->MoveResize(100, 10, 100, 25);
  AInputBox* iU64 = AInputBox::AttachTo(w, "0");
  iU64->MoveResize(100, 40, 100, 25);
  AInputBox* iFl = AInputBox::AttachTo(w, "1.2345");
  iFl->MoveResize(100, 70, 100, 25);
  AInputBox* iStr = AInputBox::AttachTo(w, "test");
  iStr->MoveResize(100, 100, 100, 25);
  AButton* bSet = AButton::AttachTo(w, "> Set >");
  bSet->MoveResize(210, 10, 100, 25);
  AButton* bReset = AButton::AttachTo(w, "Reset>");
  bReset->MoveResize(210, 40, 100, 25);
  AButton* bRead = AButton::AttachTo(w, "Read");
  bRead->MoveResize(320, 40, 100, 25);
  ALabel* lInmem = ALabel::AttachTo(w, "In memory >");
  lInmem->MoveResize(320, 10, 100, 25);
  ALabel* lU32V = ALabel::AttachTo(w, "None");
  lU32V->MoveResize(430, 10, 80, 25);
  ALabel* lU64V = ALabel::AttachTo(w, "None");
  lU64V->MoveResize(430, 40, 80, 25);
  ALabel* lFlV = ALabel::AttachTo(w, "None");
  lFlV->MoveResize(430, 70, 80, 25);
  ALabel* lStrV = ALabel::AttachTo(w, "None");
  lStrV->MoveResize(430, 100, 80, 25);
  UNUSED TestControls tc = {lU32V, lU64V, lFlV, lStrV, iU32, iU64, iFl, iStr};
  bSet->SetOnButtonReleaseCB(ButtonSetHandler, &tc);
  bReset->SetOnButtonReleaseCB(ButtonResetHandler, &tc);
  bRead->SetOnButtonReleaseCB(ButtonReadHandler, &tc);

  cg->ProcessMessages();
  delete cg;
  cg = nullptr;
  delete gStrPtr.exchange(nullptr);
  return 0;
  
  
  
  cg = 0;

  return 0;
}


