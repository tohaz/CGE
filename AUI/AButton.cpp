#include "AButton.h"
//
#include "AUILib.h"

namespace aui {
  AButton::AButton(std::string inText, AWidget *wParent) {
    AUI *aui = wParent->AUIPtr();
    SetAUIPtr(aui);
    SetWndParent(wParent);
    Display *d = aui->Disp();
    INT32 scr = aui->Scr();
    SetType(AUIWidgetType::defaultButton);
    SetXY(AUI_DEFAULT_BUTTON_X, AUI_DEFAULT_BUTTON_Y);
    SetSizeXY(AUI_DEFAULT_BUTTON_SZX, AUI_DEFAULT_BUTTON_SZY);
    SetBGColor(AUI_DEFAULT_BUTTON_BG);
    XSetWindowAttributes swa;
    swa.background_pixmap = None;
    swa.border_pixel = BlackPixel(d, scr);
    swa.bit_gravity = StaticGravity;
    swa.save_under = True;
    swa.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask;
    Window w = XCreateWindow(d, wParent->Wnd(), SafeINT32(X()), SafeINT32(Y()),
        SafeUINT32(SizeX()), SafeUINT32(SizeY()),
        AUI_DEFAULT_BUTTON_BORDERW,
        CopyFromParent, InputOutput, CopyFromParent,
        CWBackPixmap | CWBorderPixel | CWEventMask | CWBitGravity | CWSaveUnder,
        &swa);
    InitWidgetProps(w);
    SetText(inText);
    aui->AddWidget(this);
    SetBorderSz(AUI_DEFAULT_BUTTON_BORDERW);
    UpdateBuffer();
    XMapWindow(d, Wnd());
    XSync(d, False);
  }

  void AButton::Draw() {
    AUI *cg = AUIPtr();
    if(!cg || Wnd() == 0 || GCPtr() == 0)
      return;
    if(!BB())
      UpdateBuffer();
    Display *d = cg->Disp();
    Pixmap bb = BB();
    GC gc = GCPtr();
    XFontStruct *f = Font();
    UINT32 szx = SafeUINT32(SizeX());
    UINT32 szy = SafeUINT32(SizeY());
    INT32 totalW = XTextWidth(f, Text().c_str(), (int) Text().size());
    INT32 drawX = 0, drawY = 0;
    if(HAlign() == AUIHAlign::center)
      drawX = (SafeINT32(szx) - totalW) / 2;
    else if(HAlign() == AUIHAlign::left)
      drawX = 5;
    else
      drawX = SafeINT32(szx) - totalW - 5;
    if(VAlign() == AUIVAlign::center)
      drawY = (SafeINT32(szy) + f->ascent - f->descent) / 2;
    else if(VAlign() == AUIVAlign::top)
      drawY = f->ascent + 5;
    else
      drawY = SafeINT32(szy) - f->descent - 5;
    if(mStyle == AUIWidgetStyle::Simple3D && mRenderPicture != None) {
      bool pressed = IsHL();
      int offset = pressed ? mDepth : 0;
      unsigned int shrink = pressed ? static_cast<unsigned int>(mDepth * 2) : 0;
      XSetForeground(d, gc, BGColor());
      XFillRectangle(d, bb, gc, 0, 0, szx, szy);
      if(!pressed) {
        XRenderColor shadow = { 0, 0, 0, 0x6666 };
        for (int i = 1; i <= mDepth; ++i)
          XRenderFillRectangle(d, PictOpOver, mRenderPicture, &shadow, i, i,
              szx, szy);
      } else {
        XRenderColor hole = { 0, 0, 0, 0x8888 };
        XRenderFillRectangle(d, PictOpOver, mRenderPicture, &hole, 0, 0, szx,
            szy);
      }
      XLinearGradient gradient;
      gradient.p1 = { 0, 0 };
      gradient.p2 = { 0, XDoubleToFixed(static_cast<double>(szy)) };
      XRenderColor c_top = pressed ? XRenderColor { 0xAAAA, 0xAAAA, 0xAAAA, 0xFFFF } :
                                     XRenderColor { 0xEEEE, 0xEEEE, 0xEEEE, 0xFFFF };
      XRenderColor c_bot = pressed ? XRenderColor { 0x5555, 0x5555, 0x5555, 0xFFFF } :
                                     XRenderColor { 0x8888, 0x8888, 0x8888, 0xFFFF };
      XRenderColor colors[] = { c_top, c_bot };
      XFixed stops[] = { XDoubleToFixed(0.0), XDoubleToFixed(1.0) };
      Picture grad = XRenderCreateLinearGradient(d, &gradient, stops, colors,
          2);
      XRenderComposite(d, PictOpSrc, grad, None, mRenderPicture, 0, 0, 0, 0,
          offset, offset, (szx > shrink ? szx - shrink : 1),
          (szy > shrink ? szy - shrink : 1));
      XRenderFreePicture(d, grad);
      XSetForeground(d, gc, 0x333333);
      XDrawString(d, bb, gc, drawX + offset, drawY + offset, TextPtr(),
          (int) Text().length());
    } else {
      // Flat ---
      XSetForeground(d, gc, IsHL() ? HLColor(BGColor()) : BGColor());
      XFillRectangle(d, bb, gc, 0, 0, szx, szy);
      XSetForeground(d, gc, BlackPixel(d, cg->Scr()));
      XDrawRectangle(d, bb, gc, 0, 0, szx - 1, szy - 1);
      XDrawString(d, bb, gc, drawX, drawY, TextPtr(), (int) Text().length());
    }
    XCopyArea(d, bb, Wnd(), gc, 0, 0, szx, szy, 0, 0);
    XFlush(d);
  }

  void AButton::OnButtonPress([[maybe_unused]]XEvent *ev) {
    if(!IsHL()) {
      HL(true);
      Draw();
      XSync(AUIPtr()->Disp(), False);
    }
  }

  void AButton::OnButtonRelease(XEvent *ev) {
    AUI *cg = AUIPtr();
    if(!cg->IsWindowRegistered(ev->xexpose.window))
      return;
    INT32 root_x, root_y, win_x, win_y;
    Window root_window, child_window;
    UINT32 mask = 0;
    Display *d = cg->Disp();
    Window w = Wnd();
    root_window = cg->MainWnd()->Wnd();
    XQueryPointer(d, w, &root_window, &child_window, &root_x, &root_y, &win_x,
        &win_y, &mask);
    if((win_x < (INT64) SizeX()) && (win_y < (INT64) SizeY()) && (win_x >= 0)
        && (win_y >= 0)) {
      AWidget::OnButtonRelease(ev);
    }
    // Re-verify window existence after potential user-code execution
    if(!cg->IsWindowRegistered(ev->xexpose.window))
      return;
    if(IsHL()) {
      HL(false);
      Draw();
    }
  }

  AButton* AButton::AttachTo(AWidget *w, std::string inText) {
    return new AButton(inText, w);
  }

  AButton::~AButton() {
    D3("v");
  }

}
