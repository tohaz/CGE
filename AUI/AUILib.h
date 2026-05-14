#ifndef AUILIB_H_
#define AUILIB_H_

#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <sys/types.h>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <regex>
#include <string>
#include <stack>
#include <type_traits>
#include <unordered_map>
#include <string.h>

#include "AButton.h"
#include "AInputBox.h"
#include "ALabel.h"
#include "AList.h"
#include "ATable.h"
#include "AWidget.h"
#include "AWindow.h"
#include "APopupMenu.h"
#include "AModalWindow.h"
#include "defaults.h"

namespace aui {
#pragma GCC push_options
#pragma GCC optimize ("O2")

  static __attribute__((always_inline)) inline INT16 SafeINT16(UINT16 val) {
    if(val > static_cast<UINT16>(std::numeric_limits<INT16>::max())) [[unlikely]] {
      E("UINT16 to INT16 conversion error");
    }
    return static_cast<INT16>(val);
  }

  static __attribute__((always_inline)) inline INT16 SafeINT16(INT32 val) {
    if(val > std::numeric_limits<INT16>::max()
        || val < std::numeric_limits<INT16>::min()) [[unlikely]] {
      E("INT32 to INT16 conversion error");
    }
    return static_cast<INT16>(val);
  }

  static __attribute__((always_inline)) inline INT16 SafeINT16(UINT32 val) {
    if(val > static_cast<UINT32>(std::numeric_limits<INT16>::max())) [[unlikely]] {
      E("UINT32 to INT16 conversion error");
    }
    return static_cast<INT16>(val);
  }

  static __attribute__((always_inline)) inline INT16 SafeINT16(INT64 val) {
    if(val > std::numeric_limits<INT16>::max()
        || val < std::numeric_limits<INT16>::min()) [[unlikely]] {
      E("INT64 to INT16 conversion error");
    }
    return static_cast<INT16>(val);
  }

  static __attribute__((always_inline)) inline UINT16 SafeUINT16(UINT32 val) {
    if(val > std::numeric_limits<UINT16>::max()) [[unlikely]] {
      E("UINT32 to UINT16 conversion error");
    }
    return static_cast<UINT16>(val);
  }

  static __attribute__((always_inline)) inline UINT16 SafeUINT16(INT64 val) {
      if(val < 0 || val > static_cast<INT64>(std::numeric_limits<UINT16>::max())) [[unlikely]] {
        E("INT64 to UINT16 conversion error");
      }
      return static_cast<UINT16>(val);
  }

  static __attribute__((always_inline)) inline INT32 SafeINT32(UINT32 val) {
    if(val > static_cast<UINT32>(std::numeric_limits<INT32>::max())) [[unlikely]] {
      E("UINT32 to INT32 conversion error");
    }
    return static_cast<INT32>(val);
  }

  static __attribute__((always_inline)) inline INT32 SafeINT32(INT64 val) {
    if(val > std::numeric_limits<INT32>::max()
        || val < std::numeric_limits<INT32>::min()) [[unlikely]] {
      E("INT64 to INT32 conversion error");
    }
    return static_cast<INT32>(val);
  }

  static __attribute__((always_inline)) inline INT32 SafeINT32(UINT64 val) {
    if(val > static_cast<UINT64>(std::numeric_limits<INT32>::max())) [[unlikely]] {
      E("UINT64 to INT32 conversion error");
    }
    return static_cast<INT32>(val);
  }

  static __attribute__((always_inline)) inline UINT32 SafeUINT32(INT32 val) {
    if(val < 0) [[unlikely]] {
      E("INT32 to UINT32 conversion error (negative)");
    }
    return static_cast<UINT32>(val);
  }


  static __attribute__((always_inline)) inline UINT32 SafeUINT32(INT64 val) {
    if(val > static_cast<INT64>(std::numeric_limits<UINT32>::max()) || val < 0) [[unlikely]] {
      E("INT64 to UINT32 conversion error");
    }
    return static_cast<UINT32>(val);
  }

  static __attribute__((always_inline)) inline UINT32 SafeUINT32(UINT64 val) {
    if(val > (UINT64) std::numeric_limits<UINT32>::max()) [[unlikely]] {
      E("UINT64 to UINT32 conversion error");
    }
    return static_cast<UINT32>(val);
  }

  static __attribute__((always_inline)) inline INT64 SafeINT64(UINT64 val) {
    if(val > static_cast<UINT64>(std::numeric_limits<INT64>::max())) [[unlikely]] {
      E("UINT64 to INT64 conversion error");
    }
    return static_cast<INT64>(val);
  }

  static __attribute__((always_inline)) inline UINT64 SafeUINT64(INT32 val) {
    if(val < 0) [[unlikely]] {
      DS()
      E("INT32 to UINT64 conversion error");
    }
    return static_cast<UINT64>(val);
  }

  static __attribute__((always_inline)) inline UINT64 SafeUINT64(UINT32 val) {
    return static_cast<UINT64>(val);
  }

  static __attribute__((always_inline)) inline UINT64 SafeUINT64(INT64 val) {
    if(val < 0) [[unlikely]] {
      E("INT64 to UINT64 conversion error");
    }
    return static_cast<UINT64>(val);
  }

  static __attribute__((always_inline)) inline INT32 SafeINT32(double val) {
    if(std::isnan(val) || val > static_cast<double>(std::numeric_limits<INT32>::max())
        || val < static_cast<double>(std::numeric_limits<INT32>::min())) [[unlikely]] {
      E("double to INT32 conversion error (overflow or NaN)")
    }
    return static_cast<INT32>(val);
  }

  static __attribute__((always_inline)) inline UINT32 SafeUINT32(double val) {
    if(std::isnan(val) || val > static_cast<double>(std::numeric_limits<UINT32>::max())
        || val < 0.0) [[unlikely]] {
      E("double to UINT32 conversion error (overflow, negative, or NaN)")
    }
    return static_cast<UINT32>(val);
  }

  static __attribute__((always_inline)) inline INT64 SafeINT64(double val) {
    if (std::isnan(val) || val > static_cast<double>(std::numeric_limits<INT64>::max())
          || val < static_cast<double>(std::numeric_limits<INT64>::min())) [[unlikely]] {
      E("double to INT64 conversion error (overflow or NaN)")
    }
    return static_cast<INT64>(val);
  }


#pragma GCC pop_options

  class AWidget;
  class AWindow;
  class APopupMenu;
  class AModalWindow;
  UINT32 HLColor(UINT32 ci);
  UINT32 HoverColor(UINT32 ci);
  XRenderColor ScaleAndBlend(uint8_t base, uint8_t target, double t);
  UINT32 GetBlendedColor(UINT32 baseColor, uint8_t target, double t);

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
      APopupMenu* mActiveRootMenu = nullptr;
      AWidget* mModalWidget = nullptr;
//      std::vector<AModalWindow*> mModalStack;
      std::vector<AWidget*> mModalStack;
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
      void SetModal(AWidget* w) { mModalWidget = w; }
      bool HasWidget(Window w) const { return mWidg.contains(w); }
      void RegisterExternalWindow(Window w, AWidget* owner);
      void UnregisterExternalWindow(Window w);
      void PushModal(AWidget* win);
      void PopModal(AWidget* win);
      AWidget* GetModal() const { return mModalStack.empty() ? nullptr : mModalStack.back(); }

      ~AUI();
  };
}

#endif
