#include <chrono>
#include <future>
#include <thread>
#include <memory.h>
#include "AUILib.h"
#include "AProgressBar.h"

using namespace aui;

std::thread StartProgressBarTenSecondLoop(AProgressBar* bar, AUI* au) {
  if (!bar || !au) return std::thread();
  D1("StartProgressBarTenSecondLoop() -> Launching 10-second background thread pipeline");
  return std::thread([bar, au]() {
    const int totalSteps = 100;
    const int stepDelayMs = 93;
    for (int step = 0; step <= totalSteps; ++step) {
      double calculatedProgress = static_cast<double>(step) / static_cast<double>(totalSteps);
      bar->SetProgress(calculatedProgress);
      std::this_thread::sleep_for(std::chrono::milliseconds(stepDelayMs));
    }
    D1("StartProgressBarTenSecondLoop() -> 100% Target reached. Triggering main loop exit sequence.");
    au->ExitAUI();
  });
}

void CloseButtonHandler(UNUSED XEvent* ev, AWidget* w, UNUSED void* d) {
  D1("CloseButtonHandler() -> Close click intercepted. Aborting main process loop thread.");
  w->AUIPtr()->ExitAUI();
}

int main() {
  AUI* au = AUI::Create("AProgressBar 10-Second Precision Test Suite");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(500, 250);
  AProgressBar* progressBar = AProgressBar::AttachTo(w);
  progressBar->Move(50, 120);
  progressBar->Resize(400, 45);
  progressBar->SetBarColor(0xAACCAA); // Soft mint-green fills
  std::thread worker = StartProgressBarTenSecondLoop(progressBar, au);
  au->ProcessMessages();
  if (worker.joinable()) {
    D1("main() -> Synchronizing background execution tracks via join()...");
    worker.join();
  }
  delete au;
  au = nullptr;
  progressBar = nullptr;
  D1("main() -> Finished. Safe shutdown confirmed.");
  return 0;
}

