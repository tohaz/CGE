#include "AUILib.h"

#include "AWidget.h"
#include "AWindow.h"

namespace aui {

  UINT32 GetBlendedColor(UINT32 baseColor, uint8_t target, double t) {
      RGBAColor c;
      c.value = baseColor;

      c.rgba.r = static_cast<uint8_t>(c.rgba.r + (target - c.rgba.r) * t);
      c.rgba.g = static_cast<uint8_t>(c.rgba.g + (target - c.rgba.g) * t);
      c.rgba.b = static_cast<uint8_t>(c.rgba.b + (target - c.rgba.b) * t);

      return c.value;
  }

  XRenderColor ScaleAndBlend(uint8_t base, uint8_t target, double t) {
    // Linear interpolation: base + (target - base) * t
    uint8_t blended8 = static_cast<uint8_t>(base + (static_cast<double>(target) - base) * t);

    // Manual scale to 16-bit to avoid any compiler warnings
    uint16_t val16 = static_cast<uint16_t>((static_cast<uint32_t>(blended8) << 8) | blended8);

    return XRenderColor { val16, val16, val16, 0xFFFF }; // We'll set R, G, B separately
  }

  UINT32 HLColor(UINT32 ci) {
    RGBAColor c;
    c.value = ci;
    UINT32 overall = static_cast<UINT32>(c.rgba.r) +
                       static_cast<UINT32>(c.rgba.g) +
                       static_cast<UINT32>(c.rgba.b);
    // 0x80 * 3 = 384
    if(overall > 384) {
      c.rgba.r = (c.rgba.r > AUI_HL_SHIFT) ? static_cast<uint8_t>(c.rgba.r - AUI_HL_SHIFT) : static_cast<uint8_t>(0);
      c.rgba.g = (c.rgba.g > AUI_HL_SHIFT) ? static_cast<uint8_t>(c.rgba.g - AUI_HL_SHIFT) : static_cast<uint8_t>(0);
      c.rgba.b = (c.rgba.b > AUI_HL_SHIFT) ? static_cast<uint8_t>(c.rgba.b - AUI_HL_SHIFT) : static_cast<uint8_t>(0);
    } else {
      c.rgba.r = (c.rgba.r < (255 - AUI_HL_SHIFT)) ? static_cast<uint8_t>(c.rgba.r + AUI_HL_SHIFT) : static_cast<uint8_t>(255);
      c.rgba.g = (c.rgba.g < (255 - AUI_HL_SHIFT)) ? static_cast<uint8_t>(c.rgba.g + AUI_HL_SHIFT) : static_cast<uint8_t>(255);
      c.rgba.b = (c.rgba.b < (255 - AUI_HL_SHIFT)) ? static_cast<uint8_t>(c.rgba.b + AUI_HL_SHIFT) : static_cast<uint8_t>(255);
      }
    return c.value;
  }

  // A milder version of HLColor for hovering
  UINT32 HoverColor(UINT32 ci) {
    RGBAColor c;
    c.value = ci;
    // We use a smaller shift for a subtle "glow"
    const uint8_t hover_shift = 12;
    UINT32 overall = static_cast<UINT32>(c.rgba.r)
        + static_cast<UINT32>(c.rgba.g) + static_cast<UINT32>(c.rgba.b);

    if(overall > 384) { // Brighter background -> darken slightly
      c.rgba.r = (c.rgba.r > hover_shift) ? c.rgba.r - hover_shift : 0;
      c.rgba.g = (c.rgba.g > hover_shift) ? c.rgba.g - hover_shift : 0;
      c.rgba.b = (c.rgba.b > hover_shift) ? c.rgba.b - hover_shift : 0;
    } else { // Darker background -> brighten slightly
      c.rgba.r =
          (c.rgba.r < (255 - hover_shift)) ? c.rgba.r + hover_shift : 255;
      c.rgba.g =
          (c.rgba.g < (255 - hover_shift)) ? c.rgba.g + hover_shift : 255;
      c.rgba.b =
          (c.rgba.b < (255 - hover_shift)) ? c.rgba.b + hover_shift : 255;
    }
    return c.value;
  }

  AUI::AUI() {
    D3("AUI sizeof {}", sizeof(AUI))
    mDisplay = XOpenDisplay(NULL);
    XInitThreads();
    if(mDisplay == NULL) {E("Cannot open default display")}
    D3("opened display {}", (UINT64)mDisplay)
    mScreen = DefaultScreen(mDisplay);
    CreateMainWindow();
    D("init finished, widget size {}, short size {}, int {}, long {}, float {}, double {}, ptr {}",
        sizeof(AWidget),
        sizeof(short), sizeof(int), sizeof(long),
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
    mShouldExit = false;
    D3("entering message loop")
    while (!mShouldExit) {
      D4("before XNextEvent")
      XNextEvent(mDisplay, &event);
      if(mShouldExit) {
        D1("===========XNextEvent fired while exit signal is active")
        return;
      }
      Window targetWin = event.xany.window;
      if(mWidg.contains(targetWin)) {
        AWidget* widget = mWidg[targetWin];
        switch (event.type) {
          case Expose:
            if(event.xexpose.count == 0) {
              if(mWidg.contains(event.xany.window)) {
                mWidg[event.xany.window]->Draw();
              } else if(mMainWnd && event.xany.window == mMainWnd->Wnd()) {
                mMainWnd->Draw();
              }
            }
            break;
          case NoExpose:
            D3("NoExpose event for widget {}", (UINT64)targetWin)
            break;
          case ButtonPress:
            D3("ButtonPress event for widget {}", (UINT64)targetWin)
            widget->CorrectCoordinates(event);
            widget->OnButtonPress(&event);

            break;
          case ButtonRelease:
            D3("ButtonRelease event for widget {}", (UINT64)targetWin)
            widget->CorrectCoordinates(event);
            widget->OnButtonRelease(&event);
            break;
          case MotionNotify:
            D3("MotionNotify event for widget {}", (UINT64)targetWin)
            widget->OnMouseMove(&event);
            break;
          case KeyPress:
            D2("KeyPress event for widget {}", (UINT64)targetWin)
            widget->OnKeyPress(&event);
            break;
          case FocusIn:
            D3()
            widget->OnFocusIn(&event);
            break;
          case FocusOut:
            D3()
            widget->OnFocusOut(&event);
            break;
          case EnterNotify:
            D3()
            widget->OnMouseEnter(&event);
            break;
          case LeaveNotify:
            D3()
            widget->OnMouseLeave(&event);
            break;
          case ClientMessage:
            if(targetWin == mMainWnd->Wnd()) {
              D1("Shutting down AUI (version '{}')", AUI_GIT_VERSION)
              ExitAUI();
              return;
            } else {
              D2("Closing secondary window {}", (UINT64)targetWin)
              RemoveWidget(targetWin);
            }
            break;
          case ConfigureNotify:
            D3("ConfigureNotify for widget {}", (UINT64)targetWin)
            break;
          case UnmapNotify:
            break;
          case MapNotify:
            break;
          case ReparentNotify:
            D3("System notification %d for window {}", event.type,
                (UINT64)targetWin)
            break;
          default:
            D("Event {} not processed for widget", event.type)
            break;
        }
      }
      else {
        if(event.type == Expose && mMainWnd && targetWin == mMainWnd->Wnd()) {
          if(event.xexpose.count == 0) {
            D3("Expose event for MAIN window")
            mMainWnd->Draw();
          }
        }
        else {
          D2("Event {} for non-registered window {}", event.type,
              (UINT64)targetWin)
        }
      }
    }
    if(mDisplay) {
      D2(">>>>>>>>>>>>>>>>>>>>>syncing XLib")
      XSync(mDisplay, False);
    } else {
      E("display vanished while message loop active")
    }
    D2("======================ProcessMessages ends")
  }

  AWindow* AUI::MainWnd() {
    return mMainWnd;
  }

  void AUI::AddWidget(AWidget *w) {
    AWidget* wp = w->ParentWidget();
    mWidg.insert({w->Wnd(), w});
    D2("adding widget type {}, reg sz={}, wnd {}", (UINT32)w->Type(), mWidg.size(), w->Wnd())
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
      E("attempt to erase missing key {}", (UINT64)w)
    }
  }

  void AUI::CloseDisplay() {
    if(mDisplay != 0) {
      D2("freeing display {}", (UINT64)mDisplay)
      XCloseDisplay(mDisplay);
      mDisplay = 0;
    }
    else {
      D2("not freeing display")
      DS()
    }
  }

  bool AUI::IsWindowRegistered(Window w) {
    return mWidg.contains(w);
  }

  void AUI::RemoveWidget(Window w) {
    // 1. Check if it exists in the global map
    if(!mWidg.contains(w)) {
      D2("Attempted to remove non-registered window {}", (UINT64)w);
      return;
    }
    AWidget *wi = mWidg[w];
    // 2. Remove from parent's internal list so the parent
    // doesn't try to delete it again later.
    if(wi->ParentWidget() != nullptr) {
      wi->ParentWidget()->UnregisterChild(wi);
    }
    // 3. Initiate destruction.
    // NOTE: We do NOT call UnregisterWindow(w) here.
    // ~AWidget will call it. This ensures the map entry exists
    // until the very start of the destructor.
    delete wi;
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
    UINT32 uEv = static_cast<UINT32>(ev);
    if(CGXEventNames.contains(uEv)) {
      return CGXEventNames.at(uEv).c_str();
    }
    E("unknown event encountered: {}", ev);
    return "unknown";
  }

  std::string AUI::NumberToBaseString(UINT64 n) {
    D3("entering with '{}', alphabet len '{}'", n, mAlphabetLen)
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
    D3("'{}'", result.c_str())
    return result;
  }

  AUI::~AUI() {
    D3("============AUI destructor starts");
    UNUSED size_t threadId = std::hash<std::thread::id> { }(
        std::this_thread::get_id());
    D2("ExitAUI called from thread {}", (UINT64)threadId);
    mShouldExit = true;
    // 1. Delete the Main Window first.
    // This triggers recursive destruction of all children via ~AWidget.
    if(mMainWnd != nullptr) {
      AWidget *root = mMainWnd;
      mMainWnd = nullptr;
      delete root;
    }
    // 2. Clear any leftover secondary/orphan windows.
    // We use a while loop because calling 'delete' on a widget
    // will trigger UnregisterWindow, which removes it from this map.
    while (!mWidg.empty()) {
      auto it = mWidg.begin();
      AWidget *w = it->second;
      if(w) {
        delete w;
      } else {
        mWidg.erase(it);
      }
    }
    // 3. Final X11 shutdown
    if(mDisplay != nullptr) {
      XSync(mDisplay, False);
      XCloseDisplay(mDisplay);
      mDisplay = nullptr;
    }
    D3("============AUI destructor ends");
  }
}
