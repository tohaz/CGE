#ifndef ATABLE_H_
#define ATABLE_H_

#include "AWidget.h"
#include <map>
#include <string>
#include <X11/cursorfont.h>
#include <xcb/xcb_cursor.h>
#include <X11/Xlib-xcb.h>

namespace aui {
  typedef struct {
    INT64 cell = -1;
    INT64 offset = -1;
  }ATableRangeData1;

  typedef struct {
    INT64 cell = -1;
    INT64 offset = -1;
    INT64 cell2 = -1;
    INT64 offset2 = -1;
  } ATableRangeData2;

  typedef struct {
    std::string data = "";
    AUIHAlign hAlign = AUIHAlign::center;
    AUIVAlign vAlign = AUIVAlign::center;
  } AUICellData;

//  typedef1 struct {
//    std::string name;
//  } AUIColumnData;

  enum class AUIScrollMode{AUINone, AUIVertical, AUIHorizontal};
  enum class AUIResizeMode {AUINone, AUIColumn, AUIRow, AUIHeader};

  class ATable : public AWidget {
    private:
      ATable(AWidget* wParent);
      void DrawColumnHeader(Drawable dest, ATableRangeData1, ATableRangeData1);
      void DrawRowHeader(Drawable dest, ATableRangeData1 rowStartData, ATableRangeData1 rowEndData);
      void DrawCells(Drawable dest, ATableRangeData2 *rowRange, ATableRangeData2 *colRange);
      void DrawIntersectionBox(Drawable dest);
      void DrawScrollbars(Drawable dest);
      ATableRangeData1 Offset2Column(INT64 offset);
      ATableRangeData1 Offset2Row(INT64 offset);
      ATableRangeData1 Offset2ColumnRange(ATableRangeData1 start, INT64 offset);
      ATableRangeData1 Offset2RowRange(ATableRangeData1 start, INT64 offset);
      std::pair<INT64, INT64> ScreenToCell(INT32 x, INT32 y);
      void PrintDebugState();
      // this is data
      std::map<INT64, std::map<INT64, AUICellData> > mRows;
      std::map<INT64, std::map<INT64, AUICellData*> > mColumns;
      // this is rows and colums descriptors
      std::map<INT64, std::pair<INT64, std::string> > mRowH;
      std::map<INT64, std::pair<INT64, std::string> > mColumnW;
      //
      INT64 mHOffset = 0;
      INT64 mVOffset = 0;
      UINT32 mColumnHeaderHeight = 20;
      UINT32 mRowHeaderWidth = 50;
      INT64 mCursorRow = 0;
      INT64 mCursorCol = 0;
      bool mIsDragging = false;
      AUIScrollMode mCurrentScrollMode = AUIScrollMode::AUINone;
      int mLastMouseY = 0;
      int mLastMouseX = 0;
      INT64 mTotalContentWidth = 0;
      INT64 mTotalContentHeight = 0;
      double mGrabOffset = 0; // Store the relative click point inside the thumb
      INT64 mScrollGrabOffset = 0;
      bool mRowSelectMode = true; // Toggle for the mode
      INT64 mSelectedRow = -1;    // Track the clicked row
      AUIResizeMode mResizeMode = AUIResizeMode::AUINone;
      INT64 mResizeId = -1;
      int mResizeBasePos = 0;
      INT64 mResizeBaseSize = 0;
      Cursor mHorizCursor = None;
      Cursor mVertCursor = None;
      bool mAutoWiden = false;
      bool mAllowColumnResize = true;
      bool mAllowRowResize = true;
      bool mRowHeaderResizeEnabled = true;
      bool mRowHeightResizeEnabled = true;
    public:
      virtual ~ATable();
      static  ATable* AttachTo(AWidget* wParent);
      void AddColumn();
      void AddColumns(UINT32 number);
      std::string ColumnName(INT64 colIdx);
      UINT64 Columns();
      void AddRows(UINT32 number);
      INT64 AddRow();
      void Clear();
      void DisableRowHeader();
      void Draw();
      AUICellData* Get(INT64 row, INT64 col);
      INT64 GetColumnWidth(INT64 id) const;
      INT64 GetRowHeight(INT64 id) const;
      void Insert(INT64 row, INT64 col, AUICellData* cell);
      void RemoveColumn(INT64 colIdx);
      void RemoveRow(INT64 rowIdx);
      void RemoveLastRow();
      void RemoveLastColumn();
      UINT64 Rows();
      std::string RowName(INT64 rowIdx);
      void SetAutoWiden(bool enable);
      void SetColumnName(INT64 colIdx, std::string name);
      void SetColumnWidth(INT64 colIdx, INT64 width);
      void SetRowName(INT64 rowIdx, const std::string name);
      void ScrollDownPx(INT64 px);
      void ScrollUpPx(INT64 px);
      void ScrollLeftPx(INT64 px);
      void ScrollRightPx(INT64 px);
      void SetRowHeaderWidth(UINT32 width);
      void SetRowHeaderResizeEnabled(bool enable);
      void SetRowHeightResizeEnabled(bool enable);
      void UpdateColumnWidthToFit(INT64 colIdx);
      void OnButtonPress(XEvent *ev);
      void OnButtonRelease(XEvent *ev);
      void OnMouseMove(XEvent *ev);
      void SetCursorPosition(INT64 row, INT64 column);
      INT64 CursorRow();
      INT64 CursorColumn();
      std::string DataAtCursor();
      std::string DataAt(INT64 row, INT64 col);
  };
}

#endif
