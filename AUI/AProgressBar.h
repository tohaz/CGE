#ifndef APROGRESS_BAR_H_
#define APROGRESS_BAR_H_

#include "AWidget.h"

namespace aui {

  class AProgressBar : public AWidget {
    private:
      double mProgress = 0.0;       // Progress value bounding between 0.0 and 1.0
      UINT32 mBarColor = 0xAACCAA;  // Color code token for the active filling bar
      AProgressBar(AWidget* wParent);
    public:
      virtual ~AProgressBar();
      static AProgressBar* AttachTo(AWidget* wParent);
      void Draw() override;
      void SetProgress(double progress);
      double GetProgress() const;
      void SetBarColor(UINT32 color);
      void Clear(); // Resets progress to 0.0
  };

}

#endif // APROGRESS_BAR_H_
