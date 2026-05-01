#include <X11/Xlib.h>

#include "AUILib.h"
#include "AInputBox.h"

namespace aui {

  AInputBox::AInputBox(AWidget *wParent) {
      AUI *aui = wParent->AUIPtr();
      Display* d = aui->Disp();
      UINT32 scr = aui->Scr();
      SetAUIPtr(aui);
      SetWndParent(wParent);
      SetType(AUIWidgetType::defaultInputBox);
      SetTitle(std::string("InputBox"));
      SetXY(AUI_DEFAULT_INPUT_X, AUI_DEFAULT_INPUT_Y);
      SetSizeXY(AUI_DEFAULT_INPUT_SZX, AUI_DEFAULT_INPUT_SZY);
      SetBGColor(AUI_DEFAULT_INPUT_BG);
      SetHAlign(AUIHAlign::right);
      InitWidgetProps(XCreateSimpleWindow(d, wParent->Wnd(), X(), Y(), SizeX(), SizeY(), 1,
          BlackPixel(d, scr), BGColor()));
      XSelectInput(d, Wnd(), ExposureMask | ButtonReleaseMask | KeyPressMask);
      D3("inputbox: disp=%lu, wnd=%lu, scr=%d, auiptr %lu", (INT64)d, (INT64)Wnd(), scr, (UINT64)aui)
      XMapWindow(d, Wnd());
      aui->AddWidget(this);
      SetBorderSz(AUI_DEFAULT_INPUT_BORDERW);
      mFilter = mFilterStr;

      // Start the blink thread
      mBlinkThread = std::thread([this]() {
        while (!mStopBlink) {
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
          mCursorVisible = !mCursorVisible;
          Display* d = AUIPtr()->Disp();
          // Zero-initialize the entire structure to satisfy Valgrind/X11
          XExposeEvent ev = {0};
          ev.type = Expose;
          ev.display = d;
          ev.window = Wnd();
          ev.count = 0;
          // These fields are technically required for an Expose event
          ev.x = 0;
          ev.y = 0;
          ev.width = SizeX();
          ev.height = SizeY();
          XLockDisplay(d);
          XSendEvent(d, Wnd(), False, ExposureMask, (XEvent*)&ev);
          XFlush(d);
          XUnlockDisplay(d);
        }
      });
    }

  AInputBox* AInputBox::AttachTo(AWidget* wParent) {
    return new AInputBox(wParent);
  }

  AInputBox* AInputBox::AttachTo(AWidget* wParent, const char* val) {
    AInputBox* ib = AttachTo(wParent);
    ib->SetText((char *)val);
    return ib;
  }

  void AInputBox::Draw() {
    Display *d = AUIPtr()->Disp();
        Window w = Wnd();
        GC gc = GCPtr();
        XFontStruct *f = Font();
        XClearWindow(d, w);
        int totalW = XTextWidth(f, Text().c_str(), (int) Text().size());
        int textBeforeCursorW = XTextWidth(f, Text().c_str(), (int) mCursorPos);
        int fontHeight = f->ascent + f->descent;
        int drawX = 0;
        int drawY = 0;
        switch (HAlign()) {
          case AUIHAlign::left:
            drawX = 5;
            break;
          case AUIHAlign::center:
            drawX = (SizeX() - totalW) / 2;
            break;
          case AUIHAlign::right:
            drawX = SizeX() - totalW - 5;
            break;
        }
        switch (VAlign()) {
          case AUIVAlign::top:
            drawY = f->ascent + 5;
            break;
          case AUIVAlign::center:
            drawY = (SizeY() + f->ascent - f->descent) / 2;
            break;
          case AUIVAlign::bottom:
            drawY = SizeY() - f->descent - 5;
            break;
        }
        XDrawString(d, w, gc, drawX, drawY, Text().c_str(), (int) Text().size());
        if (mCursorVisible) {
          int cursorX = drawX + textBeforeCursorW;
          int cursorYTop = drawY - f->ascent;
          XDrawLine(d, w, gc, cursorX, cursorYTop, cursorX, cursorYTop + fontHeight);
        }
  }

  void AInputBox::OnKeyPress(XEvent *ev) {
    mCursorVisible = true;
        KeySym keysym = XLookupKeysym(&ev->xkey, 0);
        std::string k = XKeysymToString(keysym);
        if(std::regex_match(k.c_str(), mFilter)) {
          Text().insert(mCursorPos++, k);
          D2("kb input '%s', data '%s'", k.c_str(), Text().c_str())
        } else {
          switch(string_to_case.count(k.c_str()) ? string_to_case.at(k.c_str()) : 0) {
            case 0:
              D1("Key pressed: '%s' and is filtered", k.c_str());
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
              if (mCursorPos > 0) mCursorPos--;
              break;
            case 6:
              if (mCursorPos < Text().size()) mCursorPos++;
              break;
            case 7:
              if (mCursorPos < Text().size()) Text().erase(mCursorPos, 1);
              break;
            default:
              E("strange value2(%lu)", string_to_case.at(k.c_str()))
              break;
          }
        }
        Draw();
  }

  void AInputBox::OnButtonPress([[maybe_unused]]XEvent *ev) {
    D3("ev %lu", (UINT64)ev)
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
    if(Text().size() > 0) {
      Text().erase(mCursorPos-- - 1, 1);
      D2("deleting character at pos %lu", mCursorPos--)
    }
    else {
      D1("data empty")
    }
  }

  void AInputBox::SetText(char *v) {
    AWidget::SetText(v);
    mCursorPos = Text().size();
    Draw();
  }

  AInputBox::~AInputBox() {
    D3()
    mStopBlink = true;
    if(mBlinkThread.joinable()) mBlinkThread.join();
  }
}

