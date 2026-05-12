#ifndef APOPUPMENU_H_
#define APOPUPMENU_H_

#include <vector>
#include <string>
#include <functional>
#include "AWidget.h"

namespace aui {
  /**
   * Structure for menu entries.
   * Uses std::function for flexible callbacks.
   */
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
      // Constructor is private to enforce factory usage and proper X11 setup
      APopupMenu(AWidget* wParent, const std::vector<AMenuItem>& inItems);
      std::vector<AMenuItem> mItems;
      APopupMenu* mActiveSubMenu = nullptr;
      APopupMenu* mParentMenu = nullptr;
      INT32 mHoveredIndex = -1;
      // Internal layout and resource management
      void CalculateLayout(UINT32& outW, UINT32& outH);
      void CloseSubMenu();
      // Using your SafeINT32 for pixel-perfect Xlib calls
      void DrawItem(INT32 index, INT32 x, INT32 y, UINT32 w, UINT32 h);
      bool mIsGrabbed = false;
      INT32 mMinWidth = 80;
      INT32 mDepth = 2;
      INT32 mItemHeight = 16;
      INT32 mLeftMargin = 1;         // Distance from left border to text
      INT32 mRightMargin = 1;       // Space reserved for the submenu arrow ">"
      INT32 mSeparatorPadding = 10;   // Horizontal inset for the separator line
      // Colors & Blending
      UINT32 mDisabledColor = 0x888888;
      UINT32 mSeparatorShadow = 0x666666;
      UINT32 mSeparatorHighlight = 0xFFFFFF;
      INT32 mSeparatorHeight = 3; // Total vertical space for separator
      // Visual Factors (0.0 to 1.0)
      double mHoverIntensity = 0.15;      // How much to lighten the background on hover
      double mBaguetteLightFactor = 0.2;  // 3D border brightness (top/left)
      double mBaguetteDarkFactor = 0.3;   // 3D border darkness (bottom/right)

    protected:
      // Handlers for hover effects already supported by your AUI loop
      void OnMouseEnter(XEvent* ev) override;
      void OnMouseLeave(XEvent* ev) override;
    public:
      /**
       * Factory method: Creates the menu window with override_redirect.
       * This is critical for menus to appear outside parent window bounds.
       */
      static APopupMenu* AttachTo(AWidget* wParent, const std::vector<AMenuItem>& inItems);
      virtual ~APopupMenu();
      // Standard AUI overrides
      void Draw() override;
      void OnButtonPress(XEvent* ev) override;
      void OnButtonRelease(XEvent* ev) override;
      void OnMouseMove(XEvent* ev) override;
      /**
       * Opens the menu at screen coordinates and performs XGrabPointer.
       */
      void Popup(INT32 rootX, INT32 rootY);
      /**
       * Dismisses the entire menu chain and releases the grab.
       */
      void Dismiss();
      // Management methods
      void SetItems(const std::vector<AMenuItem>& newItems);
      void SetParentMenu(APopupMenu* parent) { mParentMenu = parent; }
      // Getter for testing framework
      INT32 HoveredIndex() const { return mHoveredIndex; }
      bool IsGrabbed() const { return mIsGrabbed; }
      INT32 ItemHeight() const { return mItemHeight; }
      INT32 SeparatorHeight() const { return mSeparatorHeight; }
      INT32 PressDepth() const { return mDepth; }


  };
}

#endif // APOPUPMENU_H_
