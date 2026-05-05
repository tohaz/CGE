#include "AWindow.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "AUILib.h"

namespace aui {
  AWindow::AWindow(std::string newTitle, AUI* cg) {
    SetAUIPtr(cg);
    SetTitle(newTitle);
    InitWindow();
  }

  AWindow::AWindow(std::string newTitle, AWidget* par) {
    SetAUIPtr(par->AUIPtr());
    SetWndParent(par);
    SetTitle(newTitle);
    InitWindow();
  }

  void AWindow::InitWindow() {
    D3("sizeof window %lu", sizeof(AWindow))
    SetType(AUIWidgetType::defaultWindow);
    SetBGColor(AUI_DEFAULT_WINDOW_BG);
    AUI* cg = AUIPtr();
    Display* d = cg->Disp();
    INT32 scr = cg->Scr();
    D3("wnd %lu, disp %lu, screen %lu", (UINT64)Wnd(), (UINT64)d, (UINT64)scr)
    SetSizeXY(AUI_DEFAULT_WINDOW_SZX, AUI_DEFAULT_WINDOW_SZY);
    if(!ParentWidget()) {
      D3("creating rooted window")
      InitWidgetProps(XCreateSimpleWindow(d, RootWindow(d, scr), 0, 0,
        SafeUINT32(SizeX()),
        SafeUINT32(SizeY()),
        1, BlackPixel(d, scr), WhitePixel(d, scr)));
      DisableResize();
    }
    else {
      D3("creating child window")
      InitWidgetProps(XCreateSimpleWindow(d, ParentWidget()->Wnd(), 0, 0,
        SafeUINT32(SizeX()),
        SafeUINT32(SizeY()),
        1, BlackPixel(d, scr), WhitePixel(d, scr)));
      DisableResize();
    }
    Window w = Wnd();
    XStoreName(d, w, Title().c_str());
    XSelectInput(d, w, ExposureMask | KeyPressMask | StructureNotifyMask| FocusChangeMask);
    XSetWindowBackground(d, w, BGColor());
    XMapWindow(d, w);
    cg->AddWidget(this);
    mWMDeleteMessage = XInternAtom(d, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(d, Wnd(), &mWMDeleteMessage, 1);
    XFlush(d);
  }

  AWindow* AWindow::AttachTo(AUI *cg, std::string newTitle) {
    return new AWindow(newTitle, cg);
  }

  AWindow* AWindow::AttachTo(AWidget *wParent, std::string newTitle) {
    return new AWindow(newTitle, wParent);
  }

  void AWindow::OnKeyPress(UNUSED XEvent *ev) {
    D3()
//    if (ev->type == ClientMessage) {
//      if (ev->xclient.data.l[0] == wmDeleteMessage) {
//      }
//    }
  }

  void AWindow::Draw() {
    D3()
  }

  AWindow::~AWindow() {
    D3("entering CGWindow destructor")
    D3("leaving CGWindow destructor")
  }

}
