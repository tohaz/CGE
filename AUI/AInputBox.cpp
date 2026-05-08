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
            szx, szy, 1, BlackPixel(d, scr), BGColor()));
    XSelectInput(d, Wnd(),
        ExposureMask | ButtonReleaseMask | ButtonPressMask | KeyPressMask
            | FocusChangeMask);
    D3("inputbox: disp=%lu, wnd=%lu, scr=%d, auiptr %lu", (INT64)d,
        (INT64)Wnd(), scr, (UINT64)aui)
    XMapWindow(d, Wnd());
    aui->AddWidget(this);
    SetBorderSz(AUI_DEFAULT_INPUT_BORDERW);
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
    Display *d = AUIPtr()->Disp();
    Window w = Wnd();
    GC gc = GCPtr();
    XFontStruct *f = Font();
    XClearWindow(d, w);
    bool showHint = Text().empty() && !mIsFocused;
    std::string stringToDraw = showHint ? Title() : Text();
    if(showHint) {
      XSetForeground(d, gc, 0x000000);
    } else {
      XSetForeground(d, gc, BlackPixel(d, AUIPtr()->Scr()));
    }
    INT32 totalW = XTextWidth(f, stringToDraw.c_str(),
        (int) stringToDraw.size());
    INT32 textBeforeCursorW = XTextWidth(f, Text().c_str(), (int) mCursorPos);
    INT32 fontHeight = f->ascent + f->descent;
    INT32 drawX = 0;
    INT32 drawY = 0;
    switch (HAlign()) {
      case AUIHAlign::left:
        drawX = 5;
        break;
      case AUIHAlign::center:
        drawX = (SafeINT32(SizeX()) - totalW) / 2;
        break;
      case AUIHAlign::right:
        drawX = SafeINT32(SizeX()) - totalW - 5;
        break;
      default:
        break;
    }
    switch (VAlign()) {
      case AUIVAlign::top:
        drawY = f->ascent + 5;
        break;
      case AUIVAlign::center:
        drawY = (SafeINT32(SizeY()) + f->ascent - f->descent) / 2;
        break;
      case AUIVAlign::bottom:
        drawY = SafeINT32(SizeY()) - f->descent - 5;
        break;
      default:
        break;
    }
    XDrawString(d, w, gc, drawX, drawY, stringToDraw.c_str(),
        (int) stringToDraw.size());
    if(mIsFocused && mCursorVisible) {
      int cursorX = drawX + textBeforeCursorW;
      int cursorYTop = drawY - f->ascent;
      XDrawLine(d, w, gc, cursorX, cursorYTop, cursorX,
          cursorYTop + fontHeight);
    }
    XSetForeground(d, gc, BlackPixel(d, AUIPtr()->Scr()));
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
    if(initialText != Text()) {
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

  AInputBox::~AInputBox() {
    D3()
    mStopBlink = true;
    if(mBlinkThread.joinable())
      mBlinkThread.join();
  }

  void AInputBox::SetOnValueChangedCB(std::function<void(AWidget *w, void *arbdata)> func, void *data) {
    mUserDataValueChanged = data;
    OnValueChanged = func;
    D1()
  }

}

