#ifndef ABUTTON_H_
#define ABUTTON_H_

#include "AWidget.h"

namespace aui {
  class AButton : public AWidget {
    private:
      AButton(std::string inText, AWidget* wParent);
      Pixmap mBackBuffer = 0;
      void UpdateBuffer(); // Helper to (re)allocate pixmap if size changes
    protected:
    public:
      static AButton* AttachTo(AWidget* wParent, std::string inText);
      virtual ~AButton();
      void Draw();
      void OnButtonPress(XEvent* ev);
      void OnButtonRelease(XEvent* ev);
      void Resize(UINT32 inW, UINT32 inH);
  };
}

#endif
