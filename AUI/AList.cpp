#include "AList.h"

#include "AUILib.h"

namespace aui {

  AList::AList(AWidget *wParent) {
    D2()
    AUI *cg = wParent->AUIPtr();
    Display *d = cg->Disp();
    INT32 scr = cg->Scr();
    SetType(AUIWidgetType::defaultList);
    SetBGColor(AUI_LIST_BG);
    SetXY(AUI_LIST_X, AUI_LIST_Y);
    SetSizeXY(AUI_LIST_SZX, AUI_LIST_SZY);
    SetAUIPtr(cg);
    mVOffset = 0;
    mHOffset = 0;
    SetWndParent(wParent);
    InitWidgetProps(
        XCreateSimpleWindow(d, wParent->Wnd(), SafeINT32(X()), SafeINT32(Y()),
            SafeUINT32(SizeX()), SafeUINT32(SizeY()), 1, BlackPixel(d, scr),
            BGColor()));
    Window w = Wnd();
    XSelectInput(d, w,
        ExposureMask | ButtonPressMask | KeyPressMask | ButtonReleaseMask);
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
    XFontStruct *fo = Font();
    Display *d = AUIPtr()->Disp();
    D2("list size in strings %lu, offset %ld", numStringsData, mVOffset)
    XTextExtents(fo, "123456789WTL", 12, &direction, &ascent, &descent,
        &overall);
    D3("text extents direction %u, ascent %u, descent %u", direction, ascent,
        descent)
    mTextY = SafeUINT64(ascent + descent);
    if(mData.size() > 0) {
      if(mTextY == 0)
        E("font reported zero height");

      numStringsMax = sizeY / mTextY + 1;
      D2("num strings to display max %lu", numStringsMax)
      if(numStringsMax > numStringsData) {
        numStringsMax = numStringsData;
        D2("reducing number of displayed strings to %lu", numStringsMax)
      } else
        D2("not reducing number of displayed strings, data %lu", numStringsData)
      UINT64 sBegin = 0; // index of first string to draw
      UINT64 sEnd = 0; // last to draw
      if(mVOffset > 0) {
        sBegin = SafeUINT64(mVOffset) / mTextY;
        D2("applying offset %lu, begin string %lu", mVOffset, sBegin)
      } else
        D2("not applying offset")
      if(sBegin > numStringsData - 1) {
        D2("scroll down limit")
        sBegin = numStringsData - 1;
        mVOffset = SafeINT64(mTextY * sBegin);
      }
      sEnd = sBegin + numStringsMax;
      if(sEnd > numStringsData - 1) {
        sEnd = numStringsData - 1;
      }
      D3("drawing strings from %lu to %lu", sBegin, sEnd)
      XClearWindow(d, wi);
      for (UINT64 i = sBegin; i <= sEnd; i++) {
        if(mTag[i])
          XSetBackground(d, gc, 0xAACCAA);
        else
          XSetBackground(d, gc, AUI_LIST_BG);
        XDrawImageString(d, wi, gc, SafeINT32(-mHOffset),
            SafeINT32(mTextY * (i + 1)) - SafeINT32(mVOffset), mData[i].c_str(),
            SafeINT32(mData[i].size()));
      }
      if(mScrollbarsEnabled)
        DrawScrollbars();
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
    Display *d = AUIPtr()->Disp();
    GC gc = GCPtr();
    Window wi = Wnd();
    UINT64 sizeX = SizeX();
    UINT64 sizeY = SizeY();
    UINT64 mArwSz1P = mArwSz1 / 5;
    UINT64 mArwSz2P = mArwSz2 / 5;
    UINT64 mArwSz1H = mArwSz1 / 2;
    UINT64 mArwSz2H = mArwSz2 / 2;
    XSetLineAttributes(d, gc, SafeUINT32(mArwSz1 / 4), LineSolid, CapRound,
        JoinMiter);
    XSetForeground(d, gc, AUI_LIST_FG_COLOR);
    if(mSrollbarTopHL) {
      XSetForeground(d, gc, 0xAAAAAA);
    }
    XDrawLine(d, wi, gc, SafeINT32(sizeX - mArwSz2 - mArwSz2P),
        SafeINT32(mArwSz1H + mArwSz1P), SafeINT32(sizeX - mArwSz2H - mArwSz2P),
        SafeINT32(mArwSz1P));
    XDrawLine(d, wi, gc, SafeINT32(sizeX - mArwSz2H - mArwSz2P),
        SafeINT32(mArwSz1P), SafeINT32(sizeX - mArwSz2P),
        SafeINT32(mArwSz1H + mArwSz1P));
    if(mSrollbarBottomHL) {
      XSetForeground(d, gc, 0xAAAAAA);
    } else {
      XSetForeground(d, gc, AUI_LIST_FG_COLOR);
    }
    XDrawLine(d, wi, gc, SafeINT32(sizeX - mArwSz2 - mArwSz2P),
        SafeINT32(sizeY - mArwSz1 - mArwSz1H - mArwSz1P),
        SafeINT32(sizeX - mArwSz2H - mArwSz2P),
        SafeINT32(sizeY - mArwSz1 - mArwSz1P));
    XDrawLine(d, wi, gc, SafeINT32(sizeX - mArwSz2H - mArwSz2P),
        SafeINT32(sizeY - mArwSz1 - mArwSz1P), SafeINT32(sizeX - mArwSz2P),
        SafeINT32(sizeY - mArwSz1 - mArwSz1H - mArwSz1P));
    if(mSrollbarRightHL) {
      XSetForeground(d, gc, 0xAAAAAA);
    } else {
      XSetForeground(d, gc, AUI_LIST_FG_COLOR);
    }
    XDrawLine(d, wi, gc, SafeINT32(sizeX - mArwSz1 - mArwSz1H - mArwSz1P),
        SafeINT32(sizeY - mArwSz2 - mArwSz2P),
        SafeINT32(sizeX - mArwSz1 - mArwSz1P),
        SafeINT32(sizeY - mArwSz2H - mArwSz2P));
    XDrawLine(d, wi, gc, SafeINT32(sizeX - mArwSz1 - mArwSz1P),
        SafeINT32(sizeY - mArwSz2H - mArwSz2P),
        SafeINT32(sizeX - mArwSz1 - mArwSz1H - mArwSz1P),
        SafeINT32(sizeY - mArwSz2P));
    if(mSrollbarLeftHL) {
      XSetForeground(d, gc, 0xAAAAAA);
    } else {
      XSetForeground(d, gc, AUI_LIST_FG_COLOR);
    }
    XDrawLine(d, wi, gc, SafeINT32(mArwSz1H + mArwSz1P),
        SafeINT32(sizeY - mArwSz2 - mArwSz2P), SafeINT32(mArwSz1P),
        SafeINT32(sizeY - mArwSz2H - mArwSz2P));
    XDrawLine(d, wi, gc, SafeINT32(mArwSz1P),
        SafeINT32(sizeY - mArwSz2H - mArwSz2P), SafeINT32(mArwSz1H + mArwSz1P),
        SafeINT32(sizeY - mArwSz2P));
    if(mSrollbarTopHL || mSrollbarBottomHL || mSrollbarLeftHL
        || mSrollbarRightHL) {
      XSetForeground(d, gc, AUI_LIST_FG_COLOR);
    }
  }

  void AList::DrawScrollbarsIndicators() {
    Display *d = AUIPtr()->Disp();
    Window wi = Wnd();
    GC gc = GCPtr();
    UINT64 sizeX = SizeX();
    UINT64 sizeY = SizeY();

    // drawing screen position bar
    double fullrange = static_cast<double>(sizeY)
        - static_cast<double>(mArwSz1 * 3);
    double allStringsInPx = static_cast<double>((mData.size() + 1) * mTextY
        + (sizeY - mTextY * 2));
    double draw_start = ((double) mVOffset / (double) allStringsInPx)
        * fullrange;
    double draw_size = ((double) sizeY / (double) allStringsInPx) * fullrange;
    if(fullrange > 0) {
      D3(
          "vscrollbar full range %.2f, draw start at %.2f, size %.2f, mVoffset %lu",
          fullrange, draw_start, draw_size, mVOffset)
    } else
      E("scrollbar fullrange became negative")
    if((mArwSz1 * 5) > sizeY) {
      D("scrollbar (arrow size 1) is too big, it's unreasonable")
      return;
    }
    XDrawLine(d, wi, gc,
        SafeINT32(
            (INT64) ((double) sizeX - (double) mArwSz2 / 2.0
                - (double) mArwSz2 / 5.0)),
        SafeINT32((INT64) ((double) mArwSz1 + draw_start)),
        SafeINT32(
            (INT64) ((double) sizeX - (double) mArwSz2 / 2.0
                - (double) mArwSz2 / 5.0)),
        SafeINT32((INT64) ((double) mArwSz1 + draw_start + draw_size)));
  }

  void AList::OnButtonRelease([[maybe_unused]]XEvent *ev) {
    // TODO: List in Examples also lack this
    D("OnButtonRelease Unimplemented")
  }

  void AList::OnButtonPress(XEvent *ev) {
    Display *d = AUIPtr()->Disp();
    Window wi = Wnd();
    UINT64 sizeX = SizeX();
    UINT64 sizeY = SizeY();

    // Xlib requires standard 'int' for coordinates
    int rx, ry, loc_x, loc_y;
    Window root_window, child_window;
    UINT32 mask = 0;
    root_window = DefaultRootWindow(d);
    D3();
    // Pass pointers to actual ints
    XQueryPointer(d, wi, &root_window, &child_window, &rx, &ry, &loc_x, &loc_y,
        &mask);
    // Convert these back to your internal types safely
    UINT64 lx = SafeUINT64(loc_x);
    UINT64 ly = SafeUINT64(loc_y);
    D2("click local coords: x %lu y %lu", lx, ly)
    switch (ev->xbutton.button) {
      case Button4:
        ScrollUpPx(SafeINT64(mTextY * 3));
        break;
      case Button5:
        ScrollDownPx(SafeINT64(mTextY * 3));
        break;
      case Button1:
        if(mScrollbarsEnabled) {
          if(lx >= sizeX - mArwSz2 - mArwSz2 / 5 && ly >= mArwSz1 / 5
              && lx < sizeX - mArwSz2 / 5 && ly <= mArwSz1 - mArwSz1 / 5) {
            mSrollbarTopHL = true;
            ScrollUpPx(SafeINT64(sizeY / 3));
            return;
          }
          if(lx >= sizeX - mArwSz2 - mArwSz2 / 5 && ly >= sizeY - (mArwSz2 * 2)
              && lx < sizeX - mArwSz2 / 5 && ly <= sizeY - mArwSz1) {
            mSrollbarBottomHL = true;
            ScrollDownPx(SafeINT64(sizeY / 3));
            return;
          }
          if(lx >= mArwSz1 / 5 && ly <= sizeY - mArwSz2 / 5
              && lx <= mArwSz1 + mArwSz1 / 5
              && ly >= sizeY - mArwSz1 - mArwSz1 / 5) {
            mSrollbarLeftHL = true;
            ScrollLeftPx(SafeINT64(sizeX / 3));
            return;
          }
          if(lx >= sizeX - (mArwSz1 * 2) - mArwSz1 / 5
              && ly >= sizeY - mArwSz2 - mArwSz2 / 5 && lx <= sizeX - mArwSz1
              && ly <= sizeY - mArwSz2 / 5) {
            mSrollbarRightHL = true;
            ScrollRightPx(SafeINT64(sizeX / 3));
            return;
          }
          // Body click
          if(lx > 0 && lx < (sizeX - mArwSz2 - mArwSz2 / 5) && ly > 0
              && ly < (sizeY - mArwSz2 - mArwSz2 / 5)) {
            mSelectIndex = IndexFromY(SafeUINT32(ly)); // Safe conversion to UINT32
            if(mSelectIndex > 0) {
              if(mSingleSelect)
                std::fill(mTag.begin(), mTag.end(), false);
              mTag[mSelectIndex - 1] = !mTag[mSelectIndex - 1];
            }
          }
        }
        break;
      default:
        D("x")
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
    } else {
      mVOffset -= px;
    }
    Draw();
  }

  void AList::ScrollLeftPx(INT64 px) {
    D2()
    if(mHOffset <= px) {
      mHOffset = 0;
    } else {
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
    UINT64 fullvcoord = SafeUINT64(mVOffset) + ((UINT64)y);
    UINT64 ind = fullvcoord / mTextY;
    if(ind > mData.size()) {
      D3("Ends")
      return 0;
    }
    if((fullvcoord % mTextY) > 0 && ind < mData.size())
      ind++;
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

