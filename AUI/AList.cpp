#include "AList.h"

#include "AUILib.h"

namespace aui {

  AList::AList(AWidget *wParent) {
    D2()
    AUI *cg = wParent->AUIPtr();
    Display* d = cg->Disp();
    UINT32 scr = cg->Scr();
    SetType(AUIWidgetType::defaultList);
    SetBGColor(AUI_LIST_BG);
    SetXY(AUI_LIST_X, AUI_LIST_Y);
    SetSizeXY(AUI_LIST_SZX, AUI_LIST_SZY);
    SetAUIPtr(cg);
    mVOffset = 0;
    mHOffset = 0;
    SetWndParent(wParent);
    InitWidgetProps(XCreateSimpleWindow(d, wParent->Wnd(), X(), Y(), SizeX(), SizeY(), 1,
        BlackPixel(d, scr), BGColor()));
    Window w = Wnd();
    XSelectInput(d, w, ExposureMask | ButtonPressMask | KeyPressMask | ButtonReleaseMask);
    XMapWindow(d, w);
    EnableResize();
    cg->AddWidget(this);
    D2("listview: disp=%lu, wnd=%lu, scr=%d", (INT64) d, (INT64) w, scr)
  }

  AList* AList::AttachTo(AWidget *wParent) {
    return new AList(wParent);
  }

  void AList::Draw() {
    XCharStruct overall;
    INT32 direction, ascent, descent;
    UINT64 numStringsMax = 0, // max lines to fit vertically
    numStringsData = mData.size();
    Window wi = Wnd();
    UINT64 sizeY = SizeY();
    GC gc = GCPtr();
    XFontStruct* fo = Font();
    Display* d = AUIPtr()->Disp();
    D2("list size in strings %lu, offset %ld", numStringsData, mVOffset)
    XTextExtents(fo, "123456789WTL", 12, &direction, &ascent, &descent, &overall);
    D3("text extents direction %u, ascent %u, descent %u", direction, ascent, descent)
    mTextY = ascent + descent;
    if(mData.size() > 0) {
       if(mTextY == 0) E("font reported zero height");

       numStringsMax = sizeY / mTextY + 1;
       D2("num strings to display max %lu", numStringsMax)
       if(numStringsMax > numStringsData) {
         numStringsMax = numStringsData;
         D2("reducing number of displayed strings to %lu", numStringsMax)
       }
       else D2("not reducing number of displayed strings, data %lu", numStringsData)
       UINT64 sBegin = 0; // index of first string to draw
       UINT64 sEnd = 0; // last to draw
       if(mVOffset > 0) {
         sBegin = mVOffset / mTextY;
         D2("applying offset %lu, begin string %lu", mVOffset, sBegin)
       }
       else D2("not applying offset")
       if(sBegin > numStringsData - 1) {
         D2("scroll down limit")
         sBegin = numStringsData - 1;
         mVOffset = mTextY * sBegin;
       }
       sEnd = sBegin + numStringsMax;
       if(sEnd > numStringsData - 1) {
          sEnd = numStringsData - 1;
       }
       D3("drawing strings from %lu to %lu", sBegin, sEnd)
       XClearWindow(d, wi);
       for(UINT64 i = sBegin; i <= sEnd; i++) {
         if(mTag[i]) XSetBackground(d, gc, 0xAACCAA);
            else XSetBackground(d, gc, AUI_LIST_BG);
         XDrawImageString(d, wi, gc, -mHOffset, mTextY * (i + 1) - mVOffset,
             mData[i].c_str(), mData[i].size());
       }
       if(mScrollbarsEnabled) DrawScrollbars();
    }
  }

  void AList::AddItem(std::string s) {
    D3()
    if(s.size() > mLongestItem) {
      mLongestItem = s.size();
      D2("changing longest item to %lu", mLongestItem)
    }
    mData.push_back(s);
    mTag.push_back(false);
    #if DEBUG_LEVEL > 1
      for (const std::string& ss : mData) {
        D1("content:%s", ss.c_str())
      }
    #endif
  }

  void AList::DrawScrollbars() {
    DrawScrollbarsArrorws();
    DrawScrollbarsIndicators();
  }

  void AList::DrawScrollbarsArrorws() {
    D2()
    Display* d = AUIPtr()->Disp();
    GC gc = GCPtr();
    Window wi = Wnd();
    UINT64 sizeX = SizeX();
    UINT64 sizeY = SizeY();
    UINT64 mArwSz1P = mArwSz1 / 5;
    UINT64 mArwSz2P = mArwSz2 / 5;
    UINT64 mArwSz1H = mArwSz1 / 2;
    UINT64 mArwSz2H = mArwSz2 / 2;
    XSetLineAttributes(d, gc, mArwSz1 / 4, LineSolid, CapRound, JoinMiter);
    XSetForeground(d, gc, AUI_LIST_FG_COLOR);
    if(mSrollbarTopHL) {
      XSetForeground(d, gc, 0xAAAAAA);
    }
    XDrawLine(d, wi, gc, sizeX - mArwSz2 - mArwSz2P, mArwSz1H + mArwSz1P,
        sizeX - mArwSz2H - mArwSz2P, mArwSz1P);
    XDrawLine(d, wi, gc, sizeX - mArwSz2H - mArwSz2P, mArwSz1P,
        sizeX - mArwSz2P, mArwSz1H + mArwSz1P);
    if(mSrollbarBottomHL) {
      XSetForeground(d, gc, 0xAAAAAA);
    }
    else {
      XSetForeground(d, gc, AUI_LIST_FG_COLOR);
    }
    XDrawLine(d, wi, gc, sizeX - mArwSz2 - mArwSz2P, sizeY - mArwSz1 - mArwSz1H - mArwSz1P,
        sizeX - mArwSz2H - mArwSz2P, sizeY - mArwSz1 - mArwSz1P);
    XDrawLine(d, wi, gc, sizeX - mArwSz2H - mArwSz2P, sizeY - mArwSz1 - mArwSz1P,
        sizeX - mArwSz2P, sizeY - mArwSz1 - mArwSz1H - mArwSz1P);
    if(mSrollbarRightHL) {
      XSetForeground(d, gc, 0xAAAAAA);
    }
    else {
      XSetForeground(d, gc, AUI_LIST_FG_COLOR);
    }
    XDrawLine(d, wi, gc, sizeX - mArwSz1 - mArwSz1H - mArwSz1P, sizeY - mArwSz2 - mArwSz2P,
        sizeX - mArwSz1 - mArwSz1P, sizeY - mArwSz2H - mArwSz2P);
    XDrawLine(d, wi, gc, sizeX - mArwSz1 - mArwSz1P, sizeY - mArwSz2H - mArwSz2P,
        sizeX - mArwSz1 - mArwSz1H - mArwSz1P, sizeY - mArwSz2P);
    if(mSrollbarLeftHL) {
      XSetForeground(d, gc, 0xAAAAAA);
    }
    else {
      XSetForeground(d, gc, AUI_LIST_FG_COLOR);
    }
    XDrawLine(d, wi, gc, mArwSz1H + mArwSz1P, sizeY - mArwSz2 - mArwSz2P,
        mArwSz1P, sizeY - mArwSz2H - mArwSz2P);
    XDrawLine(d, wi, gc, mArwSz1P, sizeY - mArwSz2H - mArwSz2P,
        mArwSz1H + mArwSz1P, sizeY - mArwSz2P);
    if(mSrollbarTopHL || mSrollbarBottomHL || mSrollbarLeftHL || mSrollbarRightHL) {
      XSetForeground(d, gc, AUI_LIST_FG_COLOR);
    }
  }

  void AList::DrawScrollbarsIndicators() {
    Display* d = AUIPtr()->Disp();
    Window wi = Wnd();
    GC gc = GCPtr();
    UINT64 sizeX = SizeX();
    UINT64 sizeY = SizeY();

    // drawing screen position bar
    double fullrange = sizeY - mArwSz1 * 3;
    double allStringsInPx = (mData.size() + 1) * mTextY + (sizeY - mTextY * 2);
    double draw_start = ((double) mVOffset / (double)allStringsInPx) * fullrange;
    double draw_size = ((double)sizeY / (double)allStringsInPx) * fullrange;
    if(fullrange > 0) {
      D3("vscrollbar full range %.2f, draw start at %.2f, size %.2f, mVoffset %lu", fullrange,
          draw_start, draw_size, mVOffset)
    }
    else E("scrollbar fullrange became negative")
    if((mArwSz1 * 5) > sizeY) {
      D("scrollbar (arrow size 1) is too big, it's unreasonable")
      return;
    }
    XDrawLine(d, wi, gc, sizeX - mArwSz2 / 2 - mArwSz2 / 5, mArwSz1 + draw_start,
        sizeX - mArwSz2 / 2 - mArwSz2 / 5, mArwSz1 + draw_start + draw_size);
  }
  void AList::OnButtonRelease(XEvent *ev) {
    // TODO: List in Examples also lack this
    D3("OnButtonRelease Unimplemented")
  }

  void AList::OnButtonPress(XEvent *ev) {
    Display* d = AUIPtr()->Disp();
    Window wi = Wnd();
    UINT64 sizeX = SizeX();
    UINT64 sizeY = SizeY();
    INT32 root_x, root_y, lx, ly;
    Window root_window, child_window;
    UINT32 mask = 0;
    root_window = DefaultRootWindow(d);
    D3();
    XQueryPointer(d, wi, &root_window, &child_window,
        &root_x, &root_y, &lx, &ly, &mask);
    D2("click local coords: x %d y %d", lx, ly)
    D2("sizeX = %lu, sizeY = %lu, mX = %lu, mY = %lu", sizeX, sizeY, X(), Y())
    switch(ev->xbutton.button) {
      case Button4:
        ScrollUpPx(mTextY * 3);
        break;
      case Button5:
        ScrollDownPx(mTextY * 3);
        break;
      case Button1:
           if(mScrollbarsEnabled) {
             D2("Scrollbars enabled");
             if((UINT64)lx >= sizeX - mArwSz2 - mArwSz2 / 5 && (UINT64)ly >= mArwSz1 / 5
                 && (UINT64)lx < sizeX - mArwSz2 / 5 && (UINT64)ly <= mArwSz1 - mArwSz1 / 5) {
               D2("V Arrow top clicked")
               mSrollbarTopHL = true;
               ScrollUpPx(sizeY / 3);
               return;
             }
             if((UINT64)lx >= sizeX - mArwSz2 - mArwSz2 / 5 && (UINT64)ly >= sizeY - (mArwSz2 * 2)
                 && (UINT64)lx < sizeX - mArwSz2 / 5 && (UINT64)ly <= sizeY - mArwSz1) {
               D2("V Arrow bottom clicked")
               mSrollbarBottomHL = true;
               ScrollDownPx(sizeY / 3);
               return;
             }
             if((UINT64)lx >= mArwSz1 / 5 && (UINT64)ly <= sizeY - mArwSz2 / 5 && (UINT64)lx <= mArwSz1 + mArwSz1 / 5
                 && (UINT64)ly >= sizeY - mArwSz1 - mArwSz1 / 5) {
               D2("V Arrow left clicked")
               mSrollbarLeftHL = true;
               ScrollLeftPx(sizeX / 3);
               return;
             }
             if((UINT64)lx >= sizeX - (mArwSz1 * 2) - mArwSz1 / 5 && (UINT64)ly >= sizeY -  mArwSz2 - mArwSz2 / 5
                 && (UINT64)lx <= sizeX - mArwSz1 && (UINT64)ly <= sizeY - mArwSz2 / 5) {
               D2("V Arrow right clicked")
               mSrollbarRightHL = true;
               ScrollRightPx(sizeX / 3);
               return;
             }
             if((UINT64)lx > 0 && (UINT64)lx < (sizeX - mArwSz2 - mArwSz2 / 5) &&
                 (UINT64)ly > 0 && (UINT64)ly < (sizeY - mArwSz2 - mArwSz2 / 5)) {
               D3("inside listview clicked")
               mSelectIndex = IndexFromY(ly);
               if(mSelectIndex > 0) {
                 if(mSingleSelect) {
                   std::fill(mTag.begin(), mTag.end(), false);
                 }
                 mTag[mSelectIndex - 1] = !mTag[mSelectIndex - 1];
               }
             }
           }
           else D2("Scrollbars disabled")
           break;
       default:
         D3("unknown button pressed")
         break;
     }
  }

  void AList::ScrollDownPx(INT64 px) {
    D2()
    mVOffset += px;
    Draw();
  }

  void AList::ScrollUpPx(INT64 px) {
    D2()
    if(mVOffset <= px) {
      mVOffset = 0;
    }
    else {
      mVOffset -= px;
    }
    Draw();
  }

  void AList::ScrollLeftPx(INT64 px) {
    D2()
    if(mHOffset <= px) {
      mHOffset = 0;
    }
    else {
      mHOffset -= px;
    }
    Draw();
  }

  void AList::ScrollRightPx(INT64 px) {
    mHOffset += px;
    Draw();
  }

  UINT64 AList::IndexFromY(UINT32 y) {
    D3()
    UINT64 fullvcoord = mVOffset + y;
    UINT64 ind = fullvcoord / mTextY;
    if(ind > mData.size()) {
      D3("Ends")
      return 0;
    }
    if((fullvcoord % mTextY) > 0 && ind < mData.size()) ind++;
    D3("Ends")
    return ind;
  }

  void AList::EnableScrollbars() {
    D2()
    mScrollbarsEnabled = true;
    Draw();
  }


  AList::~AList() {
    mData.clear();
    mTag.clear();
  }
}

