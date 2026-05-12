#include <X11/Xlib.h>

#include "AUILib.h"
#include "AInputBox.h"

namespace aui {

  AInputBox::AInputBox(AWidget *wParent) {
    AUI *aui = wParent->AUIPtr();
    Display *d = aui->Disp();
    INT32 scr = aui->Scr();
    SetAUIPtr(aui);
    SetSize(AUI_DEFAULT_INPUT_SZX, AUI_DEFAULT_INPUT_SZY);
    SetWndParent(wParent);
    UINT32 szx = SafeUINT32(SizeX());
    UINT32 szy = SafeUINT32(SizeY());
    SetType(AUIWidgetType::defaultInputBox);
    SetTitle(std::string("InputBox"));
    SetXY(AUI_DEFAULT_INPUT_X, AUI_DEFAULT_INPUT_Y);
    SetSizeXY(AUI_DEFAULT_INPUT_SZX, AUI_DEFAULT_INPUT_SZY);
    SetBGColor(AUI_DEFAULT_INPUT_BG);
    SetHAlign(AUIHAlign::right);
    D2("Creating inputbox %ux{}", szx, szy)
    InitWidgetProps(
        XCreateSimpleWindow(d, wParent->Wnd(), SafeINT32(X()), SafeINT32(Y()),
            szx, szy, 0, BlackPixel(d, scr), BGColor()));
    XSelectInput(d, Wnd(),
        ExposureMask | ButtonReleaseMask | ButtonPressMask | KeyPressMask
            | FocusChangeMask);
    D3("inputbox: disp={}, wnd={}, scr={}, auiptr {}", (INT64)d,
        (INT64)Wnd(), scr, (UINT64)aui)
    XMapWindow(d, Wnd());
    aui->AddWidget(this);
    SetBorderSz(0);
    SetPressDepth(0);
    mFilter = mFilterStr;
    // Start the blink thread
    INT32 i32szx = SafeINT32(szx);
    INT32 i32szy = SafeINT32(szy);
    mBlinkThread = std::thread([this, i32szx, i32szy]() {
      while (!mStopBlink) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        mCursorVisible = !mCursorVisible;
        Display *d1 = AUIPtr()->Disp();
        XExposeEvent ev;
        ev.type = Expose;
        ev.display = d1;
        ev.window = Wnd();
        ev.count = 0;
        // These fields are technically required for an Expose event
        ev.x = 0;
        ev.y = 0;
        ev.width = i32szx;
        ev.height = i32szy;
        XLockDisplay(d1);
        XSendEvent(d1, Wnd(), False, ExposureMask, (XEvent*) &ev);
        XFlush(d1);
        XUnlockDisplay(d1);
      }
    });
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
      AUI *aui = AUIPtr();
      if (!aui || Wnd() == 0 || GCPtr() == 0) return;
      if (!BB()) UpdateBuffer();
      Display *d = aui->Disp();
      Pixmap bb = BB();
      GC gc = GCPtr();
      XFontStruct *f = Font();
      UINT32 szx = SafeUINT32(SizeX());
      UINT32 szy = SafeUINT32(SizeY());
      // 1. Determine visual state
      bool showHint = Text().empty() && !mIsFocused;
      std::string sDraw = showHint ? Title() : Text();
      // Use mDepth for all calculations
      INT32 cur_depth = mDepth;
      // 2. Render Background and 3D Frame
      if (mStyle == AUIWidgetStyle::Simple3D && mRenderPicture != None) {
          // Fill base frame area
          XSetForeground(d, gc, BGColor());
          XFillRectangle(d, bb, gc, 0, 0, szx, szy);
          // Draw Sunken "Slopes" using mDepth as thickness
          XRenderColor inner_shadow = {0x0000, 0x0000, 0x0000, 0x7777};
          XRenderFillRectangle(d, PictOpOver, mRenderPicture, &inner_shadow, 0, 0,
                               szx, SafeUINT32(cur_depth)); // Top slope
          XRenderFillRectangle(d, PictOpOver, mRenderPicture, &inner_shadow, 0, 0,
                               SafeUINT32(cur_depth), szy); // Left slope
          // Draw light accents on the very outer edges
          XRenderColor outer_light = {0xFFFF, 0xFFFF, 0xFFFF, 0x5555};
          XRenderFillRectangle(d, PictOpOver, mRenderPicture, &outer_light,
                               0, SafeINT32(szy) - 1, szx, 1);
          XRenderFillRectangle(d, PictOpOver, mRenderPicture, &outer_light,
                               SafeINT32(szx) - 1, 0, 1, szy);
          // Render Input Field (White Gradient) shifted by cur_depth
          XLinearGradient gradient;
          gradient.p1 = {0, 0};
          gradient.p2 = {0, XDoubleToFixed(static_cast<double>(szy))};
          // Adjust field color based on Hover/Focus
          UINT32 fieldBase = 0xFFFFFF;
          if (!mIsFocused && mIsHovered) fieldBase = GetBlendedColor(0xFFFFFF, 0, 0.04);
          RGBAColor cl; cl.value = fieldBase;
          UINT16 r16 = static_cast<UINT16>((static_cast<UINT32>(cl.rgba.r) << 8) | cl.rgba.r);
          UINT16 g16 = static_cast<UINT16>((static_cast<UINT32>(cl.rgba.g) << 8) | cl.rgba.g);
          UINT16 b16 = static_cast<UINT16>((static_cast<UINT32>(cl.rgba.b) << 8) | cl.rgba.b);
          XRenderColor colors[] = { {r16, g16, b16, 0xFFFF}, {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF} };
          XFixed stops[] = {XDoubleToFixed(0.0), XDoubleToFixed(1.0)};
          Picture grad_pix = XRenderCreateLinearGradient(d, &gradient, stops, colors, 2);

          unsigned int shrink = static_cast<unsigned int>(cur_depth * 2);
          XRenderComposite(d, PictOpSrc, grad_pix, None, mRenderPicture, 0, 0, 0, 0,
                           cur_depth, cur_depth,
                           (szx > shrink ? szx - shrink : 1),
                           (szy > shrink ? szy - shrink : 1));
          XRenderFreePicture(d, grad_pix);
      } else {
          // Flat style: Background and 1px border
          XSetForeground(d, gc, mIsHovered ? GetBlendedColor(BGColor(), 255, 0.05) : BGColor());
          XFillRectangle(d, bb, gc, 0, 0, szx, szy);
          XSetForeground(d, gc, mIsFocused ? 0x0000FF : BlackPixel(d, aui->Scr()));
          XDrawRectangle(d, bb, gc, 0, 0, szx - 1, szy - 1);
      }
      // 3. Text Positioning (Respecting cur_depth to prevent overlap with slopes)
      INT32 totalW = XTextWidth(f, sDraw.c_str(), (int)sDraw.size());
      INT32 drawX = 0, drawY = 0;
      INT32 horizontalMargin = cur_depth + 5;
      if (HAlign() == AUIHAlign::left) {
          drawX = horizontalMargin;
      } else if (HAlign() == AUIHAlign::center) {
          drawX = (SafeINT32(szx) - totalW) / 2;
      } else {
          drawX = SafeINT32(szx) - totalW - horizontalMargin;
      }
      drawY = (SafeINT32(szy) + f->ascent - f->descent) / 2;
      // 4. Draw Text
      XSetForeground(d, gc, showHint ? 0xAAAAAA : BlackPixel(d, aui->Scr()));
      if (!sDraw.empty()) {
          XDrawString(d, bb, gc, drawX, drawY, sDraw.c_str(), (int)sDraw.size());
      }
      // 5. Draw Cursor (Positioned relative to drawX)
      if (mIsFocused && mCursorVisible) {
          int cursorX = drawX + XTextWidth(f, Text().c_str(), (int)mCursorPos);
          XSetForeground(d, gc, BlackPixel(d, aui->Scr()));
          XDrawLine(d, bb, gc, cursorX, drawY - f->ascent, cursorX, drawY + f->descent);
      }
      XCopyArea(d, bb, Wnd(), gc, 0, 0, szx, szy, 0, 0);
      XFlush(d);
  }

  void AInputBox::OnKeyPress(XEvent *ev) {
    mCursorVisible = true;
    std::string initialText = Text();
    KeySym keysym = XLookupKeysym(&ev->xkey, 0);
    std::string k = XKeysymToString(keysym);
    if(std::regex_match(k.c_str(), mFilter)) {
      Text().insert(mCursorPos++, k);
      D2("kb input '{}', data '{}'", k.c_str(), Text().c_str());
    } else {
      switch (string_to_case.count(k.c_str()) ? string_to_case.at(k.c_str()) : 0) {
        case 0:
          D1("Key pressed: '{}' and is filtered", k.c_str());
          ;
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
          if(mCursorPos > 0)
            mCursorPos--;
          break;
        case 6:
          if(mCursorPos < Text().size())
            mCursorPos++;
          break;
        case 7:
          if(mCursorPos < Text().size())
            Text().erase(mCursorPos, 1);
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

  void AInputBox::OnButtonPress([[maybe_unused]]XEvent *ev) {
    D3("Requesting focus for window {}", (UINT64)Wnd())
    XSetInputFocus(AUIPtr()->Disp(), Wnd(), RevertToParent, CurrentTime);
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

  void AInputBox::SetText(std::string val) {
    AWidget::SetText(val);
    mCursorPos = Text().size();
    Draw();
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

  AInputBox::~AInputBox() {
    OnValueChanged = nullptr;
    mStopBlink = true;
    if(mBlinkThread.joinable())
      mBlinkThread.join();
    D3("v")
  }

}

