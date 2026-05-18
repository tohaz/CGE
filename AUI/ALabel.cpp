#include "ALabel.h"

#include "AUILib.h"

namespace aui {

  ALabel::ALabel(UNUSED std::string inText, UNUSED AWidget *wParent) : AWidget() {
//    D3();
//    AUI *cg = wParent->AUIPtr();
//    Display* d = cg->Disp();
//    INT32 scr = cg->Scr();
//    SetType(AUIWidgetType::defaultLabel);
//    SetBGColor(AUI_DEFAULT_LABEL_BG);
//    SetXY(30, 30);
//    SetSizeXY(AUI_DEFAULT_LABEL_SZX, AUI_DEFAULT_LABEL_SZY);
//    SetAUIPtr(cg);
//    SetWndParent(wParent);
//    InitWidgetProps(XCreateSimpleWindow(d, wParent->Wnd(), SafeINT32(X()), SafeINT32(Y()),
//        SafeUINT32(SizeX()), SafeUINT32(SizeY()), BorderSz(),
//        BlackPixel(d, scr), BGColor()));
//    Window w = Wnd();
//    XSelectInput(d, w, ExposureMask|ButtonPressMask|KeyPressMask|ButtonReleaseMask);
//    XMapWindow(d, w);
//    AUIPtr()->AddWidget(this);
//    SetText(inText);
//    SetBorderSz(AUI_DEFAULT_LABEL_BORDERW);
//    D3("label: disp={}, wnd={}, scr={}", (INT64) d, (INT64) w, scr)
  }

  ALabel* ALabel::AttachTo(AWidget *w, std::string inText) {
    return new ALabel(inText, w);
  }

  void ALabel::Draw() {
    D2("label:{}", Text().c_str())
    Display *d = AUIPtr()->Disp();
    Window wi = Wnd();
    XFontStruct *f = Font();
    XClearWindow(d, wi);
    XSetWindowBorderWidth(d, wi, BorderSz());
    INT32 totalW = XTextWidth(f, Text().c_str(), (INT32) Text().size());
    INT32 drawX = 0;
    INT32 drawY = 0;
    switch (HAlign()) {
      case AUIHAlign::left:
        drawX = 5;
        break;
      case AUIHAlign::center:
        drawX = (SafeINT32(SizeX()) - totalW) / 2;
        break;
      case AUIHAlign::right:
        drawX = SafeINT32(SizeX()) - totalW - 5;
        break;
      default:
        E("halign junk")
        break;
    }
    switch (VAlign()) {
      case AUIVAlign::top:
        drawY = f->ascent + 5;
        break;
      case AUIVAlign::center:
        drawY = (SafeINT32(SizeY()) + f->ascent - f->descent) / 2;
        break;
      case AUIVAlign::bottom:
        drawY = SafeINT32(SizeY()) - f->descent - 5;
        break;
      default:
        E("valign junk")
        break;
    }
    XDrawString(d, wi, GCPtr(), drawX, drawY, Text().c_str(),
        (int) Text().size());
    XFlush(d);
  }

  ALabel::~ALabel() {
  }

  ALabel& ALabel::operator =(UNUSED const std::string &newValue) {
    SetText(newValue);
    return *this;
  }

  ALabel& ALabel::operator+=(const std::string& text) {
    this->AddText(text);
    return *this;
  }

  ALabel& ALabel::operator+=(const char* text) {
    this->AddText(text);
    return *this;
  }

}
