#include "AButton.h"
//
#include "AUILib.h"

namespace aui {
  AButton::AButton(UNUSED std::string inText, UNUSED AWidget *wParent) {
//    AUI *aui = wParent->AUIPtr();
//    SetAUIPtr(aui);
//    SetWndParent(wParent);
//    Display *d = aui->Disp();
//    INT32 scr = aui->Scr();
//    SetType(AUIWidgetType::defaultButton);
//    SetXY(AUI_DEFAULT_BUTTON_X, AUI_DEFAULT_BUTTON_Y);
//    SetSizeXY(AUI_DEFAULT_BUTTON_SZX, AUI_DEFAULT_BUTTON_SZY);
//    SetBGColor(AUI_DEFAULT_BUTTON_BG);
//    XSetWindowAttributes swa;
//    //swa.background_pixmap = None;
//    swa.border_pixel = BlackPixel(d, scr);
//    swa.bit_gravity = StaticGravity;
//    swa.save_under = True;
//    swa.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask ;
//    swa.background_pixel = BGColor();
//    Window w = XCreateWindow(d, wParent->Wnd(), SafeINT32(X()), SafeINT32(Y()),
//        SafeUINT32(SizeX()), SafeUINT32(SizeY()),
//        0,
//        CopyFromParent, InputOutput, CopyFromParent,
//        CWBackPixel | CWBorderPixel | CWEventMask | CWBitGravity | CWSaveUnder,
//        &swa);
//    InitWidgetProps(w);
//    SetText(inText);
//    aui->AddWidget(this);
//    SetBorderSz(0);
//    UpdateBuffer();
//    Draw();
//    XMapWindow(d, Wnd());
//    XSync(d, False);
  }

  void AButton::Draw() {
//    AUI *cg = AUIPtr();
//    if(!cg || Wnd() == 0 || GCPtr() == 0)
//      return;
//    if(!BB())
//      UpdateBuffer();
//    Display *d = cg->Disp();
//    Pixmap bb = BB();
//    GC gc = GCPtr();
//    XFontStruct *f = Font();
//    UINT32 szx = SafeUINT32(SizeX());
//    UINT32 szy = SafeUINT32(SizeY());
//    INT32 totalW = XTextWidth(f, Text().c_str(), (int) Text().size());
//    INT32 drawX = 0, drawY = 0;
//    UINT32 drawColor = BGColor();
//    if (!IsEnabled()) {
//        drawColor = 0x888888; // Block mutations and force uniform disabled layout grey
//    } else if (IsHL()) {
//        drawColor = HLColor(BGColor());
//    } else if (mIsHovered) {
//        drawColor = GetBlendedColor(BGColor(), 255, 0.50);
//    }
//    if (!Font()) {
//        D1("[AUI WARNING] Font() pointer is NULL!");
//    }
//    int direction, ascent, descent;
//    XCharStruct overall;
//    XTextExtents(f, TextPtr(), (int)Text().length(), &direction, &ascent, &descent, &overall);
//    INT32 calculatedCenter = (SafeINT32(szx) - totalW) / 2;
//    if (calculatedCenter < 5) calculatedCenter = 5;
//    // 2. Расчет координат с защитой от вылета за границы
//    if (HAlign() == AUIHAlign::center) {
//        drawX = (SafeINT32(szx) - totalW) / 2;
//        if (drawX < 0) drawX = 5; // Защитный фолбек, если текст шире кнопки
//    }
//    else if (HAlign() == AUIHAlign::left) {
//        drawX = 5;
//    }
//    else {
//        drawX = SafeINT32(szx) - totalW - 5;
//    }
//
//    if (drawX < 0) {
//        drawX = 5;
//    }
//    D3("[AUI CENTERING] Text: '{}' | EnumHAlign: {} | Button Width: {} | Measured Text Width: {} | Result drawX: {} | IdealCenter: {}",
//       Text().c_str(),
//       static_cast<int>(HAlign()),
//       szx,
//       totalW,
//       drawX,
//       calculatedCenter);
//    if (VAlign() == AUIVAlign::center) {
//        // Query the precise layout bounding box metrics for a standard lowercase character 'x'
//        int xDir, xAsc, xDesc;
//        XCharStruct xOverall;
//        XTextExtents(f, "x", 1, &xDir, &xAsc, &xDesc, &xOverall);
//        // Balance vertical offset based on lowercase x-height ascent to correct visual downward shift artifacts
//        drawY = (SafeINT32(szy) + xOverall.ascent - overall.descent) / 2 + 1;
//    }
//    else if (VAlign() == AUIVAlign::top) {
//        drawY = overall.ascent + 5;
//    }
//    else {
//        drawY = SafeINT32(szy) - overall.descent - 5;
//    }
//    if(mStyle == AUIWidgetStyle::Simple3D && mRenderPicture != None) {
//      D2("drawing 3d style {}", (UINT64)this)
//      bool pressed = IsHL();
//      int offset = pressed ? mDepth : 0;
//      unsigned int shrink = pressed ? static_cast<unsigned int>(mDepth * 2) : 0;
//      XSetForeground(d, gc, BGColor());
//      XFillRectangle(d, bb, gc, 0, 0, szx, szy);
//      if(!pressed) {
//        XRenderColor shadow = { 0, 0, 0, 0x6666 };
//        for (int i = 1; i <= mDepth; ++i)
//          XRenderFillRectangle(d, PictOpOver, mRenderPicture, &shadow, i, i,
//              szx, szy);
//      } else {
//        XRenderColor hole = { 0, 0, 0, 0x8888 };
//        XRenderFillRectangle(d, PictOpOver, mRenderPicture, &hole, 0, 0, szx,
//            szy);
//      }
//      XLinearGradient gradient;
//      RGBAColor cl;
//      cl.value = drawColor;
//      XRenderColor cTop, cBot;
//      double intensity = 0.5;
//      gradient.p1 = { 0, 0 };
//      gradient.p2 = { 0, XDoubleToFixed(static_cast<double>(szy)) };
//      if (pressed) {
//        // Pressed state: move top towards black, bottom towards white
//        cTop.red   = ScaleAndBlend(cl.rgba.r, 0, 0.2).red;
//        cTop.green = ScaleAndBlend(cl.rgba.g, 0, 0.2).green;
//        cTop.blue  = ScaleAndBlend(cl.rgba.b, 0, 0.2).blue;
//        cTop.alpha = 0xFFFF;
//        cBot.red   = ScaleAndBlend(cl.rgba.r, 255, 0.1).red;
//        cBot.green = ScaleAndBlend(cl.rgba.g, 255, 0.1).green;
//        cBot.blue  = ScaleAndBlend(cl.rgba.b, 255, 0.1).blue;
//        cBot.alpha = 0xFFFF;
//      } else {
//        // Normal/Hover: Top towards white, Bottom towards black
//        cTop.red   = ScaleAndBlend(cl.rgba.r, 255, intensity).red;
//        cTop.green = ScaleAndBlend(cl.rgba.g, 255, intensity).green;
//        cTop.blue  = ScaleAndBlend(cl.rgba.b, 255, intensity).blue;
//        cTop.alpha = 0xFFFF;
//        cBot.red   = ScaleAndBlend(cl.rgba.r, 0, intensity).red;
//        cBot.green = ScaleAndBlend(cl.rgba.g, 0, intensity).green;
//        cBot.blue  = ScaleAndBlend(cl.rgba.b, 0, intensity).blue;
//        cBot.alpha = 0xFFFF;
//      }
//      XRenderColor colors[] = { cTop, cBot };
//      XFixed stops[] = { XDoubleToFixed(0.0), XDoubleToFixed(1.0) };
//      Picture grad = XRenderCreateLinearGradient(d, &gradient, stops, colors,
//          2);
//      XRenderComposite(d, PictOpSrc, grad, None, mRenderPicture, 0, 0, 0, 0,
//          offset, offset, (szx > shrink ? szx - shrink : 1),
//          (szy > shrink ? szy - shrink : 1));
//      XRenderFreePicture(d, grad);
//      XSetForeground(d, gc, BlackPixel(d, cg->Scr()));
//      XDrawRectangle(d, bb, gc, 0, 0, szx - 1, szy - 1);
//      XSetForeground(d, gc, 0x333333);
//      XDrawString(d, bb, gc, drawX + offset, drawY + offset, TextPtr(),
//          (int) Text().length());
//    } else {
//      // Flat ---
//      XSetForeground(d, gc, drawColor);
//      XFillRectangle(d, bb, gc, 0, 0, szx, szy);
//      // Border and text
//      XSetForeground(d, gc, BlackPixel(d, cg->Scr()));
//      XDrawRectangle(d, bb, gc, 0, 0, szx - 1, szy - 1);
//      XDrawString(d, bb, gc, drawX, drawY, TextPtr(), (int) Text().length());
//    }
//    XCopyArea(d, bb, Wnd(), gc, 0, 0, szx, szy, 0, 0);
//    XFlush(d);
//    D2("button style {} {}", (UINT64)this, (UINT64)Style())
  }

  void AButton::OnButtonPress([[maybe_unused]]XEvent *ev) {
    if (!IsEnabled()) return; // Discard click tracking routines if button is disabled
    if(!IsHL()) {
      HL(true);
      Draw();
      XSync(AUIPtr()->Disp(), False);
    }
    AWidget::OnButtonPress(ev);
  }

  void AButton::OnButtonRelease(XEvent *ev) {
    if (!IsEnabled()) return;
    AUI *au = AUIPtr();
    if(!au || !au->IsWindowRegistered(ev->xany.window))
      return;
    INT64 win_x = ev->xbutton.x;
    INT64 win_y = ev->xbutton.y;
    // 1. Calculate hit test before any state modifications
    bool inside = (win_x >= 0 && win_y >= 0 && win_x < (INT64) SizeX()
        && win_y < (INT64) SizeY());
    // 2. Reset the highlight state safely
    if(IsHL()) {
      HL(false);
      // Redraw only if the click released outside.
      // If inside, the widget might be destroyed in the next step, making Draw() dangerous.
      Draw();
      XFlush(au->Disp());
    }
    if(inside) {
      AWidget::OnButtonRelease(ev);
      return;
    }
    AWidget::OnButtonRelease(ev);
  }

  AButton* AButton::AttachTo(AWidget *w, std::string inText) {
    return new AButton(inText, w);
  }

  void AButton::OnMouseEnter(UNUSED XEvent* ev) {
    if (!IsEnabled()) return; // Block hover updates inside disabled layouts
    mIsHovered = true;
    Draw();
  }

  void AButton::OnMouseLeave(UNUSED XEvent* ev) {
    if (!IsEnabled()) return;
    mIsHovered = false;
    Draw();
  }

  AButton::~AButton() {
    D3("v");
  }
}
