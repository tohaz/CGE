#ifndef ALIST_H_
#define ALIST_H_

#include "AWidget.h"

struct AWidgetSettings;

namespace aui {
  class AList : public AWidget{
    private:
      AList(AWidget *wParent);
      AList(AWidget *wParent, const AWidgetSettings& settings);
      std::vector<std::string> mData;
      std::vector<bool> mTag;
      INT64 mVOffset = 0;
      INT64 mHOffset = 0;
      UINT64 mTextY = 0; // line height in px
      bool mScrollbarsEnabled = false;
      void DrawScrollbars();
      void DrawScrollbarsArrorws();
      UINT64 mArwSz1 = AUI_LIST_ARROWSZ1; //Arrow size along the arrow line >=== [---]
      UINT64 mArwSz2 = AUI_LIST_ARROWSZ2; // and against it                 >=== IIIII
      bool mSrollbarTopHL = false;
      bool mSrollbarBottomHL = false;
      bool mSrollbarLeftHL = false;
      bool mSrollbarRightHL = false;
      bool mSingleSelect = true;
      UINT64 mCursorIndex = 0; // first element is 1, not 0
      bool mIsFocused = false;
      bool mIsDraggingV = false;
      bool mIsDraggingH = false;
      UINT64 mMaxWidthPx = 0; // Calculated in AddItem
      bool mShowVThumb = true;
      bool mShowVArrows = true;
      bool mShowHThumb = true;
      bool mShowHArrows = true;

    protected:
    public:
      UINT64 IndexFromY(UINT32 y);
      void Clear();
      static AList* AttachTo(AWidget *wParent);
      static AList* AttachTo(AWidget *wParent, const AWidgetSettings& settings);

      virtual ~AList();
      void Draw();
      void AddItem(std::string s);
      void OnButtonPress(XEvent *ev);
      void OnButtonRelease(XEvent *ev);
      void OnFocusIn(XEvent *ev);
      void OnFocusOut(XEvent *ev);
      void OnKeyPress(XEvent *ev);
      void OnMouseMove(XEvent *ev);
      void ScrollDownPx(INT64 px);
      void ScrollUpPx(INT64 px);
      void ScrollLeftPx(INT64 px);
      void ScrollRightPx(INT64 px);
      void EnableScrollbars();
      void ShowVerticalScroll(bool thumb, bool arrows);
      void ShowHorizontalScroll(bool thumb, bool arrows);
      void SetCursorPosition(UINT64 line);
      UINT64 CursorPos();
      std::string DataAtCursor();
      std::string DataAt(UINT64 line);
      UINT64 SelectedIndex();
      void SetSelectedIndex(UINT64 ind);
  };
}

#endif
