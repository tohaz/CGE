#include "AWidget.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "AUILib.h"

namespace aui {

  AWidget::AWidget() {
    D3("sizeof widget {}", sizeof(AWidget))
//    OnButtonPressCB = [this](XEvent* ev, CGWidget* w, void *data) {
//      D1("default OnButtonPressCB callback fired")
//    };
  }

  bool AWidget::TryLoadFont() {
    Display *d = mAUI->Disp();
    if(!mFont) {
      mFont = XLoadQueryFont(d, AUI_DEFAULT_FONT);
      if(!mFont) {
        D("Cannot open font {}, using 'fixed' instead", AUI_DEFAULT_FONT)
        mFont = XLoadQueryFont(d, "fixed");
        if(!mFont) {
          E("Cannot open 'fixed' font either, exit.")
          return false;
        }
      }
      D4("opened font with description:{}, Pointer: {}, Real X11 FID: {}",
         AUI_DEFAULT_FONT, (UINT64)mFont, (UINT64)mFont->fid);
    }
    else {
      D3("Font is already set")
    }
    return true;  }

  void AWidget::InitWidgetProps(Window w) {
    D3("window {}", (UINT64) w)
    Display *d = mAUI->Disp();
    if(mWindow == 0)
      mWindow = w;
    else
      E("attempt to reset window")

    if(mGC == 0) {
      mGC = XCreateGC(d, mWindow, 0, NULL);
      if(!mGC) {
        E("Error creating GC")
      }
    }
    else {
      D3("GC is already set")
    }
    if(!TryLoadFont()) {
      E("font load failure")
    }
    if(mGC && mFont) {
      XSetFont(d, mGC, mFont->fid);
    }
    XSetWindowAttributes swa;
    swa.bit_gravity = ForgetGravity;
    XChangeWindowAttributes(d, mWindow, CWBitGravity, &swa);
  }

  AUI* AWidget::AUIPtr() {
    return mAUI;
  }

  void AWidget::SetAUIPtr(AUI *p) {
    mAUI = p;
  }

  Window AWidget::Wnd() {
    return mWindow;
  }

  AUIWidgetType AWidget::Type() {
    return mType;
  }

  void AWidget::SetTitle(std::string newTitle) {
    D3("setting title to {}", newTitle.c_str())
    mTitle = newTitle;
  }

  void AWidget::SetText(std::string newText) {
    mText = newText;
    Draw();
  }

  void AWidget::AddText(std::string newText) {
    mText += newText;
    Draw();
  }

  void AWidget::SetType(AUIWidgetType inType) {
    mType = inType;
  }

  void AWidget::SetXY(INT64 newX, INT64 newY) {
    mX = newX;
    mY = newY;
  }

  INT64 AWidget::X() {
    return mX;
  }

  INT64 AWidget::Y() {
    return mY;
  }

  UINT64 AWidget::SizeX() {
    return mSzX;
  }

  UINT64 AWidget::SizeY() {
    return mSzY;
  }

  UINT32 AWidget::XUI32() {
    if(mX > 0xFFFFFFFF) {
      E("X overflow")
    }
    return (UINT32) mX;
  }

  UINT32 AWidget::YUI32() {
    if(mY > 0xFFFFFFFF) {
      E("Y overflow")
    }
    return (UINT32) mY;
  }

  UINT32 AWidget::SizeXUI32() {
    if(mSzX > 0xFFFFFFFF) {
      E("SizeX overflow")
    }
    return (UINT32) mSzX;
  }

  UINT32 AWidget::SizeYUI32() {
    if(mSzY > 0xFFFFFFFF) {
      E("SizeY overflow")
    }
    return (UINT32) mSzY;
  }

  void AWidget::SetSizeXY(UINT64 newSzX, UINT64 newSzY) {
    mSzX = newSzX;
    mSzY = newSzY;
  }

  void AWidget::SetBGColor(UINT32 newBGColor) {
    mBGColor.value = newBGColor;
  }

  UINT32 AWidget::BGColor() {
    return mBGColor.value;
  }

  void AWidget::Draw() {
    E("default draw routine called")
  }

  const CHAR8* AWidget::TextPtr() {
    return (const CHAR8*) mText.c_str();
  }

  std::string& AWidget::Text() {
    return mText;
  }

  std::string& AWidget::Title() {
    return mTitle;
  }

  XFontStruct* AWidget::Font() {
    return mFont;
  }

  GC AWidget::GCPtr() {
    return mGC;
  }

  bool AWidget::IsHL() {
    return mHL;
  }

  void AWidget::HL(bool newState) {
    mHL = newState;
  }

  AWidget* AWidget::ParentWidget() {
    D3("wndparent get {}", (UINT64) mWindowParent)
    return mWindowParent;
  }

  void AWidget::SetWndParent(AWidget *newParent) {
    mWindowParent = newParent;
    D3("wmdparent set {}", (UINT64) mWindowParent)
  }

  void AWidget::AddWidgetChild(AWidget *newChild) {
    mWidg[newChild->Wnd()] = newChild;
  }

  void AWidget::DestroyChildWidgets() {
    if(mWidg.size() > 0) {
      D2(">widget '{}' has children", mTitle.c_str());
      while (!mWidg.empty()) {
        auto it = mWidg.begin();
        AWidget *child = (AWidget*) it->second;
        // 1. Remove from the local parent map first to prevent re-entry/infinite loops
        mWidg.erase(it);
        if(child) {
          // 2. We do NOT set child->mWindow = 0 here.
          // The child needs its ID to successfully call UnregisterWindow() in its own destructor.
          // 3. Instead, we use a flag or check in the base destructor to see
          // if the X server already killed the window.
          //child->mWindow = 0;
          delete child;
        }
      }
    } else {
      D2(">widget '{}' doesn't have children", mTitle.c_str());
    }
  }

  void AWidget::DisableResize() {
    D3();
    mResizeEnabled = false;
    XSizeHints *hints = XAllocSizeHints();
    // Use the current size as both min and max
    hints->min_width = hints->max_width = SafeINT32(mSzX);
    hints->min_height = hints->max_height = SafeINT32(mSzY);
    hints->base_width = SafeINT32(mSzX);
    hints->base_height = SafeINT32(mSzY);
    hints->flags = PMinSize | PMaxSize | PBaseSize; // Set flags explicitly
    XSetWMNormalHints(mAUI->Disp(), mWindow, hints);
    XFree(hints);
  }

  void AWidget::EnableResize() {
    D3();
    mResizeEnabled = true;
    XSizeHints *hints = XAllocSizeHints();
    hints->min_width = 100;
    hints->min_height = 100;
    // Instead of clearing flags, set a massive MaxSize
    hints->max_width = 10000;
    hints->max_height = 10000;
    hints->base_width = SafeINT32(mSzX);
    hints->base_height = SafeINT32(mSzY);
    hints->flags = PMinSize | PMaxSize | PBaseSize;
    XSetWMNormalHints(mAUI->Disp(), mWindow, hints);
    XFree(hints);
  }

  void AWidget::Move(UINT32 x, UINT32 y) {
    mX = x;
    mY = y;
    XMoveWindow(mAUI->Disp(), mWindow, SafeINT32(x), SafeINT32(y));
    XSync(mAUI->Disp(), False);
  }

  void AWidget::UnregisterChild(AWidget *w) {
    Window wnd = w->Wnd();
    if(mWidg.contains(wnd)) {
      mWidg.erase(wnd);
    } else {
      E("attempt to remove child that is not registered")
    }
  }

  void AWidget::OnKeyPress([[maybe_unused]]XEvent *ev) {
    E("default key press handler should not be used")
  }

  void AWidget::SetBorderSz(unsigned int borderSz) {
    if(!mAUI || mWindow == 0) {
      mBorderSz = borderSz; // Сохраняем для инициализации
      return;
    }
    if(mBorderSz == borderSz)
      return;
    mBorderSz = borderSz;
    Display *d = mAUI->Disp();

    XSetWindowBorderWidth(d, mWindow, mBorderSz);
    XSync(d, False);
  }

  UINT32 AWidget::BorderSz() {
    return mBorderSz;
  }

  void AWidget::SetOnButtonPressCB(
      std::function<void(XEvent *ev, AWidget *w, void *arbdata)> func,
      void *data) {
    OnButtonPressCB = func;
    mUserDataButtonPress = data;
  }

  void AWidget::SetOnButtonReleaseCB(
      std::function<void(XEvent *ev, AWidget *w, void *arbdata)> func,
      void *data) {
    OnButtonReleaseCB = func;
    mUserDataButtonRelease = data;
  }

  void AWidget::SetOnSubmitCB(
      std::function<void(AWidget *w, void *arbdata)> func, void *data) {
    OnSubmitCB = func;
    mUserDataSubmit = data;
  }

  void AWidget::OnFocusIn(UNUSED XEvent *ev) {
    D3()
    if(OnFocusInCB == 0) {
      D3("CB is not set")
      return;
    } else {
      D3("CB is set")
      OnFocusInCB(ev, this, mUserDataFocusIn);
    }
  }

  void AWidget::OnFocusOut(UNUSED XEvent *ev) {
    D3()
    if(OnFocusOutCB == 0) {
      D3("CB is not set")
      return;
    } else {
      D3("CB is set")
      OnFocusOutCB(ev, this, mUserDataFocusOut);
    }
  }

  void AWidget::OnButtonPress(XEvent *ev) {
    D2()
    if(OnButtonPressCB == 0) {
      D2("CB is not set")
      return;
    } else {
      D2("CB is set")
      OnButtonPressCB(ev, this, mUserDataButtonPress);
    }
  }

  void AWidget::OnButtonRelease(XEvent *ev) {
    if(OnButtonReleaseCB == 0) {
      D3("CB is not set")
      return;
    } else {
      D3("CB is set")
      OnButtonReleaseCB(ev, this, mUserDataButtonRelease);
    }
  }

  void AWidget::OnMouseMove(XEvent *ev) {
    if(OnMouseMoveCB == 0) {
      D3("CB is not set")
      return;
    } else {
      D3("CB is set")
      OnMouseMoveCB(ev, this, mUserDataMouseMove);
    }
  }

  void AWidget::OnSubmit() {
    if(OnSubmitCB == 0) {
      D3("CB is not set")
      return;
    } else {
      D3("CB is set")
      OnSubmitCB(this, mUserDataSubmit);
    }
  }

  void AWidget::OnMouseEnter(UNUSED XEvent *ev) {
    D("Default fired")
  }

  void AWidget::OnMouseLeave(UNUSED XEvent *ev) {
    D("Default fired")
  }

  AUIHAlign AWidget::HAlign() const {
    return mHAlign;
  }

  void AWidget::SetHAlign(AUIHAlign hAlign) {
    mHAlign = hAlign;
  }

  AUIVAlign AWidget::VAlign() const {
    return mVAlign;
  }

  void AWidget::SetVAlign(AUIVAlign vAlign) {
    mVAlign = vAlign;
  }

  void AWidget::PrintDimensions() {
    D("x={},y={},szX={},szY={}", mX, mY, mSzX, mSzY)
  }

  Pixmap AWidget::BB() {
    return mBackBuffer;
  }

  void AWidget::SetBB(Pixmap backBuffer) {
    mBackBuffer = backBuffer;
  }

  void AWidget::UpdateBuffer() {
      AUI *aui = AUIPtr();
      if(!aui || Wnd() == 0) return;
      // Safety guard to prevent BadValue crashes inside XCreatePixmap during zero-size allocations
      if(SizeX() <= 0 || SizeY() <= 0) {
        D1("UpdateBuffer skipped due to zero/negative dimensions: {}x{}", SizeX(), SizeY());
        return;
      }
      Display *d = aui->Disp();
      // Query actual physical attributes of the current window context
      XWindowAttributes watt;
      XGetWindowAttributes(d, Wnd(), &watt);
      D2("UpdateBuffer for '{}': WindowDepth={}, VisualID=0x{:x}",
         mTitle, (int)watt.depth, (UINT64)watt.visual->visualid);
      // Thread safety guard: lock the X display before pipeline resource transformations
      XLockDisplay(d);
      // Free the XRender Picture upfront to eliminate memory leaks during dynamic style mutations
      if(mRenderPicture != None) {
        XRenderFreePicture(d, mRenderPicture);
        mRenderPicture = None;
      }
      // Release the old back-buffer allocation from server space safely
      if(mBackBuffer) {
        XFreePixmap(d, mBackBuffer);
        mBackBuffer = None;
      }
      // Allocate new server-side Pixmap matching the exact pixel bit depth of the target window
      mBackBuffer = XCreatePixmap(d, Wnd(), SafeUINT32(SizeX()),
          SafeUINT32(SizeY()), (unsigned int)watt.depth);
      D3("Created Pixmap ID: {}", (UINT64)mBackBuffer);
      // Re-initialize XRender context mapping if the current layout requires Simple3D visual states
      if(mStyle == AUIWidgetStyle::Simple3D) {
        XRenderPictFormat *fmt = XRenderFindVisualFormat(d, watt.visual);
        if(fmt) {
          mRenderPicture = XRenderCreatePicture(d, mBackBuffer, fmt, 0, nullptr);
          D3("XRender Picture created: ID={}", (UINT64)mRenderPicture);
        } else {
          XUnlockDisplay(d);
          E("XRender failed to find format for VisualID 0x{:x}", (UINT64)watt.visual->visualid);
        }
      }
      XUnlockDisplay(d);
  }

  void AWidget::Resize(UINT32 szx, UINT32 szy) {
    if(szx == mSzX && szy == mSzY) {
      D3()
      return;
    }
    if(mResizeEnabled) {
      mSzX = szx;
      mSzY = szy;
      Display *d = mAUI->Disp();
      XResizeWindow(d, mWindow, szx, szy);
      // --- NEW: Sync hints with WM so it re-aligns the X button ---
      XSizeHints *hints = XAllocSizeHints();
      if(hints) {
        hints->flags = PMinSize | PBaseSize;
        hints->min_width = 100; // Or a reasonable minimum
        hints->min_height = 100;
        hints->base_width = SafeINT32(szx);
        hints->base_height = SafeINT32(szy);
        XSetWMNormalHints(d, mWindow, hints);
        XFree(hints);
      }
      // Force the server to process the resize and hint update
      // before we try to Draw()
      XSync(d, False);
      UpdateBuffer();
    } else {
      DS()
      D("==========resize is disabled for widget, call EnableResize() if needed")
    }
    Draw();
  }

  void AWidget::ResizeX(UINT32 szx) {
    AWidget::Resize(szx, SafeUINT32(mSzY));
    D3("v")
  }

  void AWidget::ResizeY(UINT32 szy) {
    AWidget::Resize(SafeUINT32(mSzX), szy);
    D3("v")
  }

  void AWidget::SetSize(UINT64 x, UINT64 y) {
    mSzX = x;
    mSzY = y;
  }

  void AWidget::CorrectNegativeCoordinates(XEvent &event) {
    if(event.xbutton.x < 0) {
      D1("negative (x={}) coordinate corrected", event.xbutton.x)
      event.xbutton.x = 0;
    }
    if(event.xbutton.y < 0) {
      D1("negative (y={}) coordinate corrected", event.xbutton.y)
      event.xbutton.y = 0;
    }
  }
  // Xlib sends negative and too large positive coordinates...
  void AWidget::CorrectCoordinates(XEvent &event) {
    // Check negatives
    if(event.xbutton.x < 0) {
      D2("negative (x={}) coordinate corrected", event.xbutton.x)
      event.xbutton.x = 0;
    }
    if(event.xbutton.y < 0) {
      D2("negative (y={}) coordinate corrected", event.xbutton.y)
      event.xbutton.y = 0;
    }
    // Check overflys
    if((UINT64) event.xbutton.x > SizeX()) {
      D2("overflew (x={}) coordinate corrected", event.xbutton.x)
      event.xbutton.x = (INT32) SizeX();
    }
    if((UINT64) event.xbutton.y > SizeY()) {
      D2("overflew (y={}) coordinate corrected", event.xbutton.y)
      event.xbutton.y = (INT32) SizeY();
    }
  }

  void AWidget::CorrectCoordinateX(INT32 &x) {
    if(x < 0) {
      D1("negative (x={}) coordinate corrected", x)
      x = 0;
    }
    if(x > SafeINT32(SizeX())) {
      D1("overflew (x={}) coordinate corrected", x)
      x = SafeINT32(SizeX());
    }
  }

  void AWidget::CorrectCoordinateY(INT32 &y) {
    if(y < 0) {
      D1("negative (x={}) coordinate corrected", y)
      y = 0;
    }
    if(y > SafeINT32(SizeY())) {
      D1("overflew (y={}) coordinate corrected", y)
      y = SafeINT32(SizeY());
    }
  }

  void AWidget::Close() {
    mAUI->RemoveWidget(mWindow);
  }

  void AWidget::SetStyle(AUIWidgetStyle style) {
    if(mStyle == style)
      return;
    mStyle = style;
    Display *d = AUIPtr()->Disp();
    if(mStyle == AUIWidgetStyle::Flat && mRenderPicture != None) {
      XRenderFreePicture(d, mRenderPicture);
      mRenderPicture = None; // Reset to None to pass the integrity test
      D("XRender Picture freed for widget '{}' (Style: Flat)", Title().c_str());
    }
    UpdateBuffer();
    Draw();
  }

  void AWidget::SetPressDepth(int depth) {
    mDepth = (depth < 0) ? 0 : (depth > 15) ? 15 : depth;
    Draw();
  }

  AUIWidgetStyle AWidget::Style() const {
    return mStyle;
  }

  Picture AWidget::GetRenderPicture() const {
    return mRenderPicture;
  }

  /**
   * Maps the window to the X server, making it visible.
   * Triggers a redraw to ensure content is displayed correctly.
   */
  void AWidget::Show() {
    if (mAUI && mWindow) {
      // Make window visible on the screen
      XMapWindow(mAUI->Disp(), mWindow);
      // Explicitly redraw to populate the window from the back buffer
      Draw();
    }
  }

  /**
   * Unmaps the window from the X server, making it invisible.
   * The window still exists in memory but stops receiving input events.
   */
  void AWidget::Hide() {
    if (mAUI && mWindow) {
      // Remove window from the screen
      XUnmapWindow(mAUI->Disp(), mWindow);
      // Flush the output buffer to apply changes immediately
      XFlush(mAUI->Disp());
    }
  }

  void AWidget::TriggerFeedback() {
    E("called from base class")
  }


  INT32 AWidget::PressDepth() {
    return mDepth;
  }

  bool AWidget::IsParentOf(Window target) const {
    if(!mAUI)
      return false;
    // 1. Check if the target is actually a registered widget
    // Use mWidg.contains directly to avoid the E() exit in GetWidget
    if(!mAUI->HasWidget(target)) {
      return false;
    }

    AWidget *targetWidget = mAUI->GetWidget(target);
    if(!targetWidget)
      return false;

    // 2. Walk up the tree
    AWidget *current = targetWidget->ParentWidget();
    while (current != nullptr) {
      if(current->Wnd() == mWindow)
        return true;
      current = current->ParentWidget();
    }
    return false;
  }

  bool AWidget::ContainsGlobalCoordinates(int rootX, int rootY) {
    // We need to translate our local widget (0,0) position to absolute screen space
    Display *d = mAUI->Disp();
    Window rootWinReturn;
    int absoluteX = 0;
    int absoluteY = 0;
    // Fast hardware translation from our local coordinates to the root screen space
    XTranslateCoordinates(d, mWindow, XRootWindow(d, 0), 0, 0, &absoluteX,
        &absoluteY, &rootWinReturn);
    // Verify if the screen click hits inside our physical bounding box dimensions
    return (rootX >= absoluteX && rootX < (absoluteX + static_cast<int>(mSzX))
        && rootY >= absoluteY && rootY < (absoluteY + static_cast<int>(mSzY)));
  }

  void AWidget::ResizeNoRedraw(UINT32 szx, UINT32 szy) {
    D3("Quiet resize requested without triggering Draw. Width: %u, Height: %u", szx, szy);

    // 1. Изменяем внутренние метрики геометрии
    mSzX = SafeUINT64(szx);
    mSzY = SafeUINT64(szy);

    // 2. Отправляем изменения размеров на X-сервер напрямую
    Display* d = mAUI->Disp();
    if (d && mWindow != 0) {
      XResizeWindow(d, mWindow, SafeUINT32(mSzX), SafeUINT32(mSzY));
      XFlush(d);
    }
  }

  void AWidget::Enable() {
    mEnabled = true;
    Draw();
  }

  void AWidget::Disable() {
    mEnabled = false;
    Draw();
  }

  AWidget::~AWidget() {
    D3("widget '{}' destructor active", mTitle.c_str());
    // 1. CRITICAL: Unregister from the global AUI map FIRST.
    // Use the actual mWindow ID before we potentially lose it.
    if(mAUI && mWindow != 0) {
      mAUI->UnregisterWindow(mWindow);
    }
    // 2. Recursively delete children (Bottom-Up C++ destruction)
    DestroyChildWidgets();
    Display *d = mAUI->Disp();
    if(mRenderPicture != None) {
      XRenderFreePicture(d, mRenderPicture);
      mRenderPicture = None;
    }
    // 3. Resource Cleanup
    if(mFont != 0) {
      D2("freeing font {}", (UINT64)mFont);
      XFreeFont(d, mFont);
      mFont = 0;
    } else {
      D2("widget '{}' has no font to free", mTitle.c_str());
    }
    if(mGC != 0) {
      D2("freeing GC {}", (UINT64)mGC);
      XFreeGC(d, mGC);
      mGC = 0;
    } else {
      D2("widget '{}' has no GC to free", mTitle.c_str());
    }
    if(mBackBuffer) {
      D2("freeing backbuffer");
      XFreePixmap(d, mBackBuffer);
      mBackBuffer = 0;
    } else {
      D2("widget '{}' has no backbuffer to free", mTitle.c_str());
    }
    // 4. X11 Window Destruction
    // We only call XDestroyWindow if this object is the one initiating the kill.
    // If mWindow is already 0, it means a parent handled the X server side.
    if(mWindow != 0) {
      D2("destroying window {}", (UINT64)mWindow);
      XDestroyWindow(d, mWindow);
      mWindow = 0;
    }
    D2("<widget '{}' destructor ends", mTitle.c_str());
  }

  void AWidget::MoveResize(UINT32 x, UINT32 y, UINT32 szx, UINT32 szy) {
    Move(x, y);
    Resize(szx, szy);
  }

  void AWidget::MoveResizeText(UINT32 x, UINT32 y, UINT32 szx, UINT32 szy,
      std::string txt) {
    Move(x, y);
    Resize(szx, szy);
    SetText(txt);
  }

}

