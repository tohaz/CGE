#include "AModalWindow.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "AUILib.h"
#include <sstream>
#include <cmath>
#include <ctime>

namespace aui {

  AModalWindow::AModalWindow(AWidget *wParent, std::string title, std::string text) :
      mMessage(std::move(text)), mIsShaking(false) {
    SetAUIPtr(wParent->AUIPtr());
    SetTitle(std::move(title));
    SetType(AUIWidgetType::unset);
    Display *d = AUIPtr()->Disp();
    UINT32 winW = 0, winH = 0;
    CalculateLayout(winW, winH);
    SetSize(winW, winH);
    static int cascadeStep = 0;
    INT64 posX = (static_cast<INT64>(wParent->SizeX())
        - static_cast<INT64>(winW)) / 2;
    INT64 posY = (static_cast<INT64>(wParent->SizeY())
        - static_cast<INT64>(winH)) / 2;
    cascadeStep = (cascadeStep + 20) % 120; // Loops cascading offset coordinates cleanly
    if(posX < 0)
      posX = 10;
    if(posY < 0)
      posY = 10;
    SetXY(posX, posY);
    mBaseX = static_cast<INT32>(posX);
    mBaseY = static_cast<INT32>(posY);
    // Create dialog window directly inside the main application window
    Window w = XCreateSimpleWindow(d, wParent->Wnd(), mBaseX, mBaseY, winW,
        winH, 1, BlackPixel(d, 0), wParent->BGColor());
    XSelectInput(d, w,
        ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask
            | StructureNotifyMask);
    InitWidgetProps(w);
    UpdateBuffer();
    this->SetWndParent(wParent);
    AUIPtr()->AddWidget(this);
    UINT32 btnW = 100;
    UINT32 btnH = 32;
    INT32 btnX = static_cast<INT32>((winW - btnW) / 2);
    INT32 btnY = static_cast<INT32>(winH) - mPadding - static_cast<INT32>(btnH);
    mOkBtn = AButton::AttachTo(this, "OK");
    mOkBtn->Move(static_cast<UINT32>(btnX), static_cast<UINT32>(btnY));
    mOkBtn->Resize(btnW, btnH);
    mOkBtn->SetOnButtonPressCB(AModalWindow::OnOkClick, this);
    XMapWindow(d, w);
    XRaiseWindow(d, w);
    AUIPtr()->PushModal(this);
  }

  void AModalWindow::Message(AWidget *wParent, std::string title, std::string text) {
    if(!wParent) {
      E("Cannot create modal window with null parent!");
    }
    UNUSED AModalWindow *modal = new AModalWindow(wParent, title, text);
  }

  AModalWindow* AModalWindow::AttachTo(AWidget *wParent, std::string title, std::string text) {
    if(!wParent) {
      E("Cannot create modal window with null parent!");
      return nullptr;
    }
    return new AModalWindow(wParent, title, text);
  }

  void AModalWindow::WrapText(UINT32 maxWidth) {
    mWrappedLines.clear();
    XFontStruct *font = Font();
    if(!font)
      return;
    std::stringstream ss(mMessage);
    std::string paragraph;
    while (std::getline(ss, paragraph, '\n')) {
      std::stringstream words(paragraph);
      std::string word;
      std::string currentLine;
      while (words >> word) {
        std::string testLine =
            currentLine.empty() ? word : currentLine + " " + word;
        int width = XTextWidth(font, testLine.c_str(),
            static_cast<int>(testLine.length()));
        if(width > static_cast<int>(maxWidth) && !currentLine.empty()) {
          mWrappedLines.push_back(currentLine);
          currentLine = word;
        } else {
          currentLine = testLine;
        }
      }
      if(!currentLine.empty()) {
        mWrappedLines.push_back(currentLine);
      }
    }
  }

  void AModalWindow::CalculateLayout(UINT32 &outW, UINT32 &outH) {
    UINT32 minWidth = 320;
    UINT32 maxWidth = 550;
    XFontStruct *font = nullptr;
    if(TryLoadFont()) {
      D4("AModalWindow font loaded")
      font = Font();
    }
    else {
      E("failed to load font")
    }
    UINT32 calculatedMaxTextW = maxWidth - static_cast<UINT32>(mPadding * 2);
    WrapText(calculatedMaxTextW);
    int maxLineW = 0;
    if(font) {
      for (const auto &line : mWrappedLines) {
        int w = XTextWidth(font, line.c_str(), static_cast<int>(line.length()));
        if(w > maxLineW)
          maxLineW = w;
      }
    }
    outW = static_cast<UINT32>(maxLineW) + static_cast<UINT32>(mPadding * 2);
    if(outW < minWidth)
      outW = minWidth;
    if(outW > maxWidth)
      outW = maxWidth;
    int lineHeight = font ? (font->ascent + font->descent + 4) : 18;
    UINT32 textZoneH =
        static_cast<UINT32>(static_cast<int>(mWrappedLines.size()) * lineHeight);
    outH = static_cast<UINT32>(mTitleHeight) + static_cast<UINT32>(mPadding)
        + textZoneH + static_cast<UINT32>(mPadding) + 32U
        + static_cast<UINT32>(mPadding);
  }

  void AModalWindow::Draw() {
    Display *d = AUIPtr()->Disp();
    Pixmap buffer = BB();
    GC gc = GCPtr();
    XFontStruct *font = Font();
    if(!d || !buffer || !gc)
      return;
    // If BGColor() is uninitialized (0), fallback to standard light gray
    unsigned long bg = BGColor();
    if(bg == 0) {
      bg = XWhitePixel(d, 0); // Or use a hardcoded gray color hex if available
    }
    // 1. Clear the dialog inner canvas with its native background color
    XSetForeground(d, gc, bg);
    XFillRectangle(d, buffer, gc, 0, 0, SizeXUI32(), SizeYUI32());
    // 2. Render the dialogue header/title bar area
    XSetForeground(d, gc, BlackPixel(d, 0));
    XFillRectangle(d, buffer, gc, 0, 0, SizeXUI32(),
        static_cast<unsigned int>(mTitleHeight));
    if(font) {
      XSetFont(d, gc, font->fid);
    }
    if(font && !Title().empty()) {
      XSetForeground(d, gc, WhitePixel(d, 0));
      int titleBarH = mTitleHeight;
      int textH = font->ascent + font->descent;
      int titleY = ((titleBarH - textH) / 2) + font->ascent;
      XDrawString(d, buffer, gc, mPadding, titleY, Title().c_str(),
          static_cast<int>(Title().length()));
    }
    // 3. Render the wrapped multi-line message body text
    XSetForeground(d, gc, BlackPixel(d, 0));

    if(mWrappedLines.empty() && !mMessage.empty()) {
      UINT32 calculatedMaxTextW = SizeXUI32()
          - static_cast<UINT32>(mPadding * 2);
      WrapText(calculatedMaxTextW);
    }
    UINT32 uLineHeight =
        font ? static_cast<UINT32>(font->ascent + font->descent + 4) : 18U;
    UINT32 uTitleHeight = static_cast<UINT32>(mTitleHeight);
    UINT32 uPadding = static_cast<UINT32>(mPadding);
    // Dynamic Zone Calculation:
    // Upper boundary: bottom of the Title Bar (uTitleHeight)
    // Lower boundary: top of the Button Area. The button is placed at: winH - mPadding - 32U
    UINT32 buttonZoneTop = SizeYUI32() - uPadding - 32U;
    // The strict drawing zone height between titlebar and button area
    UINT32 messageZoneH = buttonZoneTop - uTitleHeight;
    UINT32 totalTextHeight = static_cast<UINT32>(mWrappedLines.size())
        * uLineHeight;
    int currentY = mTitleHeight; // Base starting point
    if(messageZoneH > totalTextHeight) {
      // Offset starting Y position so the entire text block balances perfectly in the middle
      currentY += static_cast<int>((messageZoneH - totalTextHeight) / 2U);
    }
    // Adjust currentY to target the first line's baseline
    currentY += (font ? font->ascent : 12);
    for (const auto &line : mWrappedLines) {
      int currentX = mPadding;
      // Horizontal Centering
      if(HAlign() == AUIHAlign::center && font) {
        int lw = XTextWidth(font, line.c_str(),
            static_cast<int>(line.length()));
        currentX =
            static_cast<int>((SizeXUI32() - static_cast<unsigned int>(lw)) / 2U);
      }
      XDrawString(d, buffer, gc, currentX, currentY, line.c_str(),
          static_cast<int>(line.length()));
      // Advance vertical baseline to the next line row
      currentY += static_cast<int>(uLineHeight);
    }
    // 4. Draw high-contrast outer window borders
    XSetForeground(d, gc, BlackPixel(d, 0));
    XDrawRectangle(d, buffer, gc, 0, 0, SizeXUI32() - 1, SizeYUI32() - 1);
    // Copy the completely formed back-buffer to the actual screen window
    XCopyArea(d, buffer, Wnd(), gc, 0, 0, SizeXUI32(), SizeYUI32(), 0, 0);
    // 5. Force the OK button to update its geometry and render inside our canvas
    if(mOkBtn) {
      mOkBtn->Draw();
    }
    // Flush all drawing commands to the server immediately
    XFlush(d);
  }

  void AModalWindow::TriggerFeedback() {
    if(mIsShaking)
      return;
    mIsShaking = true;
    Display *d = AUIPtr()->Disp();
    int offsets[] = { -12, 12, -8, 8, -4, 4, 0 };
    for (int offset : offsets) {
      XMoveWindow(d, Wnd(), mBaseX + offset, mBaseY);
      // Force the X Server queue to process spatial translation updates immediately
      XFlush(d);
      // REFACTOR: Preserved simple structural sleep sequence but added defensive checks
      struct timespec ts = { 0, 20000000 }; // 20ms delay sequence
      nanosleep(&ts, nullptr);
    }
    mIsShaking = false;
    Draw();
  }

  void AModalWindow::OnOkClick(UNUSED XEvent *ev, UNUSED AWidget *w, void *data) {
    D4("ENTERED: AModalWindow::OnOkClick callback triggered!");
    auto *modal = reinterpret_cast<AModalWindow*>(data);
    if(modal) {
      modal->Close();
    } else {
      D1("ERROR: Modal callback data context is NULL!");
    }
  }

  void AModalWindow::OnButtonPress(XEvent *ev) {
    if(ev->xbutton.button == Button1) {
      if(ev->xbutton.y >= 0 && ev->xbutton.y <= mTitleHeight) {
        mIsDragging = true;
        mDragOffsetX = ev->xbutton.x;
        mDragOffsetY = ev->xbutton.y;

        // PERFORMANCE FIX: Cache parent coords once upon mouse down to eliminate X Server lag
        Display *d = AUIPtr()->Disp();
        AWidget *wParent = ParentWidget();
        if(d && wParent) {
          Window rootWinReturn;
          XTranslateCoordinates(d, wParent->Wnd(), XRootWindow(d, 0), 0, 0,
              &mParentAbsX, &mParentAbsY, &rootWinReturn);
        }
        D2("Modal window drag sequence started");
        return;
      }
    }
    AWidget::OnButtonPress(ev);
  }

  void AModalWindow::Close() {
    D4("ENTERED: AModalWindow::Close() deferred sequence starting");
    Display *d = AUIPtr()->Disp();
    if(!d) {
      D1("ERROR: Display pointer is NULL in Close()");
      return;
    }

    // 1. Muted and clean isolation: instantly release focus and hide window from screen
    AUIPtr()->PopModal(this);
    XUnmapWindow(d, Wnd());
    XFlush(d);

    // 2. Formulate a custom ClientMessage to trigger asynchronous deletion safely
    XEvent delEvent;
    memset(&delEvent, 0, sizeof(delEvent));
    delEvent.type = ClientMessage;
    delEvent.xclient.window = Wnd();
    delEvent.xclient.message_type = XInternAtom(d, "WM_DELETE_WINDOW", False);
    delEvent.xclient.format = 32;

    // 3. Dispatch the event directly to the main loop architecture queue
    XSendEvent(d, Wnd(), False, NoEventMask, &delEvent);
    XFlush(d);

    D4("SUCCESS: Close notification posted to X-Server loop queue");
  }

  void AModalWindow::OnMouseMove(XEvent *ev) {
    if(mIsDragging) {
      AWidget *wParent = ParentWidget();
      if(!wParent || !AUIPtr()) return;

      // Render coordinate values fast utilizing local cached absolute position structures
      int newX = ev->xbutton.x_root - mParentAbsX - mDragOffsetX;
      int newY = ev->xbutton.y_root - mParentAbsY - mDragOffsetY;

      if(newY < 0) newY = 0;
      if(newX < 0) newX = 0;

      this->Move(static_cast<UINT32>(newX), static_cast<UINT32>(newY));
      mBaseX = static_cast<INT32>(newX);
      mBaseY = static_cast<INT32>(newY);
      return;
    }
    AWidget::OnMouseMove(ev);
  }

  const std::vector<std::string>& AModalWindow::WrappedLines() const {
    return mWrappedLines;
  }

  void AModalWindow::OnButtonRelease(XEvent *ev) {
    if(ev->xbutton.button == Button1 && mIsDragging) {
      mIsDragging = false;
      D2("Modal window drag sequence finished");
      return;
    }
    AWidget::OnButtonRelease(ev);
  }

  AModalWindow::~AModalWindow() {
    D3("AModalWindow destructor executed");
  }

} // namespace aui
