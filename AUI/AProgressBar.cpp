#include "AProgressBar.h"
#include "AUILib.h"
#include <X11/Xlib.h>
#include <string>
namespace aui {
  AProgressBar::AProgressBar(AWidget* wParent) : AWidget() {
    if (!wParent) [[unlikely]] return;
    AUI* cg = wParent->AUIPtr();
    Display* d = cg->Disp();
    INT32 scr = cg->Scr();
    SetType(AUIWidgetType::defaultProgressBar);
    SetBGColor(AUI_DEFAULT_BUTTON_BG);
    SetXY(AUI_TABLE_X, AUI_TABLE_Y);
    SetSizeXY(200, 30);
    SetAUIPtr(cg);
    SetWndParent(wParent);
    SetBB(None);
    mProgressProvider = nullptr;
    InitWidgetProps(XCreateSimpleWindow(d, wParent->Wnd(), SafeINT32(X()), SafeINT32(Y()), SafeUINT32(SizeX()), SafeUINT32(SizeY()), 1, BlackPixel(d, scr), BGColor()));
    Window w = Wnd();
    XSelectInput(d, w, ExposureMask | StructureNotifyMask);
    XMapWindow(d, w);
    cg->AddWidget(this);
    // The background thread is now the SOLE authority responsible for executing Draw cycles at a controlled frame rate
    mUpdateThread = std::thread([this]() {
      while (!mStopThread) {
        std::unique_lock<std::mutex> lock(mThreadMutex);
        if (mThreadCv.wait_for(lock, std::chrono::milliseconds(mUpdateIntervalMs), [this] { return mStopThread; })) {
          break;
        }
        if (mProgressProvider != nullptr) {
          double fetchedProgress = mProgressProvider();
          if (fetchedProgress < 0.0) fetchedProgress = 0.0;
          if (fetchedProgress > 1.0) fetchedProgress = 1.0;
          mProgress = fetchedProgress;
        }
        lock.unlock(); // Release state lock before the thread takes exclusive control of the X11 draw pipeline
        Draw();
      }
    });
  }

  AProgressBar* AProgressBar::AttachTo(AWidget* wParent) {
    return new AProgressBar(wParent);
  }

  void AProgressBar::SetProgressProvider(std::function<double()> provider) {
    std::lock_guard<std::mutex> lock(mThreadMutex);
    mProgressProvider = provider;
  }

  void AProgressBar::SetProgress(double progress) {
    std::lock_guard<std::mutex> lock(mThreadMutex);
    if (progress < 0.0) progress = 0.0;
    if (progress > 1.0) progress = 1.0;
    mProgress = progress;
    // Thread-safe data mutation only. Draw() invocation is intentionally omitted to let the update thread handle rendering.
  }

  double AProgressBar::GetProgress() const {
    std::lock_guard<std::mutex> lock(mThreadMutex);
    return mProgress;
  }

  void AProgressBar::SetBarColor(UINT32 color) {
    mBarColor = color;
  }

  void AProgressBar::Clear() {
    std::lock_guard<std::mutex> lock(mThreadMutex);
    mProgress = 0.0;
  }

  void AProgressBar::SetUpdateInterval(UINT32 intervalMs) {
    std::lock_guard<std::mutex> lock(mThreadMutex);
    mUpdateIntervalMs = intervalMs;
  }

  void AProgressBar::Draw() {
    if (Wnd() == 0) return;
    AUI* au = AUIPtr();
    if (!au) return;
    GC gc = GCPtr();
    if (!gc) return;
    Display* d = au->Disp();
    XFontStruct* fo = Font();
    XLockDisplay(d);
    if (!BB()) {
      UpdateBuffer();
    }
    Drawable dest = BB();
    if (dest == 0) {
      dest = Wnd();
    }
    UINT32 w = static_cast<UINT32>(SizeX());
    UINT32 h = static_cast<UINT32>(SizeY());
    XSetForeground(d, gc, BGColor());
    XFillRectangle(d, dest, gc, 0, 0, w, h);
    double localProgress = 0.0;
    {
      std::lock_guard<std::mutex> lock(mThreadMutex);
      localProgress = mProgress;
    }
    UINT32 barWidth = static_cast<UINT32>(static_cast<double>(w) * localProgress);
    if (barWidth > 0) {
      XSetForeground(d, gc, mBarColor);
      XFillRectangle(d, dest, gc, 0, 0, barWidth, h);
    }
    int percent = static_cast<int>(localProgress * 100.0);
    std::string text = std::to_string(percent) + "%";
    int textLen = static_cast<int>(text.length());
    int textW = fo ? XTextWidth(fo, text.c_str(), textLen) : textLen * 7;
    int textX = (static_cast<int>(w) - textW) / 2;
    int textH = fo ? (fo->ascent + fo->descent) : 14;
    int textY = (static_cast<int>(h) - textH) / 2 + (fo ? fo->ascent : 11);
    XSetForeground(d, gc, 0x000000);
    XDrawString(d, dest, gc, textX, textY, text.c_str(), textLen);
    if (dest != Wnd()) {
      XCopyArea(d, dest, Wnd(), gc, 0, 0, w, h, 0, 0);
    }
    XSync(d, False);
    XUnlockDisplay(d);
  }

  AProgressBar::~AProgressBar() {
    {
      std::lock_guard<std::mutex> lock(mThreadMutex);
      mStopThread = true;
    }
    mThreadCv.notify_all();
    if (mUpdateThread.joinable()) {
      mUpdateThread.join();
    }
    D3()
  }
} // namespace aui
