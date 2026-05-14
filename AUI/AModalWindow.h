#ifndef AMODALWINDOW_H_
#define AMODALWINDOW_H_

#include <string>
#include <vector>
#include "AWidget.h"
#include "AButton.h"

namespace aui {
  class AModalWindow;

  class AModalWindow : public AWidget {
    private:
      AModalWindow(AWidget* wParent, std::string title, std::string text);
      AButton* mOkBtn = nullptr;
      std::string mMessage;
      std::vector<std::string> mWrappedLines;
      INT32 mTitleHeight = 28;
      INT32 mPadding = 20;
      INT32 mDepth = 3;
      INT32 mBaseX = 0;
      INT32 mBaseY = 0;
      INT32 mDragOffsetX = 0;
      INT32 mDragOffsetY = 0;
      bool mIsShaking = false;
      bool mIsDragging = false;
      static void OnOkClick(XEvent* ev, AWidget* w, void* data);
      INT32 mParentAbsX = 0;
      INT32 mParentAbsY = 0;
    public:
      static void Message(AWidget* wParent, std::string title, std::string text);
      static AModalWindow* AttachTo(AWidget* wParent, std::string title, std::string text);
      virtual ~AModalWindow();
      void CalculateLayout(UINT32& outW, UINT32& outH);
      void Draw() override;
      void TriggerFeedback() override;
      void Close();
      void OnButtonPress(XEvent* ev);
      void OnMouseMove(XEvent* ev);
      void OnButtonRelease(XEvent* ev);
      void WrapText(UINT32 maxWidth);
      const std::vector<std::string>& WrappedLines() const;
      bool IsDragging() const {return mIsDragging;}
  };
}
#endif
