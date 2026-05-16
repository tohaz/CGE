#ifndef APROGRESSBAR_H_
#define APROGRESSBAR_H_

#include "AWidget.h"
#include <functional> // Required for std::function progress provider callback
#include <thread>      // Required for the background animation/polling thread
#include <mutex>       // Required for protecting state mutations across threads
#include <condition_variable> // Required for instant interruptible thread sleep handling

namespace aui {

  class AProgressBar : public AWidget {
    public:
      AProgressBar(AWidget* wParent);
      static AProgressBar* AttachTo(AWidget* wParent);
      void SetProgress(double progress);
      double GetProgress() const;
      void SetBarColor(UINT32 color);
      void Clear();
      void Draw() override;
      void SetUpdateInterval(UINT32 intervalMs);
      void SetProgressProvider(std::function<double()> provider); // Attached data source method declaration
      ~AProgressBar() override;

    private:
      double mProgress = 0.0;
      UINT32 mBarColor = 0x00FF00; // Default green progress color
      std::thread mUpdateThread;
      mutable std::mutex mThreadMutex;
      std::condition_variable mThreadCv;
      bool mStopThread = false;
      UINT32 mUpdateIntervalMs = 1000; // Default refresh rate boundary interval
      std::function<double()> mProgressProvider; // Internal callback object variable storage
  };

} // namespace aui

#endif // APROGRESSBAR_H_
