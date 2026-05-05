#ifndef AINPUTBOX_H_
#define AINPUTBOX_H_

#include <thread>
#include <chrono>
#include <atomic>

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
  };
}

#endif
