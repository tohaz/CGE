#include "APopupMenu.h"
#include "AUILib.h"

namespace aui {

  AMenuItem createEmptyItem() {
    AMenuItem item;
    item.text = "";
    item.hotkey = "";
    item.action = nullptr;
    item.subItems = { };// Empty vector
    item.isSeparator = false;
    item.isCheckable = false;
    item.isChecked = false;
    item.isEnabled = true;
    item.icon = None;// Picture is a typedef for XID (usually unsigned long)
    return item;
  }

  APopupMenu::APopupMenu(AWidget *wParent, const std::vector<AMenuItem> &inItems) :
      mItems(inItems) {
//    AUI *aui = wParent->AUIPtr();
//    SetAUIPtr(aui);
//    SetWndParent(wParent);
//    SetType(AUIWidgetType::defaultPopupMenu);
//    mStyle = AUIWidgetStyle::Simple3D;
//    SetBGColor(AUI_DEFAULT_BUTTON_BG);
//    Display *d = aui->Disp();
//    INT32 scr = aui->Scr();
//    XSetWindowAttributes swa;
//    swa.override_redirect = True;// Prevent WM from adding decorations/titlebar
//    swa.save_under = True;// Ask X server to preserve pixels beneath
//    swa.background_pixel = BGColor();
//    swa.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask |
//    PointerMotionMask | EnterWindowMask | LeaveWindowMask;
//// 3. Create window with a temporary 1x1 size to get a valid handle
//    Window win = XCreateWindow(d, RootWindow(d, scr), 0, 0, 1, 1, 0,
//    CopyFromParent, InputOutput, CopyFromParent,
//    CWOverrideRedirect | CWSaveUnder | CWBackPixel | CWEventMask, &swa);
//// 4. Initialize basic widget properties (This loads the font)
//    InitWidgetProps(win);
//// 5. Calculate the real layout now that the font is available
//    UINT32 w = 0, h = 0;
//    CalculateLayout(w, h);
//// 6. Synchronize both internal state and the X11 Window dimensions
//    SetSizeXY(w, h);
//    XResizeWindow(d, Wnd(), w, h);
//// Finalize window background and register in AUI map
//    XSetWindowBackground(d, Wnd(), BGColor());
//    aui->AddWidget(this);
//// Initialize BackBuffer and XRender Picture based on the final size
//    UpdateBuffer();
  }

  APopupMenu* APopupMenu::AttachTo(AWidget *wParent, const std::vector<AMenuItem> &inItems) {
    return new APopupMenu(wParent, inItems);
  }

  void APopupMenu::Popup(INT32 rootX, INT32 rootY) {
    Display *d = AUIPtr()->Disp();
// 1. Position the menu at absolute screen coordinates
    Move(SafeUINT32(rootX), SafeUINT32(rootY));
// 2. Map the window and trigger the initial Draw()
    Show();
// 3. Force X server to process the mapping before we attempt the grab
    XSync(d, False);
// 4. Perform the Pointer Grab
// owner_events = True is critical here: it allows events to be
// reported to their natural windows (like submenus) while
// keeping the mouse "locked" to our application.
    int grabResult = XGrabPointer(d, Wnd(), True,
    ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
    GrabModeAsync, GrabModeAsync,
    None, None, CurrentTime);
    if(grabResult == GrabSuccess) {
      mIsGrabbed = true;
      D2("Menu grab successful for window {}", (UINT64)Wnd());
    } else {
      E("Menu failed to grab pointer (code: {})", grabResult);
    }
// Final flush to ensure visibility
    XFlush(d);
  }

  void APopupMenu::Dismiss() {
    CloseSubMenu();
    if(mIsGrabbed) {
      XUngrabPointer(AUIPtr()->Disp(), CurrentTime);
      mIsGrabbed = false;
    }
    APopupMenu *parent = mParentMenu;
    Hide();
    if(parent) {
      parent->Dismiss();
    }
  }

  void APopupMenu::CalculateLayout(UINT32 &outW, UINT32 &outH) {
    XFontStruct *f = Font();
    INT32 maxTextW = 0;
    INT64 totalH = (static_cast<INT64>(mDepth) * 2);
    for (size_t i = 0; i < mItems.size(); ++i) {
      if(mItems[i].isSeparator) {
// Add separator specific height
        totalH += mSeparatorHeight;
      } else {
// Add regular item height
        totalH += mItemHeight;
        INT32 w = XTextWidth(f, mItems[i].text.c_str(), SafeINT32(mItems[i].text.length()));
        if(w > maxTextW)
          maxTextW = w;
      }
    }
    INT32 finalW = maxTextW + mLeftMargin + mRightMargin + (mDepth * 2);
    if(finalW < mMinWidth)
      finalW = mMinWidth;
    outW = SafeUINT32(finalW);
    outH = SafeUINT32(totalH);
  }

  void APopupMenu::Draw() {
//    AUI *aui = AUIPtr();
//// Basic integrity checks
//    if(!aui || !BB()) {
//      E("Menu Draw aborted: AUI or BackBuffer is NULL");
//      return;
//    }
//    Display *d = aui->Disp();
//    Pixmap bb = BB();
//    GC gc = GCPtr();
//    XFontStruct *f = Font();
//    UINT32 szx = SafeUINT32(SizeX());
//    UINT32 szy = SafeUINT32(SizeY());
//// 1. Clear the entire backbuffer with the background color
//    XSetForeground(d, gc, BGColor());
//    XFillRectangle(d, bb, gc, 0, 0, szx, szy);
//// 2. Draw the 3D Baguette Frame (Outer Border)
//    if(mStyle == AUIWidgetStyle::Simple3D && GetRenderPicture() != None) {
//// Use member variables for lighting intensity
//      XRenderColor light = ScaleAndBlend(255, 255, mBaguetteLightFactor);
//      XRenderColor dark = ScaleAndBlend(0, 0, mBaguetteDarkFactor);
//// Top and Left light edges
//      XRenderFillRectangle(d, PictOpOver, GetRenderPicture(), &light, 0, 0, szx, SafeUINT32(mDepth));
//      XRenderFillRectangle(d, PictOpOver, GetRenderPicture(), &light, 0, 0, SafeUINT32(mDepth), szy);
//// Bottom and Right dark edges
//      XRenderFillRectangle(d, PictOpOver, GetRenderPicture(), &dark, 0, SafeINT32(szy) - mDepth, szx,
//          SafeUINT32(mDepth));
//      XRenderFillRectangle(d, PictOpOver, GetRenderPicture(), &dark, SafeINT32(szx) - mDepth, 0, SafeUINT32(mDepth),
//          szy);
//    }
//    INT32 currentY = mDepth;
//    for (size_t i = 0; i < mItems.size(); ++i) {
//      const AMenuItem &item = mItems[i];
//      INT32 thisItemHeight = item.isSeparator ? mSeparatorHeight : mItemHeight;
//// Highlight logic
//      if(static_cast<INT32>(i) == mHoveredIndex && item.isEnabled && !item.isSeparator) {
//        XSetForeground(d, gc, GetBlendedColor(BGColor(), 255, mHoverIntensity));
//        XFillRectangle(d, bb, gc, mDepth, currentY, szx - SafeUINT32(mDepth * 2), SafeUINT32(thisItemHeight));
//      }
//      if(item.isSeparator) {
//// Draw line in the middle of mSeparatorHeight
//        INT32 midY = currentY + (mSeparatorHeight / 2);
//        XSetForeground(d, gc, mSeparatorShadow);
//        XDrawLine(d, bb, gc, mDepth + mSeparatorPadding, midY, SafeINT32(szx) - mDepth - mSeparatorPadding, midY);
//// Draw highlight line only if there's enough height
//        if(mSeparatorHeight > 1) {
//          XSetForeground(d, gc, mSeparatorHighlight);
//          XDrawLine(d, bb, gc, mDepth + mSeparatorPadding, midY + 1, SafeINT32(szx) - mDepth - mSeparatorPadding,
//              midY + 1);
//        }
//      } else {
//// Draw Text
//        XSetForeground(d, gc, item.isEnabled ? BlackPixel(d, aui->Scr()) : mDisabledColor);
//        INT32 textY = currentY + (mItemHeight + f->ascent - f->descent) / 2;
//        XDrawString(d, bb, gc, mDepth + mLeftMargin, textY, item.text.c_str(), SafeINT32(item.text.length()));
//        if(!item.subItems.empty()) {
//          XDrawString(d, bb, gc, SafeINT32(szx) - mDepth - mRightMargin, textY, ">", 1);
//        }
//      }
//      currentY += thisItemHeight;// Increment Y by actual height of this element
//    }
//    XCopyArea(d, bb, Wnd(), gc, 0, 0, szx, szy, 0, 0);
//    XFlush(d);
  }

  void APopupMenu::OnMouseMove(XEvent *ev) {
    INT32 mouseY = ev->xmotion.y;
    INT32 currentY = mDepth;
    INT32 foundIndex = -1;
    INT32 itemTopY = mDepth;// Track the start of the hovered item for submenu placement
// 1. Find which item is under the mouse by summing heights
    for (size_t i = 0; i < mItems.size(); ++i) {
      INT32 h = mItems[i].isSeparator ? mSeparatorHeight : mItemHeight;
      if(mouseY >= currentY && mouseY < currentY + h) {
        foundIndex = static_cast<INT32>(i);
        itemTopY = currentY;
        break;
      }
      currentY += h;
    }
// 2. If we found a valid item and it's a new hover state
    if(foundIndex != -1 && foundIndex != mHoveredIndex) {
      mHoveredIndex = foundIndex;
      const AMenuItem &item = mItems[static_cast<size_t>(mHoveredIndex)];
// 3. Submenu logic
      if(item.subItems.empty()) {
        CloseSubMenu();
      } else if(item.isEnabled) {
        CloseSubMenu();
        INT32 rootX = 0, rootY = 0;
        Window child;
        Display *d = AUIPtr()->Disp();
// Calculate screen coordinates.
// We use itemTopY instead of (index * mItemHeight)
        XTranslateCoordinates(d, Wnd(), RootWindow(d, AUIPtr()->Scr()), SafeINT32(SizeX()) - mDepth, itemTopY, &rootX,
            &rootY, &child);
        mActiveSubMenu = APopupMenu::AttachTo(this, item.subItems);
        mActiveSubMenu->SetParentMenu(this);
        mActiveSubMenu->SetStyle(this->Style());
        mActiveSubMenu->Popup(rootX, rootY);
      }
      Draw();
    }
  }

  void APopupMenu::OnButtonRelease(XEvent *ev) {
// 1. If released inside our window
    if(ev->xbutton.window == Wnd()) {
// Check if we are over a valid item
      if(mHoveredIndex >= 0 && mHoveredIndex < static_cast<INT32>(mItems.size())) {
        const AMenuItem &item = mItems[static_cast<size_t>(mHoveredIndex)];
// CRITICAL FIX: To prevent "instant close" when releasing RMB after opening:
// We check if the mouse has actually moved over an item since the menu appeared.
// If mHoveredIndex is valid but it's just the very first release,
// we only trigger if it's Button1 (Left Click).
// If it's Button3 (Right Click), we ignore the release to let the menu stay.
        if(ev->xbutton.button == Button3) {
          D3("Ignoring Button3 release to keep menu persistent");
          return;
        }
        if(item.isEnabled && !item.isSeparator && item.subItems.empty()) {
          std::function<void()> actionToRun = item.action;
          D2("Item '{}' selected, dismissing chain", item.text);
          Dismiss();
          if(actionToRun)
            actionToRun();
        }
      }
    }
// 2. Forward to submenu
    else if(mActiveSubMenu && ev->xbutton.window == mActiveSubMenu->Wnd()) {
      mActiveSubMenu->OnButtonRelease(ev);
    }
  }

  void APopupMenu::CloseSubMenu() {
    if(mActiveSubMenu) {
// 1. Manually unregister it from THIS widget's child map
// so ~AWidget() -> DestroyChildWidgets() won't find it.
      this->UnregisterChild(mActiveSubMenu);
// 2. Now it is safe to delete
      delete mActiveSubMenu;
      mActiveSubMenu = nullptr;
    }
  }

  void APopupMenu::OnMouseEnter(UNUSED XEvent *ev) {
// Logic for hover
    D()
  }

  void APopupMenu::OnMouseLeave(UNUSED XEvent *ev) {
// Logic for hover
    D()
  }

  void APopupMenu::OnButtonPress(XEvent *ev) {
    if(ev->xbutton.window != Wnd()) {
      if(mActiveSubMenu && ev->xbutton.window == mActiveSubMenu->Wnd())
        return;
      D2("Click outside detected, dismissing");
      Dismiss();
    }
  }

  APopupMenu::~APopupMenu() {
    CloseSubMenu();
  }

}// namespace aui
