#include "AWidget.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "AUILib.h"

namespace aui {

  AWidget::AWidget() {
    D3("sizeof widget %lu", sizeof(AWidget))
//    OnButtonPressCB = [this](XEvent* ev, CGWidget* w, void *data) {
//      D1("default OnButtonPressCB callback fired")
//    };
  }

  void AWidget::InitWidgetProps(Window w) {
    D3("set window %lu", (UINT64) w)
    Display *d = mAUI->Disp();
    if(mWindow == 0) mWindow = w;
    else E("attempt to reset window")
    if(mGC == 0) {
      mGC = XCreateGC(d, mWindow, 0, NULL);
    }
    else E("GC is already set")
    if(mFont == 0) {
      mFont = XLoadQueryFont(d, AUI_DEFAULT_FONT);
      if (!mFont) {
        D("Cannot open font %s, using 'fixed' instead", AUI_DEFAULT_FONT)
        mFont = XLoadQueryFont(d, "fixed");

        if (!mFont) E("Cannot open 'fixed' font either, exit.")
      }
      else D3("opened font with description:%s, id %llu", AUI_DEFAULT_FONT, (UINT64)mFont)
      XSetFont(d, mGC, mFont->fid);
    }
    else E("Font is already set")
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
    D3("setting title to %s", newTitle.c_str())
    mTitle = newTitle;
  }

  void AWidget::SetText(std::string newText) {
    mText = newText;
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

  CHAR8* AWidget::TextPtr() {
    return (CHAR8*)mText.c_str();
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
    D3("wndparent get %lu", (UINT64) mWindowParent)
    return mWindowParent;
  }

  void AWidget::SetWndParent(AWidget* newParent) {
    mWindowParent = newParent;
    D3("wmdparent set %lu", (UINT64) mWindowParent)
  }

  void AWidget::AddWidgetChild(AWidget *newChild) {
    mWidg[newChild->Wnd()] = newChild;
  }

  void AWidget::DestroyChildWidgets() {
    AWidget* w = 0;
    if(mWidg.size() > 0) {
      D2(">widget '%s' have children", mTitle.c_str())
      D2("scrolling widget map for child widgets");
      for (auto const& [key, val] : mWidg) {
        w = (AWidget*) val;
        D2("=deleting child %lu", (UINT64) key)
        mAUI->UnregisterWindow(key);
        delete w;
      }
    }
    else {
      D2(">widget '%s' dont have children", mTitle.c_str())
    }
  }

  void AWidget::DisableResize() {
    D3();
    mResizeEnabled = false;
    XSizeHints *hints = XAllocSizeHints();
    hints->min_width = mSzX;
    hints->max_width = mSzX;
    hints->min_height = mSzY;
    hints->max_height = mSzY;
    hints->flags |= PMinSize + PMaxSize;
    XSetWMNormalHints(mAUI->Disp(), mWindow, hints);
    XFree(hints);
  }

  void AWidget::EnableResize() {
    D3();
    mResizeEnabled = true;
    XSizeHints *hints = XAllocSizeHints();
    hints->min_width = mSzX;
    hints->max_width = mSzX;
    hints->min_height = mSzY;
    hints->max_height = mSzY;
    hints->flags &= ~(PMinSize + PMaxSize);
    XSetWMNormalHints(mAUI->Disp(), mWindow, hints);
    XFree(hints);
  }

  void AWidget::Resize(UINT32 szx, UINT32 szy) {
    if(mResizeEnabled) {
      mSzX = szx;
      mSzY = szy;
      XResizeWindow(mAUI->Disp(), mWindow, szx, szy);
    }
    else {
      DS()
      D("resize is disabled for widget, call EnableResize() if needed")
    }
  }

  void AWidget::Move(unsigned int x, unsigned int y) {
    mX = x;
    mY = y;
    XMoveWindow(mAUI->Disp(), mWindow, x, y);
    XSync(mAUI->Disp(), False);
  }

  void AWidget::UnregisterChild(AWidget *w) {
    Window wnd = w->Wnd();
    if(mWidg.contains(wnd)) {
      mWidg.erase(wnd);
    }
    else {
      E("attempt to remove child that is not registered")
    }
  }

  void AWidget::OnKeyPress([[maybe_unused]]XEvent *ev) {
    E("default key press handler should not be used")
  }

  void AWidget::SetBorderSz(unsigned int borderSz) {
    if (!mAUI || mWindow == 0) {
        mBorderSz = borderSz; // Сохраняем для инициализации
        return;
    }

    // Если в классе уже записано это значение — выходим.
    // Это предотвратит лишние запросы к X-серверу при вызове из конструктора.
    if (mBorderSz == borderSz) return;

    mBorderSz = borderSz;
    Display* d = mAUI->Disp();

    XSetWindowBorderWidth(d, mWindow, mBorderSz);
    XSync(d, False);
  }

  UINT32 AWidget::BorderSz() {
    return mBorderSz;
  }

  void AWidget::SetOnButtonPressCB(std::function<void(XEvent *ev, AWidget *w, void *arbdata)> func, void *data) {
    OnButtonPressCB = func;
    mUserDataButtonPress = data;
  }

  void AWidget::SetOnButtonReleaseCB(std::function<void(XEvent *ev, AWidget *w, void *arbdata)> func, void *data) {
    OnButtonReleaseCB = func;
    mUserDataButtonRelease = data;
   }

  void AWidget::SetOnSubmitCB(std::function<void(AWidget *w, void *arbdata)> func,
    void *data) {
    OnSubmitCB = func;
    mUserDataSubmit = data;
  }

  void AWidget::OnButtonPress(XEvent *ev) {
    D2()
    if(OnButtonPressCB == 0) {
      D2("CB is not set")
      return;
    }
    else {
      D2("CB is set")
      OnButtonPressCB(ev, this, mUserDataButtonPress);
    }
  }

  void AWidget::OnButtonRelease(XEvent *ev) {
    if(OnButtonReleaseCB == 0) {
      D3("CB is not set")
      return;
    }
    else {
      D3("CB is set")
      OnButtonReleaseCB(ev, this, mUserDataButtonRelease);
    }
  }

  void AWidget::OnMouseMove(XEvent *ev) {
    if(OnMouseMoveCB == 0) {
      D3("CB is not set")
      return;
    }
    else {
      D3("CB is set")
      OnMouseMoveCB(ev, this, mUserDataMouseMove);
    }

  }

  void AWidget::OnSubmit() {
    if(OnSubmitCB == 0) {
      D3("CB is not set")
      return;
    }
    else {
      D3("CB is set")
      OnSubmitCB(this, mUserDataSubmit);
    }
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


  AWidget::~AWidget() {
    D3("widget '%s' destructor active", mTitle.c_str())
    Display* d = mAUI->Disp();
    if(mFont != 0) {
     D2("freeing font %lu", (UINT64)mFont)
     XFreeFont(d, mFont);
     mFont = 0;
    }
    else {
      D2("not freeing font")
    }
    if(mGC != 0) {
      D2("freeing GC %lu", (UINT64)mGC)
      XFreeGC(d, mGC);
      mGC = 0;
    }
    else {
      D2("not freeing GC")
    }
    if(mWindow != 0) {
       D2("destroying window")
       XDestroyWindow(d, mWindow);
       mWindow = 0;
    }
    else {
      D2("not destroying window")
    }
    D2("<widget '%s' destructor ends", mTitle.c_str())
  }

}

