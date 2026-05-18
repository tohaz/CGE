#ifndef AINPUTBOX_H_
#define AINPUTBOX_H_

#include <thread>
#include <chrono>
#include <atomic>
#include <regex>
#include <condition_variable>
#include "AWidget.h"

namespace aui {
  class AInputBox : public AWidget{
    private:
      AInputBox(AWidget* wParent);
      std::string mFilterStr = R"([a-zA-Z0-9.])";
      std::regex mFilter;
      UINT64 mCursorPos = 0;
      UINT64 mCursorW = AUI_DEFAULT_INPUT_CURSORW;
      UINT64 mCursorH = AUI_DEFAULT_INPUT_CURSORH;
      std::thread mBlinkThread;
      std::atomic<bool> mStopBlink{false};
      bool mCursorVisible = true;
      bool mIsFocused = false;
      std::function<void(AWidget* w, void* data)> OnValueChanged = nullptr;
      void* mUserDataValueChanged = nullptr;
      INT32 mInnerInset = 3;
      std::mutex mBlinkMutex;
      std::condition_variable mBlinkCv;
      bool mIsEditable = true;

    protected:
    public:
      static AInputBox* AttachTo(AWidget* wParent);
      static AInputBox* AttachTo(AWidget* wParent, std::string value);
      virtual ~AInputBox();
      void Draw();
      void OnBackSpace();
      void OnKeyPress(XEvent* ev);
      void OnButtonPress(XEvent* ev);
      void OnButtonRelease(XEvent* ev);
      void OnFocusIn([[maybe_unused]] XEvent* ev);
      void OnFocusOut([[maybe_unused]] XEvent* ev);
      void SetInputFilter(std::string f);
//      virtual void SetText(std::string value);
      void SetOnValueChangedCB(std::function<void(AWidget* w, void* arbdata)> func, void* data);
      void Enable();
      void Disable();
      void SetEditable(bool state) { mIsEditable = state; }
      bool IsEditable() const { return mIsEditable; }
      UINT64 CursorPos() const {return mCursorPos;}
      void SetCursorPos(UINT64 cursorPos = 0) {mCursorPos = cursorPos;}
  };
}

#endif
