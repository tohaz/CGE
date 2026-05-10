#ifndef AINPUTBOX_H_
#define AINPUTBOX_H_

#include <thread>
#include <chrono>
#include <atomic>

#include "AWidget.h"

namespace aui {
  void draw_3d_baguette(Display *dpy, Window win, GC gc, int x, int y, int w, int h);

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
      virtual void SetText(std::string value);
      void SetOnValueChangedCB(std::function<void(AWidget* w, void* arbdata)> func, void* data);
  };
}

#endif
