#ifndef ALIST_H_
#define ALIST_H_

#include "AWidget.h"

namespace aui {
  class AList : public AWidget{
    private:
      AList(AWidget *wParent);
      std::vector<std::string> mData;
      std::vector<bool> mTag;
      UINT64 IndexFromY(UINT32 y);
      INT64 mVOffset = 0;
      INT64 mHOffset = 0;
      UINT64 mLongestItem = 1;
      UINT64 mTextY = 0; // line height in px
      bool mScrollbarsEnabled = false;
      void DrawScrollbars();
      void DrawScrollbarsArrorws();
      void DrawScrollbarsIndicators();
      UINT64 mArwSz1 = AUI_LIST_ARROWSZ1; //Arrow size along the arrow line >=== [---]
      UINT64 mArwSz2 = AUI_LIST_ARROWSZ2; // and against it                 >=== IIIII
      bool mSrollbarTopHL = false;
      bool mSrollbarBottomHL = false;
      bool mSrollbarLeftHL = false;
      bool mSrollbarRightHL = false;
      bool mSingleSelect = true;
      UINT64 mSelectIndex = 0; // first element is 1, not 0

    protected:
    public:
      static AList* AttachTo(AWidget *wParent);
      virtual ~AList();
      void Draw();
      void AddItem(std::string s);
      void OnButtonPress(XEvent *ev);
      void OnButtonRelease(XEvent *ev);
      void ScrollDownPx(INT64 px);
      void ScrollUpPx(INT64 px);
      void ScrollLeftPx(INT64 px);
      void ScrollRightPx(INT64 px);
      void EnableScrollbars();
  };
}

#endif
