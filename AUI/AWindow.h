#ifndef AWINDOW_H_
#define AWINDOW_H_

#include <X11/Xlib.h>

#include "AWidget.h"

namespace aui {

  class AWindow : public AWidget {
    private:
      void InitWindow();
      AWindow(std::string title, AUI* au);
      AWindow(std::string title, AWidget* parent);
      Atom mWMDeleteMessage = 0;

    public:
      static AWindow* AttachTo(AUI* au, std::string title);
      static AWindow* AttachTo(AWidget* parent, std::string title);
      void OnKeyPress(XEvent *ev);

      void Draw();
      virtual ~AWindow();
  };
}

#endif
