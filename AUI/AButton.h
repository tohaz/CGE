#ifndef ABUTTON_H_
#define ABUTTON_H_

#include "AWidget.h"

namespace aui {
  class AButton : public AWidget {
    private:
      AButton(std::string inText, AWidget* wParent);
    protected:
    public:
      static AButton* AttachTo(AWidget* wParent, std::string inText);
      virtual ~AButton();
      void Draw();
      void OnButtonPress(XEvent* ev);
      void OnButtonRelease(XEvent* ev);
  };
}

#endif
