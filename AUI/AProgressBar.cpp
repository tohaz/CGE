#include "AProgressBar.h"
#include "AUILib.h"
#include <X11/Xlib.h>
#include <string>

namespace aui {

  AProgressBar::AProgressBar(AWidget* wParent) : AWidget() {
    if (!wParent) [[unlikely]] return;
    AUI* cg = wParent->AUIPtr();
    Display* d = cg->Disp();
    INT32 scr = cg->Scr();
    SetType(AUIWidgetType::defaultProgressBar);
    SetBGColor(AUI_DEFAULT_BUTTON_BG); // Standard framework grey tint
    SetXY(AUI_TABLE_X, AUI_TABLE_Y);   // Reusing default layout constants
    SetSizeXY(200, 30);                // Default single-line widget dimensions
    SetAUIPtr(cg);
    SetWndParent(wParent);
    SetBB(None);                       // Initialize the backbuffer pointer state to None
    InitWidgetProps(XCreateSimpleWindow(d, wParent->Wnd(), SafeINT32(X()), SafeINT32(Y()),
            SafeUINT32(SizeX()), SafeUINT32(SizeY()), 1, BlackPixel(d, scr),
            BGColor()));
    Window w = Wnd();
    XSelectInput(d, w, ExposureMask | StructureNotifyMask);
    XMapWindow(d, w);
    cg->AddWidget(this);
  }

  AProgressBar* AProgressBar::AttachTo(AWidget* wParent) {
    return new AProgressBar(wParent);
  }

  void AProgressBar::SetProgress(double progress) {
    if (progress < 0.0) progress = 0.0;
    if (progress > 1.0) progress = 1.0;
    mProgress = progress;
    Draw(); // Cascade render ticks down instantly upon properties mutation
  }

  double AProgressBar::GetProgress() const {
    return mProgress;
  }

  void AProgressBar::SetBarColor(UINT32 color) {
    mBarColor = color;
    Draw();
  }

  void AProgressBar::Clear() {
    mProgress = 0.0;
    Draw();
  }

  void AProgressBar::Draw() {
    if (Wnd() == 0) return;
    AUI* au = AUIPtr();
    GC gc = GCPtr();
    Display* d = au->Disp();
    XFontStruct* fo = Font();
    UpdateBuffer();
    Drawable dest = BB();
    if (dest == 0) {
      dest = Wnd();
    }
    UINT32 w = static_cast<UINT32>(SizeX());
    UINT32 h = static_cast<UINT32>(SizeY());
    XSetForeground(d, gc, BGColor());
    XFillRectangle(d, dest, gc, 0, 0, w, h);
    UINT32 barWidth = static_cast<UINT32>(static_cast<double>(w) * mProgress);
    if (barWidth > 0) {
      XSetForeground(d, gc, mBarColor);
      XFillRectangle(d, dest, gc, 0, 0, barWidth, h);
    }
    int percent = static_cast<int>(mProgress * 100.0);
    std::string text = std::to_string(percent) + "%";
    int textLen = static_cast<int>(text.length());
    int textW = fo ? XTextWidth(fo, text.c_str(), textLen) : textLen * 7;
    int textX = (static_cast<int>(w) - textW) / 2;
    int textH = fo ? (fo->ascent + fo->descent) : 14;
    int textY = (static_cast<int>(h) - textH) / 2 + (fo ? fo->ascent : 11);
    XSetForeground(d, gc, 0x000000); // Sharp solid black pixels
    XDrawString(d, dest, gc, textX, textY, text.c_str(), textLen);
    if (dest != Wnd()) {
      XCopyArea(d, dest, Wnd(), gc, 0, 0, w, h, 0, 0);
    }
    XSync(d, False);
  }

  AProgressBar::~AProgressBar() {}

} // namespace aui
