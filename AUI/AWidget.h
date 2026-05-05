#ifndef AWIDGET_H_
#define AWIDGET_H_

#include <X11/Xlib.h>
#include <functional>
#include <map>
#include <string>
#include "defaults.h"

namespace aui {

  class AUI;

  class AWidget {
    private:
      Window mWindow = 0;
      AWidget* mWindowParent = 0;
      GC mGC = 0;
      XFontStruct* mFont = 0;
      AUI* mAUI = 0;
      std::string mTitle = "set title plz", mText = "set me";
      AUIWidgetType mType = AUIWidgetType::unset;
      INT64 mX = 10, mY = 10;
      UINT64 mSzX = 10, mSzY = 10;
      RGBAColor mBGColor = {0};
      std::map<Window, AWidget*> mWidg;
      bool mHL = false;
      bool mResizeEnabled = true;
      AUIHAlign mHAlign = AUIHAlign::center;
      AUIVAlign mVAlign = AUIVAlign::center;
      UINT32 mBorderSz = AUI_DEFAULT_LABEL_BORDERW;
      std::function<void(XEvent* ev, AWidget* w, void* data)> OnButtonPressCB = nullptr;
      std::function<void(XEvent* ev, AWidget* w, void* data)> OnFocusInCB = nullptr;
      std::function<void(XEvent* ev, AWidget* w, void* data)> OnFocusOutCB = nullptr;
      std::function<void(XEvent* ev, AWidget* w, void* data)> OnButtonReleaseCB = nullptr;
      std::function<void(XEvent* ev, AWidget* w, void* data)> OnMouseMoveCB = nullptr;
      std::function<void(AWidget* w, void* data)> OnSubmitCB = nullptr;
      void* mUserDataButtonPress = nullptr;
      void* mUserDataButtonRelease = nullptr;
      void* mUserDataFocusIn = nullptr;
      void* mUserDataFocusOut = nullptr;
      void* mUserDataMouseMove = nullptr;
      void* mUserDataSubmit = nullptr;
      Pixmap mBackBuffer = None;

    protected:
      void InitWidgetProps(Window w);
      void SetAUIPtr(AUI* p);
      void SetWndParent(AWidget* wParent);
      void UpdateBuffer();
      void SetSize(UINT64 szx, UINT64 szy);

    public:
      AWidget();
      virtual ~AWidget();
      AUI* AUIPtr();
      Window Wnd();
      AUIWidgetType Type();
      void SetTitle(std::string newTitle);
      void SetText(std::string newText);
      void SetType(AUIWidgetType inType);
      void SetXY(INT64 newX, INT64 newY);
      void SetSizeXY(UINT64 newSzX, UINT64 newSzY);
      void SetBGColor(UINT32 newBGColor);
      INT64 X();
      INT64 Y();
      UINT32 SizeXUI32();
      UINT32 SizeYUI32();
      UINT64 SizeX();
      UINT64 SizeY();
      UINT32 BGColor();
      const CHAR8* TextPtr();
      std::string& Text();
      std::string& Title();
      XFontStruct* Font();
      GC GCPtr();
      virtual void Draw();
      virtual void OnButtonPress(XEvent* ev);
      virtual void OnButtonRelease(XEvent* ev);
      virtual void OnFocusIn(XEvent* ev);
      virtual void OnFocusOut(XEvent* ev);
      virtual void OnKeyPress(XEvent* ev);
      virtual void OnMouseMove(XEvent* ev);
      virtual void OnSubmit();
      AWidget* ParentWidget();
      bool IsHL();
      void HL(bool newState);
      void AddWidgetChild(AWidget* newChild);
      void DestroyChildWidgets();
      void EnableResize();
      void DisableResize();
      void Resize(UINT32 szx, UINT32 szy);
      void ResizeY(UINT32 szy);
      void ResizeX(UINT32 szx);
      void Move(UINT32 x, UINT32 y);
      void UnregisterChild(AWidget *w);
      UINT32 BorderSz();
      void SetBorderSz(UINT32 borderSz);
      void SetOnButtonPressCB(std::function<void(XEvent* ev, AWidget* w, void* arbdata)> func, void* data);
      void SetOnButtonReleaseCB(std::function<void(XEvent* ev, AWidget* w, void* arbdata)> func, void* data);
      void SetOnSubmitCB(std::function<void(AWidget* w, void* arbdata)> func, void* data);
      AUIHAlign HAlign() const;
      void SetHAlign(AUIHAlign mHAlign);
      AUIVAlign VAlign() const;
      void SetVAlign(AUIVAlign mVAlign);
      void PrintDimensions();
      Pixmap BB();
      void SetBB(Pixmap backBuffer);
      void CorrectCoordinates(XEvent& event);
      void CorrectNegativeCoordinates(XEvent& event);
      void CorrectCoordinateX(INT32 &x);
      void CorrectCoordinateY(INT32 &y);

  };
}


#endif /* AWIDGET_H_ */
