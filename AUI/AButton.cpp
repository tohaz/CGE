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
    //swa.background_pixmap = None;
    swa.border_pixel = BlackPixel(d, scr);
    swa.bit_gravity = StaticGravity;
    swa.save_under = True;
    swa.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask ;
    swa.background_pixel = BGColor();
    Window w = XCreateWindow(d, wParent->Wnd(), SafeINT32(X()), SafeINT32(Y()),
        SafeUINT32(SizeX()), SafeUINT32(SizeY()),
        0,
        CopyFromParent, InputOutput, CopyFromParent,
        CWBackPixel | CWBorderPixel | CWEventMask | CWBitGravity | CWSaveUnder,
        &swa);
    InitWidgetProps(w);
    SetText(inText);
    aui->AddWidget(this);
    SetBorderSz(0);
    UpdateBuffer();
    Draw();
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
    UINT32 drawColor = BGColor();
    if (IsHL()) {
        // High priority: Button is physically pressed
        drawColor = HLColor(BGColor());
    } else if (mIsHovered) {
        // Medium priority: Mouse is just hovering over it
        drawColor = GetBlendedColor(BGColor(), 255, 0.50);
    }
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
      D("drawing 3d style {}", (UINT64)this)
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
      RGBAColor cl;
      cl.value = drawColor;
      XRenderColor c_top, c_bot;
      double intensity = 0.5;
      gradient.p1 = { 0, 0 };
      gradient.p2 = { 0, XDoubleToFixed(static_cast<double>(szy)) };
      if (pressed) {
          // Pressed state: move top towards black, bottom towards white
          c_top.red   = ScaleAndBlend(cl.rgba.r, 0, 0.2).red;
          c_top.green = ScaleAndBlend(cl.rgba.g, 0, 0.2).green;
          c_top.blue  = ScaleAndBlend(cl.rgba.b, 0, 0.2).blue;
          c_top.alpha = 0xFFFF;

          c_bot.red   = ScaleAndBlend(cl.rgba.r, 255, 0.1).red;
          c_bot.green = ScaleAndBlend(cl.rgba.g, 255, 0.1).green;
          c_bot.blue  = ScaleAndBlend(cl.rgba.b, 255, 0.1).blue;
          c_bot.alpha = 0xFFFF;
      } else {
          // Normal/Hover: Top towards white, Bottom towards black
          c_top.red   = ScaleAndBlend(cl.rgba.r, 255, intensity).red;
          c_top.green = ScaleAndBlend(cl.rgba.g, 255, intensity).green;
          c_top.blue  = ScaleAndBlend(cl.rgba.b, 255, intensity).blue;
          c_top.alpha = 0xFFFF;

          c_bot.red   = ScaleAndBlend(cl.rgba.r, 0, intensity).red;
          c_bot.green = ScaleAndBlend(cl.rgba.g, 0, intensity).green;
          c_bot.blue  = ScaleAndBlend(cl.rgba.b, 0, intensity).blue;
          c_bot.alpha = 0xFFFF;
      }
      XRenderColor colors[] = { c_top, c_bot };
      XFixed stops[] = { XDoubleToFixed(0.0), XDoubleToFixed(1.0) };
      Picture grad = XRenderCreateLinearGradient(d, &gradient, stops, colors,
          2);
      XRenderComposite(d, PictOpSrc, grad, None, mRenderPicture, 0, 0, 0, 0,
          offset, offset, (szx > shrink ? szx - shrink : 1),
          (szy > shrink ? szy - shrink : 1));
      XRenderFreePicture(d, grad);
      XSetForeground(d, gc, BlackPixel(d, cg->Scr()));
      XDrawRectangle(d, bb, gc, 0, 0, szx - 1, szy - 1);
      XSetForeground(d, gc, 0x333333);
      XDrawString(d, bb, gc, drawX + offset, drawY + offset, TextPtr(),
          (int) Text().length());
    } else {
      // Flat ---
      XSetForeground(d, gc, drawColor);
             XFillRectangle(d, bb, gc, 0, 0, szx, szy);
             // Border and text
             XSetForeground(d, gc, BlackPixel(d, cg->Scr()));
             XDrawRectangle(d, bb, gc, 0, 0, szx - 1, szy - 1);
             XDrawString(d, bb, gc, drawX, drawY, TextPtr(), (int) Text().length());
    }
    XCopyArea(d, bb, Wnd(), gc, 0, 0, szx, szy, 0, 0);
    XFlush(d);
    D("button style {} {}", (UINT64)this, (UINT64)Style())
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

  void AButton::OnMouseEnter(UNUSED XEvent* ev) {
      mIsHovered = true;
      Draw(); // Redraw immediately with the hover color
  }

  void AButton::OnMouseLeave(UNUSED XEvent* ev) {
      mIsHovered = false;
      Draw(); // Return to normal state
  }

  AButton::~AButton() {
    D3("v");
  }

}
