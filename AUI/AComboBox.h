#ifndef ACOMBOBOX_H_
#define ACOMBOBOX_H_

#include <string>
#include <vector>
#include <functional>

#include "AWidget.h"
#include "AInputBox.h"
#include "AButton.h"
#include "AWindow.h"
#include "AList.h"

namespace aui {

  struct AComboBoxHelper : public AWidget {
    static void SetupPopup(AWidget* target, Window w, AUI* auiEnv, AWidget* parent) {
      auto* proxy = static_cast<AComboBoxHelper*>(target);
      proxy->SetAUIPtr(auiEnv);
      proxy->SetWndParent(parent);
      proxy->SetType(AUIWidgetType::defaultComboBox);
      proxy->InitWidgetProps(w);
      proxy->UpdateBuffer();
    }
  };

  class AComboBox : public AWidget {
    private:
      AInputBox* mInputBox = nullptr;   // Displays or allows editing of the active value
      AButton*   mArrowBtn = nullptr;   // The drop-down trigger button (renders '▼')
      AWindow*   mPopupWin = nullptr;   // Floating Top-level container for bounds insulation
      AList*     mListView = nullptr;   // Reused production list module for item rendering
      // Data models
      std::vector<std::string> mItems;
      INT64 mSelectedIndex = -1;
      bool mIsPopupOpen = false;
      bool mIsEditable = false;
      // Private constructor enforced by the factory initialization pattern
      AComboBox(AWidget* wParent);
      AComboBox(AWidget* wParent, std::string element);
      // Core internal synchronization procedures
      void PositionPopup();
      // Static component interaction routing callbacks
      void OnArrowClick(XEvent* ev, AWidget* w, void* data);
      static void OnInputClick(XEvent* ev, AWidget* w, void* data);
      static void OnItemSelect(XEvent* ev, AWidget* w, void* data);
      void OnGlobalIntercept(XEvent* ev, AWidget* w, void* data);
      // User notification callback hook registry
      std::function<void(INT64 index, const std::string& text)> mOnSelectionChangedCB = nullptr;
      std::function<void(AWidget* w, INT32 selectedIndex, void* arbData)> mOnSelectionChangeCB = nullptr;
      void* mUserCallbackData = nullptr;
    public:
      void OpenDropDown();
      void CloseDropDown();
      // Factory generation access boundary entry-point
      static AComboBox* AttachTo(AWidget* wParent);
      static AComboBox* AttachTo(AWidget* wParent, std::string inText);
      static AComboBox* AttachTo(AWidget* wParent, std::initializer_list<std::string> items);
      virtual ~AComboBox();
      void Draw() override;
      void Resize(UINT32 w, UINT32 h) override;
      void Move(UINT32 x, UINT32 y) override;
      void ClearItems();
      void SetSelectedIndex(INT64 index);
      INT64 SelectedIndex() const { return mSelectedIndex; }
      std::string SelectedText() const;
      size_t ItemCount() const { return mItems.size(); }
      void SetOnSelectionChanged(std::function<void(INT64 index, const std::string& text)> cb) {
        mOnSelectionChangedCB = std::move(cb);
      }
      bool IsPopupOpen() const { return mIsPopupOpen; }
      void OnButtonPress(XEvent* ev);
      void Clear();
      INT64 GetSelectedIndex() const;
      void SetSelectedIndex(INT32 index);
      std::string GetItemText(INT32 index) const;
      void AddItem(std::string item);
      void SetOnSelectionChangeCB(std::function<void(AWidget* w, INT32 selectedIndex, void* arbData)> func, void* data);
  };

} // namespace aui

#endif // ACOMBOBOX_H_
