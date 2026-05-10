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
    D2("Creating inputbox %ux%u", szx, szy)
    InitWidgetProps(
        XCreateSimpleWindow(d, wParent->Wnd(), SafeINT32(X()), SafeINT32(Y()),
            szx, szy, 0, BlackPixel(d, scr), BGColor()));
    XSelectInput(d, Wnd(),
        ExposureMask | ButtonReleaseMask | ButtonPressMask | KeyPressMask
            | FocusChangeMask);
    D3("inputbox: disp=%lu, wnd=%lu, scr=%d, auiptr %lu", (INT64)d,
        (INT64)Wnd(), scr, (UINT64)aui)
    XMapWindow(d, Wnd());
    aui->AddWidget(this);
    SetBorderSz(0);
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
      // Ensure backbuffer and XRender picture are initialized
      if (!BB()) UpdateBuffer();
      Display *d = aui->Disp();
      Pixmap bb = BB();
      GC gc = GCPtr();
      XFontStruct *f = Font();
      UINT32 szx = SafeUINT32(SizeX());
      UINT32 szy = SafeUINT32(SizeY());
      // 1. Determine whether to draw the actual Text or the Hint (Title)
      bool showHint = Text().empty() && !mIsFocused;
      std::string sDraw = showHint ? Title() : Text();
      // 2. Render background based on Style
      if (mStyle == AUIWidgetStyle::Simple3D && mRenderPicture != None) {
          // --- STYLE: Simple3D (Sunken Effect) ---
          // Fill the outer area with widget background color
          XSetForeground(d, gc, BGColor());
          XFillRectangle(d, bb, gc, 0, 0, szx, szy);
          // Draw inner shadow to create the "hole" effect
          XRenderColor hole_shadow = {0x1111, 0x1111, 0x1111, 0xAAAA};
          XRenderFillRectangle(d, PictOpOver, mRenderPicture, &hole_shadow, 0, 0, szx, szy);
          // Draw the white input field with a subtle vertical gradient
          XLinearGradient gradient;
          gradient.p1 = {0, 0};
          gradient.p2 = {0, XDoubleToFixed(static_cast<double>(szy))};
          XRenderColor colors[] = {
              {0xEEEE, 0xEEEE, 0xEEEE, 0xFFFF}, // Slightly grey top
              {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF}  // Pure white bottom
          };
          XFixed stops[] = {XDoubleToFixed(0.0), XDoubleToFixed(1.0)};
          Picture grad_pix = XRenderCreateLinearGradient(d, &gradient, stops, colors, 2);
          int inset = mDepth;
          unsigned int shrink = static_cast<unsigned int>(inset * 2);
          // Composite the gradient field into the "hole"
          XRenderComposite(d, PictOpSrc, grad_pix, None, mRenderPicture, 0, 0, 0, 0,
                           inset, inset,
                           (szx > shrink ? szx - shrink : 1),
                           (szy > shrink ? szy - shrink : 1));
          XRenderFreePicture(d, grad_pix);
      } else {
          // --- STYLE: Flat ---
          XSetForeground(d, gc, BGColor());
          XFillRectangle(d, bb, gc, 0, 0, szx, szy);

          // Draw standard flat black border
          XSetForeground(d, gc, BlackPixel(d, aui->Scr()));
          XDrawRectangle(d, bb, gc, 0, 0, szx - 1, szy - 1);
      }
      // 3. Set text color (Grey for Hint, Black for Input)
      if (showHint) {
          XSetForeground(d, gc, 0xAAAAAA);
      } else {
          XSetForeground(d, gc, BlackPixel(d, aui->Scr()));
      }
      // 4. Calculate text positioning
      INT32 totalW = XTextWidth(f, sDraw.c_str(), (int)sDraw.size());
      int cur_inset = (mStyle == AUIWidgetStyle::Simple3D) ? mDepth : 0;

      INT32 drawX = 0, drawY = 0;
      // Horizontal alignment logic
      if (HAlign() == AUIHAlign::left) {
          drawX = cur_inset + 5;
      } else if (HAlign() == AUIHAlign::center) {
          drawX = (SafeINT32(szx) - totalW) / 2;
      } else {
          drawX = SafeINT32(szx) - totalW - cur_inset - 5;
      }
      // Vertical alignment logic
      if (VAlign() == AUIVAlign::top) {
          drawY = f->ascent + cur_inset + 2;
      } else if (VAlign() == AUIVAlign::center) {
          drawY = (SafeINT32(szy) + f->ascent - f->descent) / 2;
      } else {
          drawY = SafeINT32(szy) - f->descent - cur_inset - 2;
      }
      // 5. Draw the actual string (Hint or Text)
      XSetForeground(d, gc, 0x8800);
      if (!sDraw.empty()) {
          XDrawString(d, bb, gc, drawX, drawY, sDraw.c_str(), (int)sDraw.size());
      }
      // 6. Draw cursor if focused and visible (blink logic)
      if (mIsFocused && mCursorVisible) {
          int cursorX = drawX + XTextWidth(f, Text().c_str(), (int)mCursorPos);
          XSetForeground(d, gc, BlackPixel(d, aui->Scr()));
          XDrawLine(d, bb, gc, cursorX, drawY - f->ascent, cursorX, drawY + f->descent);
      }
      // 7. Transfer final image from Pixmap to Window
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
      D2("kb input '%s', data '%s'", k.c_str(), Text().c_str())
    } else {
      switch (string_to_case.count(k.c_str()) ? string_to_case.at(k.c_str()) : 0) {
        case 0:
          D1("Key pressed: '%s' and is filtered", k.c_str())
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
          E("strange value2(%lu)", string_to_case.at(k.c_str()))
          break;
      }
    }
    if(initialText != Text() && OnValueChanged != nullptr) {
      OnValueChanged(this, mUserDataValueChanged);
    }
    Draw();
  }

  void AInputBox::OnButtonPress([[maybe_unused]]XEvent *ev) {
    D3("Requesting focus for window %lu", (UINT64)Wnd())
    XSetInputFocus(AUIPtr()->Disp(), Wnd(), RevertToParent, CurrentTime);
  }

  void AInputBox::OnButtonRelease([[maybe_unused]]XEvent *ev) {
    D3()
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
      D2("deleting character, new pos %lu", mCursorPos)
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

  void AInputBox::SetOnValueChangedCB(
      std::function<void(AWidget *w, void *arbdata)> func, void *data) {
    mUserDataValueChanged = data;
    OnValueChanged = func;
    D1()
  }

  AInputBox::~AInputBox() {
    OnValueChanged = nullptr;
    mStopBlink = true;
    if(mBlinkThread.joinable())
      mBlinkThread.join();
    D3("v")
  }

}

