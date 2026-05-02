#include "AButton.h"

#include "AUILib.h"

namespace aui {
  AButton::AButton(std::string inText, AWidget *wParent) {
    AUI *aui = wParent->AUIPtr();
    SetAUIPtr(aui);
    SetWndParent(wParent);
    Display *d = aui->Disp();
    UINT32 scr = aui->Scr();
    SetType(AUIWidgetType::defaultButton);
    SetXY(AUI_DEFAULT_BUTTON_X, AUI_DEFAULT_BUTTON_Y);
    SetSizeXY(AUI_DEFAULT_BUTTON_SZX, AUI_DEFAULT_BUTTON_SZY);
    SetBGColor(AUI_DEFAULT_BUTTON_BG);
    XSetWindowAttributes swa;
    swa.background_pixmap = None;
    swa.border_pixel = BlackPixel(d, scr);
    swa.bit_gravity = StaticGravity;
    // ФИКС: SaveUnder заставляет сервер корректно обновлять фон ПОД рамкой кнопки
    swa.save_under = True;
    swa.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask;
    Window w = XCreateWindow(d, wParent->Wnd(), X(), Y(), SizeX(), SizeY(),
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
    if(!cg || Wnd() == 0 || GCPtr() == 0) {
      D3("x")
      return;
    }
    Display *d = cg->Disp();
    Window wi = Wnd();
    if(!BB()) {
      D3("x")
      AWidget::UpdateBuffer();
    }
    Pixmap bb = BB();
    if(!bb) {
      D3("x")
      return;
    }
    XFontStruct *f = Font();
    XSetForeground(d, GCPtr(), IsHL() ? HLColor(BGColor()) : BGColor());
    XFillRectangle(d, bb, GCPtr(), 0, 0, SizeX(), SizeY());
    INT32 totalW = XTextWidth(f, Text().c_str(), (INT32) Text().size());
    INT32 drawX = 0;
    INT32 drawY = 0;
    switch (HAlign()) {
      case AUIHAlign::left:
        drawX = 5;
        break;
      case AUIHAlign::center:
        drawX = (SizeX() - totalW) / 2;
        break;
      case AUIHAlign::right:
        drawX = SizeX() - totalW - 5;
        break;
    }
    switch (VAlign()) {
      case AUIVAlign::top:
        drawY = f->ascent + 5;
        break;
      case AUIVAlign::center:
        drawY = (SizeY() + f->ascent - f->descent) / 2;
        break;
      case AUIVAlign::bottom:
        drawY = SizeY() - f->descent - 5;
        break;
    }
    XSetForeground(d, GCPtr(), BlackPixel(d, cg->Scr()));
    XDrawString(d, bb, GCPtr(), drawX, drawY, TextPtr(),
        Text().length());
    XSetWindowBorder(d, wi, BlackPixel(d, cg->Scr()));
    XCopyArea(d, bb, wi, GCPtr(), 0, 0, SizeX(), SizeY(), 0, 0);
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
    D3("v")
  }

}
