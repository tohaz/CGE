#include "AUILib.h"
#include "ATable.h"

#include "defaults.h"

namespace aui {
  void ATable::AddColumns(UINT32 number) {
    if (number == 0) return;
    AUI *au = AUIPtr();
    std::string name = "";
    UINT64 startIdx = mColumnW.size();
    for (UINT32 i = 0; i < number; ++i) {
      UINT64 currentIdx = startIdx + i;
      name = au->NumberToBaseString(currentIdx);
      mColumnW[SafeINT64(currentIdx)] = {AUI_TABLE_CELL_W, name};
      mTotalContentWidth += AUI_TABLE_CELL_W;
    }
    Draw();
  }

  void ATable::AddRows(UINT32 number) {
    if (number == 0) return;
    INT64 startIdx = (INT64)mRowH.size();
    INT64 currentIdx = 0;
    for (UINT32 i = 0; i < number; ++i) {
      currentIdx = startIdx + i;
      mRowH[currentIdx] = {AUI_TABLE_CELL_H, std::to_string(currentIdx)};
      mTotalContentHeight += AUI_TABLE_CELL_H;
    }
    Draw();
  }

  void ATable::SetAutoWiden(bool enable) {
    D1()
    mAutoWiden = enable;
  }

  std::string ATable::RowName(INT64 rowIdx) {
    auto it = mRowH.find(rowIdx);
    if(it != mRowH.end())
      return it->second.second;
    return "";
  }

  void ATable::SetColumnWidth(INT64 colIdx, INT64 width) {
    auto it = mColumnW.find(colIdx);
    if(it != mColumnW.end()) {
      INT64 diff = width - it->second.first;
      it->second.first = width;
      mTotalContentWidth += diff;
      INT64 viewW = (INT64) SizeX() - mRowHeaderWidth - AUI_TABLE_SCROLL_THICK;
      INT64 maxScroll = std::max((INT64) 0, mTotalContentWidth - viewW);
      if(mHOffset > maxScroll)
        mHOffset = maxScroll;
      Draw();
    }
  }

  std::string ATable::ColumnName(INT64 colIdx) {
    auto it = mColumnW.find(colIdx);
    if(it != mColumnW.end())
      return it->second.second;
    return "";
  }

  void ATable::Clear() {
    mRows.clear();
    mColumns.clear();
    mTotalContentWidth = 0;
    mTotalContentHeight = 0;
    mRowH.clear();
    mColumnW.clear();
    mVOffset = 0;
    mHOffset = 0;
    mSelectedRow = -1;
    mCursorRow = -1;
    mCursorCol = -1;
    mRowHeaderWidth = 40;
    Draw();
  }

  void ATable::SetRowName(INT64 rowIdx, const std::string &name) {
    auto it = mRowH.find(rowIdx);
    if(it != mRowH.end()) {
      it->second.second = name;
      XFontStruct *font_info = Font();
      INT32 text_width = font_info ?
                XTextWidth(font_info, name.c_str(), SafeINT32(name.length())) :
                SafeINT32(name.length() * 7);
      if(SafeUINT32(text_width + 12) > mRowHeaderWidth) {
        mRowHeaderWidth = SafeUINT32(text_width + 12);
      }
      Draw();
    } else {
      E("Attempted to set name for non-existent row: %ld", rowIdx);
    }
  }

  void ATable::SetColumnName(INT64 colIdx, std::string &name) {
    // 1. Locate the column in the width/label map
    auto it = mColumnW.find(colIdx);
    if(it != mColumnW.end()) {
      it->second.second = name;
      XFontStruct *font_info = Font();
      int text_width =
          font_info ?
              XTextWidth(font_info, name.c_str(), (int) name.length()) :
              (int) name.length() * 7;
      if((INT64) text_width + 20 > it->second.first) {
        INT64 newW = (INT64) text_width + 20;
        mTotalContentWidth += (newW - it->second.first);
        it->second.first = newW;
      }
      Draw();
    } else {
      E("Attempted to set name for non-existent column: %ld", colIdx);
    }
  }

  void ATable::PrintDebugState() {
    D1("ATable Debug State");
    D1("Widget Size: %lux%lu", SizeX(), SizeY());
    D1("Content Size: TotalW=%ld, TotalH=%ld", mTotalContentWidth,
        mTotalContentHeight);
    D1("Offsets: HOffset=%ld, VOffset=%ld", mHOffset, mVOffset);
    D1("Headers: RowW=%d, ColH=%d", mRowHeaderWidth, mColumnHeaderHeight);
    D1("Scroll Mode: %d", (INT32)mCurrentScrollMode);
    D1("--------------------------");
  }

  void ATable::RemoveLastRow() {
    if(mRowH.empty())
      return;
    // std::map is sorted, so rbegin() is the highest index
    RemoveRow(mRowH.rbegin()->first);
  }

  AUICellData* ATable::Get(INT64 row, INT64 col) {
    return &mRows[row][col];
  }

  INT64 ATable::GetColumnWidth(INT64 id) const {
    auto it = mColumnW.find(id);
    return (it != mColumnW.end()) ? it->second.first : 0;
  }

  INT64 ATable::GetRowHeight(INT64 id) const {
    auto it = mRowH.find(id);
    return (it != mRowH.end()) ? it->second.first : 0;
  }

  UINT64 ATable::Rows() {
    D2()
    return mRows.size();
  }

  UINT64 ATable::Columns() {
    D3()
    return mColumns.size();
  }

  void ATable::SetRowHeaderWidth(UINT32 width) {
    D3("setting row header width=%u", width)
    mRowHeaderWidth = width;
    Draw();
  }

  void ATable::SetRowHeaderResizeEnabled(bool enable) {
    D3()
    mRowHeaderResizeEnabled = enable;
  }

  void ATable::SetRowHeightResizeEnabled(bool enable) {
    D3()
    mRowHeightResizeEnabled = enable;
  }

  void ATable::DisableRowHeader() {
    D3()
    mRowHeaderResizeEnabled = false;
    mRowHeightResizeEnabled = false;
    SetRowHeaderWidth(0);
  }

}
