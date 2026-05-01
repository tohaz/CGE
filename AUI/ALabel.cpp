#include "ALabel.h"

#include "AUILib.h"

namespace aui {

  ALabel::ALabel(std::string inText, AWidget *wParent) : AWidget() {
    D3();
    AUI *cg = wParent->AUIPtr();
    Display* d = cg->Disp();
    UINT32 scr = cg->Scr();
    SetType(AUIWidgetType::defaultLabel);
    SetBGColor(AUI_DEFAULT_LABEL_BG);
    SetXY(30, 30);
    SetSizeXY(AUI_DEFAULT_LABEL_SZX, AUI_DEFAULT_LABEL_SZY);
    SetAUIPtr(cg);
    SetWndParent(wParent);
    InitWidgetProps(XCreateSimpleWindow(d, wParent->Wnd(), X(), Y(), SizeX(), SizeY(), BorderSz(),
        BlackPixel(d, scr), BGColor()));
    Window w = Wnd();
    XSelectInput(d, w, ExposureMask|ButtonPressMask|KeyPressMask|ButtonReleaseMask);
    XMapWindow(d, w);
    AUIPtr()->AddWidget(this);
    SetText(inText);
    SetBorderSz(AUI_DEFAULT_LABEL_BORDERW);
    D3("label: disp=%lu, wnd=%lu, scr=%d", (INT64) d, (INT64) w, scr)
  }

  ALabel* ALabel::AttachTo(AWidget *w, std::string inText) {
    return new ALabel(inText, w);
  }

  void ALabel::Draw() {
    D2("label:%s", Text().c_str())
    Display *d = AUIPtr()->Disp();
    Window wi = Wnd();
    XFontStruct *f = Font();
    XClearWindow(d, wi);
    XSetWindowBorderWidth(d, wi, BorderSz());
    int totalW = XTextWidth(f, Text().c_str(), (int) Text().size());
    int fontHeight = f->ascent + f->descent;
    int drawX = 0;
    int drawY = 0;
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
    XDrawString(d, wi, GCPtr(), drawX, drawY, Text().c_str(),
        (int) Text().size());
    XFlush(d);

  }

  ALabel::~ALabel() {
  }

}
