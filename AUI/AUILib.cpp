#include "AUILib.h"

#include "AWidget.h"
#include "AWindow.h"


namespace aui {
  INT32 SafeINT32(UINT32 val) {
    if(val > 0x7FFFFFFF) {
      E("UINT32 to INT32 conversion error")
    }
    return val;
  }

  INT16 SafeINT16(UINT16 val) {
    if(val >= 0x8000) {
      E("UINT16 to INT16 conversion error")
    }
    return val;
  }

  INT16 SafeINT16(UINT32 val) {
    if(val >= 0x8000) {
      E("UINT16 to INT16 conversion error")
    }
    return val;
  }

  UINT16 SafeUINT16(UINT32 val) {
    if(val >= 0x10000) {
      E("UINT32 to UINT16 conversion error")
    }
    return val;
  }

  UINT32 HLColor(UINT32 ci) {
    RGBAColor c;
    c.value = ci;
    UINT32 overall = c.r + c.g + c.b;
    // 0x80 * 3 = 384
    if(overall > 384) {
      if(c.r > AUI_HL_SHIFT) c.r -= AUI_HL_SHIFT;
      else c.r = 0;
      if(c.g > AUI_HL_SHIFT) c.g -= AUI_HL_SHIFT;
      else c.g = 0;
      if(c.b > AUI_HL_SHIFT) c.b -= AUI_HL_SHIFT;
      else c.b = 0;
    }
    else {
      if(c.r < 255) c.r += AUI_HL_SHIFT;
      else c.r = 255;
      if(c.g < 255) c.g += AUI_HL_SHIFT;
      else c.g = 255;
      if(c.b < 255) c.b += AUI_HL_SHIFT;
      else c.b = 255;
    }
    return c.value;
  }

  AUI::AUI() {
    D3("AUI sizeof %lu", sizeof(AUI))
    mDisplay = XOpenDisplay(NULL);
    XInitThreads();
    if (mDisplay == NULL) E("Cannot open default display")
    D2("opened display %lu", mDisplay)
    mScreen = DefaultScreen(mDisplay);
    CreateMainWindow();
    D1("init finished, widget size %lu, short size %lu, int %lu, long %lu, float %lu, double %lu, ptr %lu",
        sizeof(AWidget),
        sizeof(short), sizeof(long), sizeof(long int),
        sizeof(float), sizeof(double), sizeof(void*))
  }

  AUI::AUI(std::string newWindowTitle) : AUI() {
    mWindowTitle = newWindowTitle;
    mMainWnd->SetTitle(newWindowTitle.c_str());
    XStoreName(mDisplay, mMainWnd->Wnd(), mWindowTitle.c_str());
    D3()
  }

  void AUI::CreateMainWindow() {
    D3();
    if(mMainWnd == 0) {
      mMainWnd = AWindow::AttachTo(this, mWindowTitle.c_str());
    }
    else E("reinitializing attempt on main window")
    mWMDeleteMessage = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(mDisplay, mMainWnd->Wnd(), &mWMDeleteMessage, 1);
    XFlush(mDisplay);
  }

  INT32 AUI::Scr() {
    return mScreen;
  }

  Display* AUI::Disp() {
    return mDisplay;
  }

  void AUI::ProcessMessages() {
      XEvent event;
      while(!mShouldExit) {
          XNextEvent(mDisplay, &event);
          // 1. Используем универсальное поле для получения ID окна для любого типа события
          Window targetWin = event.xany.window;
          // 2. Если окно зарегистрировано в карте виджетов
          if(mWidg.contains(targetWin)) {
              AWidget* widget = mWidg[targetWin];
              switch(event.type) {
                  case Expose:
                    if (event.xexpose.count == 0) {
                        if(mWidg.contains(event.xany.window)) {
                            mWidg[event.xany.window]->Draw();
                        } else if (mMainWnd && event.xany.window == mMainWnd->Wnd()) {
                            // ОЧЕНЬ ВАЖНО: Родитель должен чистить свой фон, иначе под кнопками будет "пунктир"
                            mMainWnd->Draw();
                        }
                    }
                      break;
                  case NoExpose:
                    D3("NoExpose event for widget %lu", (UINT64)targetWin)
                      break;
                  case ButtonPress:
                    D3("ButtonPress event for widget %lu", (UINT64)targetWin)
                      widget->OnButtonPress(&event);
                      break;

                  case ButtonRelease:
                      D3("ButtonRelease event for widget %lu", (UINT64)targetWin)
                      widget->OnButtonRelease(&event);
                      break;

                  case MotionNotify:
                      D3("MotionNotify event for widget %lu", (UINT64)targetWin)
                      widget->OnMouseMove(&event);
                      break;

                  case KeyPress:
                      D2("KeyPress event for widget %lu", (UINT64)targetWin)
                      widget->OnKeyPress(&event);
                      break;

                  case ClientMessage:
                      // Если это сообщение закрытия для главного окна
                      if(targetWin == mMainWnd->Wnd()) {
                          D1("Shutting down AUI...")
                          ExitAUI();
                          return;
                      } else {
                          D2("Closing secondary window %lu", (UINT64)targetWin)
                          RemoveWidget(targetWin);
                      }
                      break;

                  case ConfigureNotify:
                      D3("ConfigureNotify for widget %lu", (UINT64)targetWin)
                      // Здесь можно добавить UpdateBuffer() при изменении размера
                      break;

                  case UnmapNotify:
                  case MapNotify:
                  case ReparentNotify:
                      D3("System notification %d for window %lu", event.type, (UINT64)targetWin)
                      break;

                  default:
                      D("Event %d not processed for widget", event.type)
                      break;
              }
          }
          // 3. Если окно НЕ в карте (например, это само главное окно, если оно не в mWidg)
          else {
              if (event.type == Expose && mMainWnd && targetWin == mMainWnd->Wnd()) {
                  if (event.xexpose.count == 0) {
                      D3("Expose event for MAIN window")
                      mMainWnd->Draw(); // Очистка фона родителя фиксит глитчи под рамками
                  }
              } else {
                  D2("Event %d for non-registered window %lu", event.type, (UINT64)targetWin)
              }
          }
      }
  }

  AWindow* AUI::MainWnd() {
    return mMainWnd;
  }

  void AUI::AddWidget(AWidget *w) {
    AWidget* wp = w->ParentWidget();
    mWidg.insert({w->Wnd(), w});
    D2("adding widget type %d, reg sz=%lu, wnd %lu", w->Type(), mWidg.size(), w->Wnd())
    if(wp != 0) {
      D3("widg has parent")
      wp->AddWidgetChild(w);
    }
  }

  AUI* AUI::Create(std::string windowTitle) {
    return new AUI(windowTitle);
  }

  void AUI::ExitAUI() {
    mShouldExit = true;
    XEvent exev;
    memset(&exev, 0, sizeof(exev));
    exev.type = Expose;
    Window w = mMainWnd->Wnd();
    exev.xexpose.window = w;
    XSendEvent(mDisplay, w, False, ExposureMask, &exev);
    XFlush(mDisplay);
  }

  void AUI::UnregisterWindow(Window w) {
    if(mWidg.contains(w)) {
      mWidg.erase(w);
    }
    else {
      DS1()
      E("attempt to erase missing key %lu", (UINT64)w)
    }
  }

  void AUI::CloseDisplay() {
    if(mDisplay != 0) {
      D2("freeing display %lu", mDisplay)
      XCloseDisplay(mDisplay);
      mDisplay = 0;
    }
    else {
      D2("not freeing display")
    }
  }

  bool AUI::IsWindowRegistered(Window w) {
    return mWidg.contains(w);
  }

  void AUI::RemoveWidget(Window w) {
    AWidget *wi = mWidg[w];
    UnregisterWindow(w);
    wi->DestroyChildWidgets();
    if(wi->ParentWidget() != 0) {
      wi->ParentWidget()->UnregisterChild(wi);
    }
    delete wi;
  }
  // purpose: xlib expects all secondary windows to be closed before main one
  // otherwise it will crash or app would leak memory
  void AUI::RemoveMiscWindows() {
    D2("Removing misc windows")
    AWidget* w = 0;
    Window wmn = mMainWnd->Wnd();
    std::stack<Window> remS;
    for(auto const& [key, val] : mWidg) {
      if(key != wmn) {
        w = (AWidget*) val;
        if(w->ParentWidget() == 0) {
          remS.push(w->Wnd());
        }
      }
    }
    while (!remS.empty()) {
      RemoveWidget(remS.top());
      remS.pop();
    }
    D2("Finished removing misc windows")
  }

  void AUI::RemoveParentWidget(Window w) {
    AWidget* wi = GetWidget(w);
    RemoveWidget(wi->ParentWidget()->Wnd());
  }

  void AUI::RemoveParentWidget(AWidget* w) {
    RemoveWidget(w->ParentWidget()->Wnd());
  }

  AWidget* AUI::GetWidget(Window w) {
    if(mWidg.contains(w)) {
      return mWidg[w];
    }
    E("attempt to retrieve non-existing widget")
    return nullptr;
  }

  static inline std::map<UINT32, std::string> CGXEventNames {
  {2,"KeyPress"},{3,"KeyRelease"},{4,"ButtonPress"},{5,"ButtonRelease"},{6,"MotionNotify"},
  {7,"EnterNotify"},{8,"LeaveNotify"},{9,"FocusIn"},{10,"FocusOut"},{11,"KeymapNotify"},{12,"Expose"},
  {13,"GraphicsExpose"},{14,"NoExpose"},{15,"VisibilityNotify"},{16,"CreateNotify"},{17,"DestroyNotify"},
  {18,"UnmapNotify"},{19,"MapNotify"},{20,"MapRequest"},{21,"ReparentNotify"},{22,"ConfigureNotify"},
  {23,"ConfigureRequest"},{24,"GravityNotify"},{25,"ResizeRequest"},{26,"CirculateNotify"},
  {27,"CirculateRequest"},{28,"PropertyNotify"},{29,"SelectionClear"},{30,"SelectionRequest"},
  {31,"SelectionNotify"},{32,"ColormapNotify"},{33,"ClientMessage"},{34,"MappingNotify"},
  {35,"GenericEvent"}
  };

  const char* AUI::XEventToString(INT32 ev) {
    std::string s;
    if(CGXEventNames.contains(ev)) {
      return CGXEventNames[ev].c_str();
    }
    E("unknown event encountered")
    return "unknown";
  }

  std::string AUI::NumberToBaseString(UINT64 n) {
    //D3("entering with '%lu', alphabet len '%lu'", n, mAlphabetLen)
    std::string result = "";
    do {
      result += BaseAlphabet[n % mAlphabetLen];
      n = n / mAlphabetLen;
      if (n > 0) {
          n--;
      } else {
          break;
      }
    } while (true);
    std::reverse(result.begin(), result.end());
    D3("'%s'", result.c_str())
    return result;
  }

  AUI::~AUI() {
    D3("============AUI destructor starts, map sz %lu", mWidg.size());
    if(mMainWnd != 0) {
      UnregisterWindow(mMainWnd->Wnd());
      D2("unregistered main window, widg map, size is %lu", mWidg.size())
    }
    if((UINT64)mWidg.size() > 0) {
      D2("erasing widget map size %lu", mWidg.size());
      for (auto it = mWidg.begin(); it != mWidg.end(); ++it) {
          delete it->second;
      }
      mWidg.clear();
    }
    else {
      D2("not erasing widget map")
    }
    if(mMainWnd != 0) {
      D2("deleting main window")
      delete mMainWnd;
      mMainWnd = 0;
    }
    else {
      D2("not deleting main window")
    }
    CloseDisplay();
    D3("============AUI destructor ends");
  }
}
