#include <X11/Xlib.h>

#include "AUILib.h"
#include "AInputBox.h"

namespace aui {

  AInputBox::AInputBox(AWidget *wParent) {
//    AUI *aui = wParent->AUIPtr();
//    Display *d = aui->Disp();
//    INT32 scr = aui->Scr();
//    SetAUIPtr(aui);
//    SetSize(AUI_DEFAULT_INPUT_SZX, AUI_DEFAULT_INPUT_SZY);
//    SetWndParent(wParent);
//    UINT32 szx = SafeUINT32(SizeX());
//    UINT32 szy = SafeUINT32(SizeY());
//    SetType(AUIWidgetType::defaultInputBox);
//    SetTitle(std::string("InputBox"));
//    SetXY(AUI_DEFAULT_INPUT_X, AUI_DEFAULT_INPUT_Y);
//    SetSizeXY(AUI_DEFAULT_INPUT_SZX, AUI_DEFAULT_INPUT_SZY);
//    SetBGColor(AUI_DEFAULT_INPUT_BG);
//    SetHAlign(AUIHAlign::right);
//    D2("Creating inputbox %ux{}", szx, szy)
//    InitWidgetProps(
//        XCreateSimpleWindow(d, wParent->Wnd(), SafeINT32(X()), SafeINT32(Y()),
//            szx, szy, 0, BlackPixel(d, scr), BGColor()));
//    XSelectInput(d, Wnd(),
//        ExposureMask | ButtonReleaseMask | ButtonPressMask | KeyPressMask
//            | FocusChangeMask);
//    D3("inputbox: disp={}, wnd={}, scr={}, auiptr {}", (INT64)d,
//        (INT64)Wnd(), scr, (UINT64)aui)
//    XMapWindow(d, Wnd());
//    aui->AddWidget(this);
//    SetBorderSz(0);
//    SetPressDepth(0);
//    mFilter = mFilterStr;
//    // Start the blink thread
//    INT32 i32szx = SafeINT32(szx);
//    INT32 i32szy = SafeINT32(szy);
//    mBlinkThread = std::thread([this, i32szx, i32szy]() {
//      std::unique_lock<std::mutex> lock(mBlinkMutex);
//      while (!mStopBlink) {
//        if (mBlinkCv.wait_for(lock, std::chrono::milliseconds(500), [this]() { return mStopBlink.load(); })) {
//          break;
//        }
//        mCursorVisible = !mCursorVisible;
//        Display *d1 = AUIPtr()->Disp();
//        XExposeEvent ev;
//        ev.type = Expose;
//        ev.display = d1;
//        ev.window = Wnd();
//        ev.count = 0;
//        ev.x = 0;
//        ev.y = 0;
//        ev.width = i32szx;
//        ev.height = i32szy;
//        XLockDisplay(d1);
//        XSendEvent(d1, Wnd(), False, ExposureMask, (XEvent*) &ev);
//        XFlush(d1);
//        XUnlockDisplay(d1);
//      }
//    });
  }

  AInputBox* AInputBox::AttachTo(AWidget *wParent) {
    return new AInputBox(wParent);
  }

  AInputBox* AInputBox::AttachTo(AWidget *wParent, std::string val) {
    AInputBox *ib = AttachTo(wParent);
    ib->SetText(val);
    return ib;
  }

  void AInputBox::Draw() {
//    AUI *aui = AUIPtr();
//    if (!aui || Wnd() == 0 || GCPtr() == 0) return;
//    if (!BB()) UpdateBuffer();
//    Display *d = aui->Disp();
//    Pixmap bb = BB();
//    GC gc = GCPtr();
//    XFontStruct *f = Font();
//    UINT32 szx = SafeUINT32(SizeX());
//    UINT32 szy = SafeUINT32(SizeY());
//    // 1. Determine visual state
//    //std::string sDraw = showHint ? Title() : Text();
//    bool showHint = Text().empty() && !mIsFocused && mIsEditable;
//    std::string sDraw = Text();
//    if (sDraw.empty() && (!mIsEditable || showHint)) {
//        sDraw = Title();
//    }
//    // Use mDepth for all calculations
//    INT32 cur_depth = mDepth;
//    // 2. Render Background and 3D Frame
//    if (mStyle == AUIWidgetStyle::Simple3D && mRenderPicture != None) {
//        // Fill base frame area
//        XSetForeground(d, gc, BGColor());
//        XFillRectangle(d, bb, gc, 0, 0, szx, szy);
//        // Draw Sunken "Slopes" using mDepth as thickness
//        XRenderColor inner_shadow = {0x0000, 0x0000, 0x0000, 0x7777};
//        XRenderFillRectangle(d, PictOpOver, mRenderPicture, &inner_shadow, 0, 0,
//                             szx, SafeUINT32(cur_depth)); // Top slope
//        XRenderFillRectangle(d, PictOpOver, mRenderPicture, &inner_shadow, 0, 0,
//                             SafeUINT32(cur_depth), szy); // Left slope
//        // Draw light accents on the very outer edges
//        XRenderColor outer_light = {0xFFFF, 0xFFFF, 0xFFFF, 0x5555};
//        XRenderFillRectangle(d, PictOpOver, mRenderPicture, &outer_light,
//                             0, SafeINT32(szy) - 1, szx, 1);
//        XRenderFillRectangle(d, PictOpOver, mRenderPicture, &outer_light,
//                             SafeINT32(szx) - 1, 0, 1, szy);
//        // Render Input Field (White Gradient) shifted by cur_depth
//        XLinearGradient gradient;
//        gradient.p1 = {0, 0};
//        gradient.p2 = {0, XDoubleToFixed(static_cast<double>(szy))};
//        // Adjust field color based on Hover/Focus
//        UINT32 fieldBase = 0xFFFFFF;
//        if (!mIsFocused && mIsHovered) fieldBase = GetBlendedColor(0xFFFFFF, 0, 0.04);
//        RGBAColor cl; cl.value = fieldBase;
//        UINT16 r16 = static_cast<UINT16>((static_cast<UINT32>(cl.rgba.r) << 8) | cl.rgba.r);
//        UINT16 g16 = static_cast<UINT16>((static_cast<UINT32>(cl.rgba.g) << 8) | cl.rgba.g);
//        UINT16 b16 = static_cast<UINT16>((static_cast<UINT32>(cl.rgba.b) << 8) | cl.rgba.b);
//        XRenderColor colors[] = { {r16, g16, b16, 0xFFFF}, {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF} };
//        XFixed stops[] = {XDoubleToFixed(0.0), XDoubleToFixed(1.0)};
//        Picture grad_pix = XRenderCreateLinearGradient(d, &gradient, stops, colors, 2);
//        unsigned int shrink = static_cast<unsigned int>(cur_depth * 2);
//        XRenderComposite(d, PictOpSrc, grad_pix, None, mRenderPicture, 0, 0, 0, 0,
//                         cur_depth, cur_depth,
//                         (szx > shrink ? szx - shrink : 1),
//                         (szy > shrink ? szy - shrink : 1));
//        XRenderFreePicture(d, grad_pix);
//    } else {
//        // Flat style: Background and 1px border
//      XSetForeground(d, gc, mIsHovered ? GetBlendedColor(BGColor(), 255, 0.05) : BGColor());
//      XFillRectangle(d, bb, gc, 0, 0, szx, szy);
//      XSetForeground(d, gc, mIsFocused ? 0x0000FF : BlackPixel(d, aui->Scr()));
//      XDrawRectangle(d, bb, gc, 0, 0, szx - 1, szy - 1);
//    }
//    // 3. Text Positioning (Respecting cur_depth to prevent overlap with slopes)
//    INT32 totalW = XTextWidth(f, sDraw.c_str(), (int)sDraw.size());
//    INT32 drawX = 0, drawY = 0;
//    INT32 horizontalMargin = cur_depth + 5;
//    if (HAlign() == AUIHAlign::left) {
//        drawX = horizontalMargin;
//    } else if (HAlign() == AUIHAlign::center) {
//        drawX = (SafeINT32(szx) - totalW) / 2;
//    } else {
//        drawX = SafeINT32(szx) - totalW - horizontalMargin;
//    }
//    drawY = (SafeINT32(szy) + f->ascent - f->descent) / 2;
//    // Select color: use gray only if explicitly disabled, otherwise ensure solid black for read-only combos
//    XSetForeground(d, gc, (!IsEnabled() && mIsEditable) ? 0x888888 : (showHint ? 0xAAAAAA : BlackPixel(d, aui->Scr())));
//
//    if (!sDraw.empty()) {
//        XDrawString(d, bb, gc, drawX, drawY, sDraw.c_str(), (int)sDraw.size());
//    }
//    // 5. Draw Cursor (Positioned relative to drawX)
//    if (mIsFocused && mCursorVisible && mIsEditable) {
//        INT32 cursorX = drawX + XTextWidth(f, Text().c_str(), (int)mCursorPos);
//        XSetForeground(d, gc, BlackPixel(d, aui->Scr()));
//        if (mOverwriteMode) {
//            INT32 charW = (mCursorPos < Text().size()) ? XTextWidth(f, Text().substr(mCursorPos, 1).c_str(), 1) : 6;
//            if (charW < 4) charW = 6;
//            XFillRectangle(d, bb, gc, cursorX, drawY - f->ascent, static_cast<UINT32>(charW), static_cast<UINT32>(f->ascent + f->descent)); // Draw solid block cursor
//        } else {
//            XDrawLine(d, bb, gc, cursorX, drawY - f->ascent, cursorX, drawY + f->descent); // Draw thin line cursor
//        }
//
//    }
//    XCopyArea(d, bb, Wnd(), gc, 0, 0, szx, szy, 0, 0);
//    XFlush(d);
  }

  void AInputBox::OnKeyPress(XEvent *ev) {
    if(!IsEnabled() || !mIsEditable) return;
    mCursorVisible = true;
    mBlinkCv.notify_one();
    std::string initialText = Text();
    // Revert to the leak-free, zero-allocation native keysym lookup engine
    KeySym keysym = XLookupKeysym(&ev->xkey, 0);
    char asciiBuf = {0};
    int charCount = 0;
    // Intercept Mod2Mask to manually evaluate the active hardware state of NumLock
    bool isNumLockActive = (ev->xkey.state & Mod2Mask) != 0;
    if (isNumLockActive && keysym >= XK_KP_Home && keysym <= XK_KP_9) {
      // Hardware re-mapping logic translates navigation layout states back to characters smoothly
      switch (keysym) {
        case XK_KP_Home: keysym = XK_KP_7; asciiBuf = '7'; charCount = 1; break;
        case XK_KP_Up:   keysym = XK_KP_8; asciiBuf = '8'; charCount = 1; break;
        case XK_KP_Page_Up: keysym = XK_KP_9; asciiBuf = '9'; charCount = 1; break;
        case XK_KP_Left: keysym = XK_KP_4; asciiBuf = '4'; charCount = 1; break;
        case XK_KP_Begin: keysym = XK_KP_5; asciiBuf = '5'; charCount = 1; break;
        case XK_KP_Right: keysym = XK_KP_6; asciiBuf = '6'; charCount = 1; break;
        case XK_KP_End:  keysym = XK_KP_1; asciiBuf = '1'; charCount = 1; break;
        case XK_KP_Down: keysym = XK_KP_2; asciiBuf = '2'; charCount = 1; break;
        case XK_KP_Page_Down: keysym = XK_KP_3; asciiBuf = '3'; charCount = 1; break;
        case XK_KP_Insert: keysym = XK_KP_0; asciiBuf = '0'; charCount = 1; break;
        default: break;
      }
    } else if (keysym >= XK_space && keysym <= XK_asciitilde) {
      asciiBuf = static_cast<char>(keysym);
      charCount = 1;
    }
    std::string k = (keysym != NoSymbol) ? XKeysymToString(keysym) : "";
    std::string cleanKey = k;
    bool isControlKey = false;
    if ((keysym >= XK_Left && keysym <= XK_Down) || (keysym >= XK_KP_Left && keysym <= XK_KP_Down) || keysym == XK_BackSpace || keysym == XK_Return || keysym == XK_KP_Enter || keysym == XK_Delete || keysym == XK_Insert || keysym == XK_KP_Insert || keysym == XK_KP_Delete) {
      isControlKey = true;
      if (keysym == XK_KP_Enter) k = "KP_Enter";
      else if (keysym == XK_KP_Insert) k = "Insert";
      else if (keysym == XK_KP_Delete) k = "Delete";
      else if (keysym == XK_KP_Left) k = "Left";
      else if (keysym == XK_KP_Right) k = "Right";
      else if (keysym == XK_KP_Up) k = "Up";
      else if (keysym == XK_KP_Down) k = "Down";
    }
    if (!isControlKey && charCount > 0) {
      cleanKey = std::string(1, asciiBuf);
      if (std::regex_match(cleanKey.c_str(), mFilter)) {
        if (mOverwriteMode && mCursorPos < Text().size()) {
          Text().replace(mCursorPos++, 1, cleanKey);
        } else if (Text().size() < mMaxLength) {
          Text().insert(mCursorPos++, cleanKey);
        } else {
          D1("Key character input blocked: String maximum layout limit of {} bytes reached", mMaxLength);
          return;
        }
        D2("kb character entry input '{}', data '{}'", cleanKey.c_str(), Text().c_str());
      } else {
        D1("Key pressed: '{}' and is filtered", cleanKey.c_str());
      }
    } else if (isControlKey) {
      switch (string_to_case.count(k.c_str()) ? string_to_case.at(k.c_str()) : 0) {
        case 0:
          D1("Key pressed: '{}' and is filtered or unmapped", k.c_str());
          break;
        case 1:
          OnBackSpace();
          break;
        case 2:
          D1("space pressed")
          break;
        case 3:
          D1("enter pressed")
          OnSubmit();
          break;
        case 4:
          D1("NP enter pressed")
          OnSubmit();
          break;
        case 5:
          if(mCursorPos > 0) mCursorPos--;
          break;
        case 6:
          if(mCursorPos < Text().size()) mCursorPos++;
          break;
        case 7:
          if(mCursorPos < Text().size()) Text().erase(mCursorPos, 1);
          break;
        case 8:
          mOverwriteMode = !mOverwriteMode;
          D1("Insert key pressed, Overwrite mode: {}", mOverwriteMode);
          break;
        default:
          E("strange value2({})", string_to_case.at(k.c_str()))
          break;
      }
    }
    if(initialText != Text() && OnValueChanged != nullptr) {
      OnValueChanged(this, mUserDataValueChanged);
    }
    Draw();
  }

//  void aui::AInputBox::OnKeyPress(XEvent *ev) {
//    if(!IsEnabled()) return;
//    mCursorVisible = true;
//    std::string initialText = Text();
//    char asciiBuf[8] = {0};
//    KeySym keysym = NoSymbol;
//    XComposeStatus compose;
//    int charCount = XLookupString(&ev->xkey, asciiBuf, sizeof(asciiBuf), &keysym, &compose);
//    std::string k = (keysym != NoSymbol) ? XKeysymToString(keysym) : "";
//    std::string cleanKey = k;
//    bool isControlKey = false;
//    if ((keysym >= XK_Left && keysym <= XK_Down) || (keysym >= XK_KP_Left && keysym <= XK_KP_Down) || keysym == XK_BackSpace || keysym == XK_Return || keysym == XK_KP_Enter || keysym == XK_Delete || keysym == XK_Insert || keysym == XK_KP_Insert || keysym == XK_KP_Delete) {
//      isControlKey = true;
//      if (keysym == XK_KP_Enter) k = "KP_Enter";
//      else if (keysym == XK_KP_Insert) k = "Insert";
//      else if (keysym == XK_KP_Delete) k = "Delete";
//      else if (keysym == XK_KP_Left) k = "Left";
//      else if (keysym == XK_KP_Right) k = "Right";
//      else if (keysym == XK_KP_Up) k = "Up";
//      else if (keysym == XK_KP_Down) k = "Down";
//    }  if (!isControlKey && charCount > 0) {
//      // Direct string assignment from a single character byte bypasses dynamic heap allocation pipelines completely
//      cleanKey = std::string(1, asciiBuf[0]);
//      if (std::regex_match(cleanKey.c_str(), mFilter)) {
//        if (mOverwriteMode && mCursorPos < Text().size()) {
//          Text().replace(mCursorPos++, 1, cleanKey);
//        } else {
//          Text().insert(mCursorPos++, cleanKey);
//        }
//        D2("kb character entry input '{}', data '{}'", cleanKey.c_str(), Text().c_str());
//      } else {
//        D1("Key pressed: '{}' and is filtered", cleanKey.c_str());
//      }
//    }
//    else if (isControlKey) {
//      switch (string_to_case.count(k.c_str()) ? string_to_case.at(k.c_str()) : 0) {
//        case 0:
//          D1("Key pressed: '{}' and is filtered or unmapped", k.c_str());
//          break;
//        case 1:
//          OnBackSpace();
//          break;
//        case 2:
//          D1("space pressed")
//          break;
//        case 3:
//          D1("enter pressed")
//          OnSubmit();
//          break;
//        case 4:
//          D1("NP enter pressed")
//          OnSubmit();
//          break;
//        case 5:
//          if(mCursorPos > 0) mCursorPos--;
//          break;
//        case 6:
//          if(mCursorPos < Text().size()) mCursorPos++;
//          break;
//        case 7:
//          if(mCursorPos < Text().size()) Text().erase(mCursorPos, 1);
//          break;
//        case 8:
//          mOverwriteMode = !mOverwriteMode;
//          D1("Insert key pressed, Overwrite mode: {}", mOverwriteMode);
//          break;
//        default:
//          E("strange value2({})", string_to_case.at(k.c_str()))
//          break;
//      }
//    }
//    if(initialText != Text() && OnValueChanged != nullptr) {
//      OnValueChanged(this, mUserDataValueChanged);
//    }
//    Draw();
//  }



  void AInputBox::OnButtonPress(XEvent *ev) {
    if(!IsEnabled()) return;
    D3("Requesting focus for window {}", (UINT64)Wnd())
    XSetInputFocus(AUIPtr()->Disp(), Wnd(), RevertToParent, CurrentTime);
    // 1. Re-calculate the exact text drawing start position (drawX) identical to the Draw() method math
    XFontStruct *f = Font();
    UINT32 szx = SafeUINT32(SizeX());
    bool showHint = Text().empty() && !mIsFocused && mIsEditable;
    std::string sDraw = showHint ? Title() : Text();
    if (sDraw.empty() && (!mIsEditable || showHint)) sDraw = Title();
    INT32 totalW = XTextWidth(f, sDraw.c_str(), (int)sDraw.size());
    INT32 drawX = 0;
    INT32 horizontalMargin = mDepth + 5;
    if (HAlign() == AUIHAlign::left) drawX = horizontalMargin;
    else if (HAlign() == AUIHAlign::center) drawX = (SafeINT32(szx) - totalW) / 2;
    else drawX = SafeINT32(szx) - totalW - horizontalMargin;
    // 2. Linear hit-test scanning to find the closest character slice boundaries relative to the mouse X coordinate
    INT32 clickX = ev->xbutton.x;
    size_t calculatedPos = 0;
    INT32 minDelta = 999999;
    if (!showHint && !Text().empty()) {
      for (size_t i = 0; i <= Text().size(); ++i) {
        INT32 widthToCursor = XTextWidth(f, Text().c_str(), (int)i);
        INT32 currentCursorX = drawX + widthToCursor;
        INT32 delta = std::abs(clickX - currentCursorX);
        if (delta < minDelta) {
          minDelta = delta;
          calculatedPos = i;
        }
      }
      mCursorPos = calculatedPos; // Lock the active cursor tracking index onto the hit target symbol
    }
    mCursorVisible = true; // Instantly force the cursor to be visible and draw it immediately
    Draw();
  }

  void AInputBox::OnButtonRelease([[maybe_unused]]XEvent *ev) {
    D3();
  }

  void AInputBox::SetInputFilter(std::string f) {
    mFilterStr = f;
    mFilter = mFilterStr;
  }

  void AInputBox::OnBackSpace() {
    D2("backspace")
    if(Text().size() > 0 && mCursorPos > 0) {
      Text().erase(mCursorPos - 1, 1);
      mCursorPos--;
      D2("deleting character, new pos {}", mCursorPos)
    } else {
      D2("data empty or cursor at start")
    }
  }

  void AComboBox::SetSelectedIndex(INT64 index) {
     if (index >= 0 && index < static_cast<INT64>(mItems.size())) {
       mSelectedIndex = index;
       if (mInputBox) {
         mInputBox->SetText(mItems[static_cast<size_t>(index)]);
       }
       if (mOnSelectionChangedCB) {
         mOnSelectionChangedCB(mSelectedIndex, mItems[static_cast<size_t>(index)]);
       }
       // Only invoke Draw if the underlying window handles are fully mapped and registered
       if (Wnd() != 0 && BB() != None) {
         Draw();
       }
     }
   }

  void AInputBox::OnFocusIn(UNUSED XEvent *ev) {
    D3()
    mIsFocused = true;
    mCursorVisible = true;
    Draw();
  }

  void AInputBox::OnFocusOut(UNUSED XEvent *ev) {
    D3()
    mIsFocused = false;
    mCursorVisible = false;
    Draw();
  }

  void AInputBox::SetOnValueChangedCB(std::function<void(AWidget *w, void *arbdata)> func, void *data) {
    mUserDataValueChanged = data;
    OnValueChanged = func;
    D4()
  }

  void AInputBox::Enable() {
    AWidget::Enable();
  }

  void AInputBox::Disable() {
    mCursorVisible = false;
    aui::AWidget::Disable();
  }

  AInputBox::~AInputBox() {
    {
      std::lock_guard<std::mutex> lock(mBlinkMutex);
      mStopBlink = true;
    }
    mBlinkCv.notify_one();
    if (mBlinkThread.joinable()) {
      mBlinkThread.join();
    }
  }
}

