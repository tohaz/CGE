#ifndef ALABEL_H_
#define ALABEL_H_

#include "AWidget.h"

namespace aui {
  class ALabel : public AWidget {
    private:
      ALabel(std::string inText, AWidget *wParent);
    protected:
    public:
      static ALabel* AttachTo(AWidget* w, std::string inText);
      virtual ~ALabel();
      void Draw();
      ALabel& operator=(const std::string& newText);
      ALabel& operator+=(const std::string& text);
      ALabel& operator+=(const char* text);
  };
}

#endif
