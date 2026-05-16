#ifndef APOPUPMENU_H_
#define APOPUPMENU_H_

#include <vector>
#include <string>
#include <functional>
#include "AWidget.h"

namespace aui {
  struct AMenuItem {
    std::string text;
    std::string hotkey;
    std::function<void()> action;
    std::vector<AMenuItem> subItems; // Recursive for submenus
    bool isSeparator = false;
    bool isCheckable = false;
    bool isChecked = false;
    bool isEnabled = true;
    Picture icon = None;
  };
  AMenuItem createEmptyItem();
  class APopupMenu : public AWidget {
    private:
      APopupMenu(AWidget* wParent, const std::vector<AMenuItem>& inItems);
      std::vector<AMenuItem> mItems;
      APopupMenu* mActiveSubMenu = nullptr;
      APopupMenu* mParentMenu = nullptr;
      INT32 mHoveredIndex = -1;
      void CalculateLayout(UINT32& outW, UINT32& outH);
      void CloseSubMenu();
      void DrawItem(INT32 index, INT32 x, INT32 y, UINT32 w, UINT32 h);
      bool mIsGrabbed = false;
      INT32 mMinWidth = 80;
      INT32 mDepth = 2;
      INT32 mItemHeight = 16;
      INT32 mLeftMargin = 1;         // Distance from left border to text
      INT32 mRightMargin = 1;       // Space reserved for the submenu arrow ">"
      INT32 mSeparatorPadding = 10;   // Horizontal inset for the separator line
      UINT32 mDisabledColor = 0x888888;
      UINT32 mSeparatorShadow = 0x666666;
      UINT32 mSeparatorHighlight = 0xFFFFFF;
      INT32 mSeparatorHeight = 3; // Total vertical space for separator
      double mHoverIntensity = 0.15;      // How much to lighten the background on hover
      double mBaguetteLightFactor = 0.2;  // 3D border brightness (top/left)
      double mBaguetteDarkFactor = 0.3;   // 3D border darkness (bottom/right)
    protected:
      // Handlers for hover effects already supported by your AUI loop
      void OnMouseEnter(XEvent* ev) override;
      void OnMouseLeave(XEvent* ev) override;
    public:
      static APopupMenu* AttachTo(AWidget* wParent, const std::vector<AMenuItem>& inItems);
      virtual ~APopupMenu();
      void Draw() override;
      void OnButtonPress(XEvent* ev) override;
      void OnButtonRelease(XEvent* ev) override;
      void OnMouseMove(XEvent* ev) override;
      void Popup(INT32 rootX, INT32 rootY);
      void Dismiss();
      void SetItems(const std::vector<AMenuItem>& newItems);
      void SetParentMenu(APopupMenu* parent) { mParentMenu = parent; }
      INT32 HoveredIndex() const { return mHoveredIndex; }
      bool IsGrabbed() const { return mIsGrabbed; }
      INT32 ItemHeight() const { return mItemHeight; }
      INT32 SeparatorHeight() const { return mSeparatorHeight; }
      INT32 PressDepth() const { return mDepth; }
  };
}

#endif // APOPUPMENU_H_
