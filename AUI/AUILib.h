#ifndef AUILIB_H_
#define AUILIB_H_

#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <sys/types.h>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <stack>
#include <unordered_map>
#include <string.h>

#include "AButton.h"
#include "AInputBox.h"
#include "ALabel.h"
#include "AList.h"
#include "ATable.h"
#include "AWidget.h"
#include "AWindow.h"
#include "defaults.h"

namespace aui {
  class AWidget;
  class AWindow;
  UINT32 HLColor(UINT32 ci);

  class AUI {
    private:
      AUI();
      AUI(std::string windowTitle);
      AWindow* mMainWnd = 0;
      void CreateMainWindow();
      bool mShouldExit = false;
      Display* mDisplay = 0;
      INT32 mScreen = 0;
      std::string mWindowTitle = "set it plz";
      Atom mWMDeleteMessage = 0;
      void CloseDisplay();
      void RemoveMiscWindows();
//      std::unordered_map<Window, CGWidget*> mWidg;
      std::map<Window, AWidget*> mWidg;
      UINT64 mAlphabetLen = BaseAlphabet.length();
    protected:

    public:
      static AUI* Create(std::string windowTitle);
      void ProcessMessages();
      void AddWidget(AWidget* w);
      void ExitAUI();
      AWindow* MainWnd();
      Display* Disp();
      void UnregisterWindow(Window w);
      INT32 Scr();
      void RemoveWidget(Window w);
      void RemoveParentWidget(Window w);
      void RemoveParentWidget(AWidget* w);
      AWidget* GetWidget(Window w);
      bool IsWindowRegistered(Window w);
      const char* XEventToString(INT32 ev);
      std::string NumberToBaseString(UINT64 num);
      ~AUI();
  };
}

#endif
