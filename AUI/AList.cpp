#include "AList.h"
//
#include "AUILib.h"

namespace aui {

  AList::AList(AWidget *wParent) {
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
    XSelectInput(d, Wnd(),
        ExposureMask | ButtonPressMask | KeyPressMask | ButtonReleaseMask
            | FocusChangeMask | PointerMotionMask);
    XMapWindow(d, Wnd());
    EnableResize();
    cg->AddWidget(this);
    if (wParent) {
      wParent->AddWidgetChild(this);
    }
  }

  AList::AList(AWidget *wParent, const AWidgetSettings& settings) {
     AUI *cg = wParent->AUIPtr();
     Display *d = cg->Disp();
     INT32 scr = cg->Scr();
     SetType(settings.type != AUIWidgetType::unset ? settings.type : AUIWidgetType::defaultList);
     SetBGColor(settings.backgroundColor);
     SetXY(settings.x, settings.y);
     SetSizeXY(settings.width, settings.height);
     SetAUIPtr(cg);
     mVOffset = 0;
     mHOffset = 0;
     SetWndParent(wParent);
     // HARDWARE FIX 2: If startVisible is false, it's an unmanaged dropdown layer.
     // We parent it to XRootWindow so it can move anywhere and receive hardware click events.
     Window nativeParent = settings.startVisible ? wParent->Wnd() : XRootWindow(d, scr);
     InitWidgetProps(
         XCreateSimpleWindow(d, nativeParent, SafeINT32(X()), SafeINT32(Y()),
             SafeUINT32(SizeX()), SafeUINT32(SizeY()), 1, BlackPixel(d, scr),
             BGColor()));
     XSelectInput(d, Wnd(),
         ExposureMask | ButtonPressMask | KeyPressMask | ButtonReleaseMask
             | FocusChangeMask | PointerMotionMask);
     if (settings.startVisible) {
       XMapWindow(d, Wnd());
     }
     EnableResize();
     cg->AddWidget(this);
  }

  AList* AList::AttachTo(AWidget *wParent) {
    return new AList(wParent);
  }

  AList* AList::AttachTo(AWidget *wParent, const AWidgetSettings& settings) {
    return new AList(wParent, settings);
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
     D2("list size in strings {}, offset {}", numStringsData, mVOffset)
     XTextExtents(fo, "123456789WTL", 12, &direction, &ascent, &descent,
         &overall);
     D3("text extents direction {}, ascent {}, descent {}", direction, ascent,
         descent)
     mTextY = SafeUINT64(ascent + descent);
     if(mData.size() > 0) {
       if(mTextY == 0)
         {E("font reported zero height");}
       numStringsMax = sizeY / mTextY + 1;
       D2("num strings to display max {}", numStringsMax)
       if(numStringsMax > numStringsData) {
         numStringsMax = numStringsData;
         D2("reducing number of displayed strings to {}", numStringsMax)
       } else
         D2("not reducing number of displayed strings, data {}", numStringsData)
       UINT64 sBegin = 0; // index of first string to draw
       UINT64 sEnd = 0; // last to draw
       if(mVOffset > 0) {
         sBegin = SafeUINT64(mVOffset) / mTextY;
         D2("applying offset {}, begin string {}", mVOffset, sBegin);
       } else
         D2("not applying offset");
       if(sBegin > numStringsData - 1) {
         D2("scroll down limit")
         sBegin = numStringsData - 1;
         mVOffset = SafeINT64(mTextY * sBegin);
       }
       sEnd = sBegin + numStringsMax;
       if(sEnd > numStringsData - 1) {
         sEnd = numStringsData - 1;
       }
       D3("drawing strings from {} to {}", sBegin, sEnd)
       XSetForeground(d, gc, AUI_LIST_BG);
       XFillRectangle(d, wi, gc, 0, 0, static_cast<unsigned int>(SizeX()), static_cast<unsigned int>(SizeY()));
       XSetForeground(d, gc, BlackPixel(d, AUIPtr()->Scr()));
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
    XFontStruct *f = Font();
    if(f) {
      // XTextWidth only needs the font struct, not the Display*
      UINT64 width = SafeUINT64(
          XTextWidth(f, s.c_str(), static_cast<int>(s.size())));
      if(width > mMaxWidthPx) {
        mMaxWidthPx = width;
        D2("New max width: {} px", mMaxWidthPx)
      }
    } else {
      E("AList::AddItem - Font not initialized");
    }
    mData.push_back(s);
    mTag.push_back(false);
  }

  void AList::ScrollDownPx(INT64 px) {
    double totalHeight = static_cast<double>(mData.size() * mTextY);
    double visibleHeight = static_cast<double>(SizeY());
    // Extend max scroll by the horizontal scrollbar's thickness * 2
    INT64 maxV = SafeINT64(
        totalHeight - visibleHeight + static_cast<double>(mArwSz2 * 2));
    mVOffset += px;
    if(mVOffset > maxV)
      mVOffset = (maxV > 0) ? maxV : 0;
    Draw();
  }

  void AList::ScrollRightPx(INT64 px) {
    double totalWidth = static_cast<double>(mMaxWidthPx);
    double visibleWidth = static_cast<double>(SizeX());
    // Extend max scroll by the vertical scrollbar's thickness * 2
    INT64 maxH = SafeINT64(
        totalWidth - visibleWidth + static_cast<double>(mArwSz2 * 2));
    mHOffset += px;
    if(mHOffset > maxH)
      mHOffset = (maxH > 0) ? maxH : 0;
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

  UINT64 AList::IndexFromY(UINT32 y) {
    // If y was a wrapped negative, it will be huge.
    // Check against SizeY() to catch any leaked bad data.
    if(y > SizeY())
      return 0;
    UINT64 fullvcoord = SafeUINT64(mVOffset) + y;
    UINT64 ind = fullvcoord / mTextY;
    if(ind >= mData.size()) {
      return 0;
    }
    // Using 1-based index for your logic
    return ind + 1;
  }

  void AList::EnableScrollbars() {
    D2()
    mScrollbarsEnabled = true;
    Draw();
  }

  void AList::OnFocusIn(UNUSED XEvent *ev) {
    mIsFocused = true;
    Draw();
  }

  void AList::OnFocusOut(UNUSED XEvent *ev) {
    mIsFocused = false;
    mSrollbarTopHL = false;
    mSrollbarBottomHL = false;
    mSrollbarLeftHL = false;
    mSrollbarRightHL = false;
    Draw();
  }

  void AList::OnKeyPress(XEvent *ev) {
    KeySym keysym = XLookupKeysym(&ev->xkey, 0);
    switch (keysym) {
      case XK_Up:
        ScrollUpPx(SafeINT64(mTextY));
        break;
      case XK_Down:
        ScrollDownPx(SafeINT64(mTextY));
        break;
      case XK_Page_Up:
        ScrollUpPx(SafeINT64(SizeY()));
        break;
      case XK_Page_Down:
        ScrollDownPx(SafeINT64(SizeY()));
        break;
      case XK_Return:
        OnSubmit();
        break;
      default:
        break;
    }
  }

  void AList::OnButtonRelease([[maybe_unused]] XEvent *ev) {
    mIsDraggingV = false;
    mIsDraggingH = false;
    mSrollbarTopHL = false;
    mSrollbarBottomHL = false;
    mSrollbarLeftHL = false;
    mSrollbarRightHL = false;
    Draw();
  }

  void AList::ShowVerticalScroll(bool thumb, bool arrows) {
    mShowVThumb = thumb;
    mShowVArrows = arrows;
    Draw();
  }

  void AList::ShowHorizontalScroll(bool thumb, bool arrows) {
    mShowHThumb = thumb;
    mShowHArrows = arrows;
    Draw();
  }

  void AList::OnButtonPress(XEvent *ev) {
    Display *d = AUIPtr()->Disp();
    Window wi = Wnd();
    // Request focus so keyboard events come here
    XSetInputFocus(d, wi, RevertToParent, CurrentTime);
    UINT64 sizeX = SizeX();
    UINT64 sizeY = SizeY();
    int rx, ry, loc_x, loc_y;
    Window root_window, child_window;
    UINT32 mask = 0;
    XQueryPointer(d, wi, &root_window, &child_window, &rx, &ry, &loc_x, &loc_y,
        &mask);
    // Clamp negative values to zero before converting to UINT64
    // This seems to be the XLib glitch, it sends negative coordinate with no reason
    CorrectCoordinateX(loc_x);
    CorrectCoordinateY(loc_y);
    UINT64 lx = SafeUINT64(loc_x);
    UINT64 ly = SafeUINT64(loc_y);
    switch (ev->xbutton.button) {
      case Button4: // Mouse Wheel Up
        ScrollUpPx(SafeINT64(mTextY * 3));
        break;
      case Button5: // Mouse Wheel Down
        ScrollDownPx(SafeINT64(mTextY * 3));
        break;
      case Button1:
        if(mScrollbarsEnabled) {
          // --- Vertical Scrollbar Interactions ---
          if(mShowVThumb || mShowVArrows) {
            // 1. Check for Thumb Dragging (only if thumb is enabled)
            if(mShowVThumb) {
              if(lx >= (sizeX - mArwSz2) && ly >= mArwSz1
                  && ly <= (sizeY - mArwSz1 - mArwSz2)) {
                mIsDraggingV = true;
                return;
              }
            }
            // 2. Check for Vertical Arrow Clicks (only if arrows are enabled)
            if(mShowVArrows) {
              // Top Arrow
              if(lx >= sizeX - mArwSz2 - mArwSz2 / 5 && ly >= mArwSz1 / 5
                  && lx < sizeX - mArwSz2 / 5 && ly <= mArwSz1 - mArwSz1 / 5) {
                mSrollbarTopHL = true;
                ScrollUpPx(SafeINT64(sizeY / 3));
                return;
              }
              // Bottom Arrow (respecting the corner mArwSz2 offset)
              if(lx >= sizeX - mArwSz2 - mArwSz2 / 5
                  && ly >= sizeY - (mArwSz1 + mArwSz2)
                  && lx < sizeX - mArwSz2 / 5 && ly <= sizeY - mArwSz1) {
                mSrollbarBottomHL = true;
                ScrollDownPx(SafeINT64(sizeY / 3));
                return;
              }
            }
          }
          // --- Horizontal Scrollbar Interactions ---
          if(mShowHThumb || mShowHArrows) {
            // 1. Check for Thumb Dragging (only if thumb is enabled)
            if(mShowHThumb) {
              if(ly >= (sizeY - mArwSz2) && lx >= mArwSz1
                  && lx <= (sizeX - mArwSz1 - mArwSz2)) {
                mIsDraggingH = true;
                return;
              }
            }
            // 2. Check for Horizontal Arrow Clicks (only if arrows are enabled)
            if(mShowHArrows) {
              // Left Arrow
              if(lx >= mArwSz1 / 5 && ly <= sizeY - mArwSz2 / 5
                  && lx <= mArwSz1 + mArwSz1 / 5
                  && ly >= sizeY - mArwSz1 - mArwSz1 / 5) {
                mSrollbarLeftHL = true;
                ScrollLeftPx(SafeINT64(sizeX / 3));
                return;
              }
              // Right Arrow (respecting the corner mArwSz2 offset)
              if(lx >= sizeX - (mArwSz1 + mArwSz2)
                  && ly >= sizeY - mArwSz2 - mArwSz2 / 5
                  && lx <= sizeX - mArwSz1 && ly <= sizeY - mArwSz2 / 5) {
                mSrollbarRightHL = true;
                ScrollRightPx(SafeINT64(sizeX / 3));
                return;
              }
            }
          }
        }
        // --- Item Selection Logic ---
        // We only process selection if the click was NOT inside an active scrollbar area
        if(lx > 0 && lx < (sizeX - mArwSz2) && ly > 0
            && ly < (sizeY - mArwSz2)) {
          mCursorIndex = IndexFromY(SafeUINT32(ly));
          if(mCursorIndex > 0 && mCursorIndex <= mData.size()) {
            if(mSingleSelect) {
              std::fill(mTag.begin(), mTag.end(), false);
            }
            mTag[mCursorIndex - 1] = !mTag[mCursorIndex - 1];
            Draw();
          }
        }
        D2("AList::OnButtonPress -> Line selection click caught. Dispatching callback function.");
        AWidget::OnButtonPress(ev);
        break;
      default:
        break;
    }
  }

  void AList::OnMouseMove(XEvent *ev) {
    if(!mIsDraggingV && !mIsDraggingH)
      return;
    UINT64 sizeX = SizeX();
    UINT64 sizeY = SizeY();
    if(mIsDraggingV) {
      double totalH = static_cast<double>(mData.size() * mTextY);
      double visH = static_cast<double>(sizeY);
      double vStart = static_cast<double>(mArwSz1);
      double vEnd = static_cast<double>(sizeY) - static_cast<double>(mArwSz1)
          - static_cast<double>(mArwSz2);
      double troughLen = vEnd - vStart;
      if(troughLen > 10.0) {
        double thumbH = (visH / totalH) * troughLen;
        if(thumbH < 10.0)
          thumbH = 10.0;
        double slideRange = troughLen - thumbH;
        if(slideRange > 0.1) {
          double p = static_cast<double>(ev->xmotion.y) - vStart
              - (thumbH / 2.0);
          double ratio = p / slideRange;
          if(ratio < 0.0)
            ratio = 0.0;
          if(ratio > 1.0)
            ratio = 1.0;
          // Apply ratio to extended height
          double maxVOffset = totalH - visH + static_cast<double>(mArwSz2 * 2);
          mVOffset = SafeINT64(ratio * maxVOffset);
        }
      }
    }
    if(mIsDraggingH) {
      double totalW = static_cast<double>(mMaxWidthPx);
      double visW = static_cast<double>(sizeX);
      double hStart = static_cast<double>(mArwSz1);
      double hEnd = static_cast<double>(sizeX) - static_cast<double>(mArwSz1)
          - static_cast<double>(mArwSz2);
      double hTroughLen = hEnd - hStart;
      if(hTroughLen > 10.0) {
        double thumbW = (visW / totalW) * hTroughLen;
        if(thumbW < 10.0)
          thumbW = 10.0;
        double slideRange = hTroughLen - thumbW;
        if(slideRange > 0.1) {
          double p = static_cast<double>(ev->xmotion.x) - hStart
              - (thumbW / 2.0);
          double ratio = p / slideRange;
          if(ratio < 0.0)
            ratio = 0.0;
          if(ratio > 1.0)
            ratio = 1.0;
          // Apply ratio to extended width
          double maxHOffset = totalW - visW + static_cast<double>(mArwSz2 * 2);
          mHOffset = SafeINT64(ratio * maxHOffset);
        }
      }
    }
    Draw();
  }

  void AList::DrawScrollbars() {
    if(!mScrollbarsEnabled)
      return;
    DrawScrollbarsArrorws();
    Display *d = AUIPtr()->Disp();
    Window wi = Wnd();
    GC gc = GCPtr();
    UINT64 sizeX = SizeX(), sizeY = SizeY();
    XSetForeground(d, gc, AUI_LIST_FG_COLOR);
    XSetLineAttributes(d, gc, SafeUINT32(mArwSz1 / 2), LineSolid, CapRound,
    JoinMiter);
    // --- Vertical Thumb ---
    if(mShowVThumb) {
      double totalH = static_cast<double>(mData.size() * mTextY), visH =
          static_cast<double>(sizeY);
      double vStart = static_cast<double>(mArwSz1), vEnd =
          static_cast<double>(sizeY - mArwSz1 - mArwSz2);
      double troughLen = vEnd - vStart;
      double maxVOffset = totalH - visH + static_cast<double>(mArwSz2 * 2);
      if(maxVOffset > 0 && troughLen > 10.0) {
        double thumbH = (visH / (totalH + static_cast<double>(mArwSz2 * 2)))
            * troughLen;
        if(thumbH < 10.0)
          thumbH = 10.0;
        double slideRange = troughLen - thumbH;
        double ratio = static_cast<double>(mVOffset) / maxVOffset;
        if(ratio > 1.0)
          ratio = 1.0;
        double centerX = static_cast<double>(sizeX)
            - (static_cast<double>(mArwSz2) / 2.0)
            - (static_cast<double>(mArwSz2) / 5.0);
        XDrawLine(d, wi, gc, SafeINT32(centerX),
            SafeINT32(vStart + (ratio * slideRange)), SafeINT32(centerX),
            SafeINT32(vStart + (ratio * slideRange) + thumbH));
      }
    }
    // --- Horizontal Thumb ---
    if(mShowHThumb) {
      double totalW = static_cast<double>(mMaxWidthPx), visW =
          static_cast<double>(sizeX);
      double hStart = static_cast<double>(mArwSz1), hEnd =
          static_cast<double>(sizeX - mArwSz1 - mArwSz2);
      double hTroughLen = hEnd - hStart;
      double maxHOffset = totalW - visW + static_cast<double>(mArwSz2 * 2);
      if(maxHOffset > 0 && hTroughLen > 10.0) {
        double thumbW = (visW / (totalW + static_cast<double>(mArwSz2 * 2)))
            * hTroughLen;
        if(thumbW < 10.0)
          thumbW = 10.0;
        double slideRange = hTroughLen - thumbW;
        double ratio = static_cast<double>(mHOffset) / maxHOffset;
        if(ratio > 1.0)
          ratio = 1.0;
        double centerY = static_cast<double>(sizeY)
            - (static_cast<double>(mArwSz2) / 2.0)
            - (static_cast<double>(mArwSz2) / 5.0);
        XDrawLine(d, wi, gc, SafeINT32(hStart + (ratio * slideRange)),
            SafeINT32(centerY),
            SafeINT32(hStart + (ratio * slideRange) + thumbW),
            SafeINT32(centerY));
      }
    }
  }

  void AList::DrawScrollbarsArrorws() {
    Display *d = AUIPtr()->Disp();
    GC gc = GCPtr();
    Window wi = Wnd();
    UINT64 sizeX = SizeX(), sizeY = SizeY();
    UINT64 mArwSz1P = mArwSz1 / 5, mArwSz2P = mArwSz2 / 5;
    UINT64 mArwSz1H = mArwSz1 / 2, mArwSz2H = mArwSz2 / 2;
    XSetLineAttributes(d, gc, SafeUINT32(mArwSz1 / 4), LineSolid, CapRound,
    JoinMiter);
    if(mShowVArrows) {
      // Top Arrow
      XSetForeground(d, gc, mSrollbarTopHL ? 0xAAAAAA : AUI_LIST_FG_COLOR);
      XDrawLine(d, wi, gc, SafeINT32(sizeX - mArwSz2 - mArwSz2P),
          SafeINT32(mArwSz1H + mArwSz1P),
          SafeINT32(sizeX - mArwSz2H - mArwSz2P), SafeINT32(mArwSz1P));
      XDrawLine(d, wi, gc, SafeINT32(sizeX - mArwSz2H - mArwSz2P),
          SafeINT32(mArwSz1P), SafeINT32(sizeX - mArwSz2P),
          SafeINT32(mArwSz1H + mArwSz1P));
      // Bottom Arrow
      XSetForeground(d, gc, mSrollbarBottomHL ? 0xAAAAAA : AUI_LIST_FG_COLOR);
      XDrawLine(d, wi, gc, SafeINT32(sizeX - mArwSz2 - mArwSz2P),
          SafeINT32(sizeY - mArwSz1 - mArwSz1H - mArwSz1P),
          SafeINT32(sizeX - mArwSz2H - mArwSz2P),
          SafeINT32(sizeY - mArwSz1 - mArwSz1P));
      XDrawLine(d, wi, gc, SafeINT32(sizeX - mArwSz2H - mArwSz2P),
          SafeINT32(sizeY - mArwSz1 - mArwSz1P), SafeINT32(sizeX - mArwSz2P),
          SafeINT32(sizeY - mArwSz1 - mArwSz1H - mArwSz1P));
    }
    if(mShowHArrows) {
      // Right Arrow
      XSetForeground(d, gc, mSrollbarRightHL ? 0xAAAAAA : AUI_LIST_FG_COLOR);
      XDrawLine(d, wi, gc, SafeINT32(sizeX - mArwSz1 - mArwSz1H - mArwSz1P),
          SafeINT32(sizeY - mArwSz2 - mArwSz2P),
          SafeINT32(sizeX - mArwSz1 - mArwSz1P),
          SafeINT32(sizeY - mArwSz2H - mArwSz2P));
      XDrawLine(d, wi, gc, SafeINT32(sizeX - mArwSz1 - mArwSz1P),
          SafeINT32(sizeY - mArwSz2H - mArwSz2P),
          SafeINT32(sizeX - mArwSz1 - mArwSz1H - mArwSz1P),
          SafeINT32(sizeY - mArwSz2P));
      // Left Arrow
      XSetForeground(d, gc, mSrollbarLeftHL ? 0xAAAAAA : AUI_LIST_FG_COLOR);
      XDrawLine(d, wi, gc, SafeINT32(mArwSz1H + mArwSz1P),
          SafeINT32(sizeY - mArwSz2 - mArwSz2P), SafeINT32(mArwSz1P),
          SafeINT32(sizeY - mArwSz2H - mArwSz2P));
      XDrawLine(d, wi, gc, SafeINT32(mArwSz1P),
          SafeINT32(sizeY - mArwSz2H - mArwSz2P),
          SafeINT32(mArwSz1H + mArwSz1P), SafeINT32(sizeY - mArwSz2P));
    }
  }

  void AList::SetCursorPosition(UINT64 line) {
    mCursorIndex = line;
    Draw();
  }

  std::string AList::DataAtCursor() {
    return mData[mCursorIndex];
  }

  std::string AList::DataAt(UINT64 line) {
    return mData[line];
  }

  UINT64 AList::CursorPos() {
    return mCursorIndex;
  }

  UINT64 AList::SelectedIndex() {
    return mCursorIndex;
  }

  void AList::SetSelectedIndex(UINT64 ind) {
    mCursorIndex = ind;
    Draw();
  }

  void AList::Clear() {
    D2("AList::Clear() -> Flushing structural vector tracking allocations");
    // Clear string storage and tagging state layers
    mData.clear();
    mTag.clear();
    // Reset layout dimension tracking variables
    mMaxWidthPx = 0;
    mVOffset = 0;
    mHOffset = 0;
    // Wipe the X11 surface canvas if the window handle is alive and mapped
    Display* d = AUIPtr()->Disp();
    if (d && Wnd()) {
      XClearWindow(d, Wnd());
      XFlush(d);
    }
  }

  AList::~AList() {
    mData.clear();
    mTag.clear();
  }

}

