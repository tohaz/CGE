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
      Display* dpy = AUIPtr()->Disp();
      Window win = Wnd();
      GC gc = GCPtr();

      int b = AUI_DEFAULT_INPUT_BORDERW;
      int w = SafeINT32(SizeX());
      int h = SafeINT32(SizeY());
      int offset = b;

      // 1. ОТРИСОВКА РАМКИ
      if (Style() == AUIWidgetStyle::Simple3D) {
          // Объявляем массивы точек для каждой стороны (трапеции)
          XPoint topP[4]    = {{0, 0}, {SafeINT16(w), 0}, {SafeINT16(w - b), SafeINT16(b)}, {SafeINT16(b), SafeINT16(b)}};
          XPoint leftP[4]   = {{0, 0}, {0, SafeINT16(h)}, {SafeINT16(b), SafeINT16(h - b)}, {SafeINT16(b), SafeINT16(b)}};
          XPoint bottomP[4] = {{0, SafeINT16(h)}, {SafeINT16(w), SafeINT16(h)}, {SafeINT16(w - b), SafeINT16(h - b)}, {SafeINT16(b), SafeINT16(h - b)}};
          XPoint rightP[4]  = {{SafeINT16(w), 0}, {SafeINT16(w), SafeINT16(h)}, {SafeINT16(w - b), SafeINT16(h - b)}, {SafeINT16(w - b), SafeINT16(b)}};

          XSetForeground(dpy, gc, 0xFFFFFF); // Свет сверху и слева
          XFillPolygon(dpy, win, gc, topP, 4, Convex, CoordModeOrigin);
          XFillPolygon(dpy, win, gc, leftP, 4, Convex, CoordModeOrigin);

          XSetForeground(dpy, gc, 0x808080); // Тень снизу и справа
          XFillPolygon(dpy, win, gc, bottomP, 4, Convex, CoordModeOrigin);
          XFillPolygon(dpy, win, gc, rightP, 4, Convex, CoordModeOrigin);

          XSetForeground(dpy, gc, 0x000000); // Внутренняя черная рамка
          XDrawRectangle(dpy, win, gc, b, b, SafeUINT32(w - 2*b - 1), SafeUINT32(h - 2*b - 1));

          offset = b + 1;
      } else {
          XSetForeground(dpy, gc, 0x000000);
          XFillRectangle(dpy, win, gc, 0, 0, SafeUINT32(w), SafeUINT32(h));
      }

      // 2. ЗАЛИВКА ФОНА (фон рисуется ПЕРЕД текстом)
      XSetForeground(dpy, gc, BGColor());
      XFillRectangle(dpy, win, gc, offset, offset, SafeUINT32(w - 2 * offset), SafeUINT32(h - 2 * offset));

      // 3. ОТРИСОВКА ТЕКСТА
      if (Font()) {
          XSetFont(dpy, gc, Font()->fid);
          XSetForeground(dpy, gc, AUI_DEFAULT_INPUT_FG);

          int textLen = SafeINT32(Text().length());
          int textW = XTextWidth(Font(), Text().c_str(), textLen);
          int textH = Font()->ascent + Font()->descent;

          // Горизонтальное выравнивание
          int textX = offset + mInnerInset;
          if (HAlign() == AUIHAlign::center) textX = (w - textW) / 2;
          else if (HAlign() == AUIHAlign::right) textX = w - textW - offset - mInnerInset;

          // Вертикальное центрирование
          int textY = (h - textH) / 2 + Font()->ascent;

          XDrawString(dpy, win, gc, textX, textY, Text().c_str(), textLen);

          // 4. КУРСОР
          if (mIsFocused && mCursorVisible) {
              int cursorX = textX + XTextWidth(Font(), Text().c_str(), SafeINT32(mCursorPos));
              XSetForeground(dpy, gc, 0x000000);
              XFillRectangle(dpy, win, gc,
                             cursorX,
                             textY - Font()->ascent,
                             SafeUINT32(mCursorW),
                             SafeUINT32(textH));
          }
      }
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
      D1("data empty or cursor at start")
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
    D1()
  }

  AInputBox::~AInputBox() {
    OnValueChanged = nullptr;
    mStopBlink = true;
    if(mBlinkThread.joinable()) mBlinkThread.join();
    D3("v")
  }


}

