#include "AUILib.h"
#include "ATable.h"

#include "defaults.h"

namespace aui {

  ATable::ATable(AWidget *wParent) {
    D2();
    AUI *cg = wParent->AUIPtr();
    Display *d = cg->Disp();
    UINT32 scr = cg->Scr();
    SetType(AUIWidgetType::defaultTable);
    SetBGColor(AUI_TABLE_BG);
    SetXY(AUI_TABLE_X, AUI_TABLE_Y);
    SetSizeXY(AUI_TABLE_SZX, AUI_TABLE_SZY);
    SetAUIPtr(cg);
    SetWndParent(wParent);
    InitWidgetProps(
        XCreateSimpleWindow(d, wParent->Wnd(), X(), Y(), SizeX(), SizeY(), 1,
            BlackPixel(d, scr), BGColor()));
    Window w = Wnd();
    XSelectInput(d, w,
    ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
    XMapWindow(d, w);
    // --- XCB CURSOR INITIALIZATION ---
    // 1. Get XCB connection from Xlib Display
    xcb_connection_t *conn = XGetXCBConnection(d);
    // 2. Create a cursor context to handle theme lookups
    xcb_cursor_context_t *ctx;
    if(xcb_cursor_context_new(conn,
        xcb_setup_roots_iterator(xcb_get_setup(conn)).data, &ctx) >= 0) {
      // 3. Load cursors by string name.
      // These names are standardized in cursor themes.
      mHorizCursor = xcb_cursor_load_cursor(ctx, "sb_h_double_arrow");
      mVertCursor = xcb_cursor_load_cursor(ctx, "sb_v_double_arrow");
      // 4. Free the context (does not destroy the loaded cursors)
      xcb_cursor_context_free(ctx);
    }
    cg->AddWidget(this);
  }

  void ATable::Draw() {
    AUI *au = AUIPtr();
    Display *d = au->Disp();
    Window w = Wnd();
    GC gc = GCPtr();
    // Optimization: Only recreate pixmap if size changed or it doesn't exist
    if(mBuffer == None) {
      mBuffer = XCreatePixmap(d, w, (UINT32) SizeX(), (UINT32) SizeY(),
          DefaultDepth(d, au->Scr()));
    }
    // Clear the background of the buffer
    XSetForeground(d, gc, BGColor());
    XFillRectangle(d, mBuffer, gc, 0, 0, (UINT32) SizeX(), (UINT32) SizeY());
    ATableRangeData1 rowStart = Offset2Row(mVOffset);
    ATableRangeData1 rowEnd = Offset2RowRange(rowStart, SizeY());
    ATableRangeData1 colStart = Offset2Column(mHOffset);
    ATableRangeData1 colEnd = Offset2ColumnRange(colStart, SizeX());
    ATableRangeData2 rRow = { rowStart.cell, rowStart.offset, rowEnd.cell,
        rowEnd.offset };
    ATableRangeData2 rCol = { colStart.cell, colStart.offset, colEnd.cell,
        colEnd.offset };
    // Draw everything to the cached buffer
    DrawCells(mBuffer, &rRow, &rCol);
    DrawRowHeader(mBuffer, rowStart, rowEnd);
    DrawColumnHeader(mBuffer, colStart, colEnd);
    DrawIntersectionBox(mBuffer);
    DrawScrollbars(mBuffer);
    // Copy to screen
    XCopyArea(d, mBuffer, w, gc, 0, 0, (UINT32) SizeX(), (UINT32) SizeY(), 0,
        0);
    XFlush(d);
  }

  ATable* ATable::AttachTo(AWidget *wParent) {
    return new ATable(wParent);
  }

  void ATable::DrawIntersectionBox(Drawable dest) {
    Display *d = AUIPtr()->Disp();
    GC gc = GCPtr();
    XSetForeground(d, gc, AUI_TABLE_INTERSEC_BG);
    XFillRectangle(d, dest, gc, 0, 0, mRowHeaderWidth, mColumnHeaderHeight);
    XSetForeground(d, gc, 0x0);
    XDrawRectangle(d, dest, gc, 0, 0, mRowHeaderWidth, mColumnHeaderHeight);
  }

  ATableRangeData1 ATable::Offset2Column(INT64 offset) {
    if(mColumnW.empty())
      return {-1, -1};
    auto lastIt = mColumnW.rbegin();
    // We iterate through the map to find which column covers the pixel offset
    for (const auto& [id, data] : mColumnW) {
      if(offset >= data.first) {
        offset -= data.first;
      } else {
        // Return the column ID and the internal pixel offset within that column
        return {id, offset};
      }
    }
    // Fallback to the last column if the offset exceeds total width
    return {lastIt->first, lastIt->second.first};
  }

  ATableRangeData1 ATable::Offset2Row(INT64 offset) {
    if(mRowH.empty())
      return {-1, -1};
    // Constant time calculation instead of loop
    INT64 rowIdx = offset / AUI_TABLE_CELL_H;
    INT64 internalOffset = offset % AUI_TABLE_CELL_H;
    // Safety check against total rows
    if(rowIdx >= (INT64) mRowH.size()) {
      rowIdx = mRowH.size() - 1;
      internalOffset = AUI_TABLE_CELL_H;
    }
    return {(INT64)rowIdx, internalOffset};
  }

  ATableRangeData1 ATable::Offset2ColumnRange(ATableRangeData1 startColumn,
  INT64 width) {
    ATableRangeData1 endc = startColumn;
    auto it = mColumnW.find(startColumn.cell);
    if(it != mColumnW.end()) {
      // Pixel width remaining to be filled in the widget area
      INT64 remainingWidth = width - mRowHeaderWidth;
      // Subtract pixels already covered by the partial first visible cell
      INT64 firstCellVisibleW = it->second.first - startColumn.offset;
      remainingWidth -= firstCellVisibleW;
      // Move to the next cell to begin the filling loop
      ++it;
      // Keep adding columns to the visible range until the widget width is covered
      while (it != mColumnW.end() && remainingWidth > 0) {
        endc.cell = it->first;
        remainingWidth -= it->second.first;
        ++it;
      }
      // Clamp: Ensure the range is at least the start cell
      if(endc.cell < startColumn.cell)
        endc.cell = startColumn.cell;
    }
    return endc;
  }

  ATableRangeData1 ATable::Offset2RowRange(ATableRangeData1 startRow,
  INT64 height) {
    ATableRangeData1 endr = startRow;
    // Calculate how many rows can fit in the height
    // We add 1 to ensure we cover the partial row at the bottom
    INT64 rowsVisible = (height / AUI_TABLE_CELL_H) + 1;
    endr.cell = startRow.cell + rowsVisible;
    // Clamp to actual row count
    if(endr.cell >= (INT64) mRowH.size()) {
      endr.cell = mRowH.size() - 1;
    }
    return endr;
  }

  void ATable::DrawColumnHeader(Drawable dest, ATableRangeData1 colStartData,
      ATableRangeData1 colEndData) {
    Display *d = AUIPtr()->Disp();
    GC gc = GCPtr();
    XFontStruct *font_info = Font();
    INT64 x_pos = (INT64) mRowHeaderWidth - colStartData.offset;
    // Iterate up to endData.cell + 1 to ensure we don't truncate the last visible column
    for (INT64 i = colStartData.cell; i <= colEndData.cell; i++) {
      INT64 currentW = GetColumnWidth(i);
      // Change: Draw if any part of the column is within [mRowHeaderWidth, SizeX()]
      if(currentW > 0 && x_pos < SizeX()
          && x_pos + currentW > mRowHeaderWidth) {
        // Header BG
        XSetForeground(d, gc, 0xCCCCCC);
        XFillRectangle(d, dest, gc, (INT32) x_pos, 0, (UINT32) currentW,
            (UINT32) mColumnHeaderHeight);
        // Border
        XSetForeground(d, gc, 0x000000);
        XDrawRectangle(d, dest, gc, (INT32) x_pos, 0, (UINT32) currentW,
            (UINT32) mColumnHeaderHeight);
        // Text with clipping to prevent spill-over
        std::string label = mColumnW[i].second;
        int text_width =
            font_info ?
                XTextWidth(font_info, label.c_str(), (INT32) label.length()) :
                (INT32) label.length() * 6;
        INT32 text_x = (INT32) (x_pos + (currentW - text_width) / 2);
        if(x_pos + currentW > mRowHeaderWidth) {
          XRectangle clip = { (INT32) std::max((INT64) mRowHeaderWidth, x_pos),
              0, (UINT32) currentW, (UINT32) mColumnHeaderHeight };
          XSetClipRectangles(d, gc, 0, 0, &clip, 1, Unsorted);
          XDrawString(d, dest, gc, text_x,
              (INT32) ((mColumnHeaderHeight / 2) + 4), label.c_str(),
              (INT32) label.length());
          XSetClipMask(d, gc, None);
        }
      }
      x_pos += currentW;
      // Safety break ONLY if we are significantly past the edge
      if(x_pos > SizeX() + 100)
        break;
    }
  }

  void ATable::DrawRowHeader(Drawable dest, ATableRangeData1 rowStartData,
      ATableRangeData1 rowEndData) {
    Display *d = AUIPtr()->Disp();
    GC gc = GCPtr();
    XFontStruct *font_info = Font();
    INT64 y_pos = (INT64) mColumnHeaderHeight - rowStartData.offset;
    for (INT64 i = rowStartData.cell; i <= rowEndData.cell; i++) {
      INT64 currentH = GetRowHeight(i);
      if(currentH > 0 && y_pos + currentH > mColumnHeaderHeight
          && y_pos < SizeY()) {
        XSetForeground(d, gc, 0xCCCCCC);
        XFillRectangle(d, dest, gc, 0, (INT32) y_pos, (UINT32) mRowHeaderWidth,
            (UINT32) currentH);
        XSetForeground(d, gc, 0x000000);
        XDrawRectangle(d, dest, gc, 0, (INT32) y_pos, (UINT32) mRowHeaderWidth,
            (UINT32) currentH);
        std::string label = mRowH[i].second;
        int text_w =
            font_info ?
                XTextWidth(font_info, label.c_str(), label.length()) :
                (INT32) label.length() * 6;
        XDrawString(d, dest, gc, (INT32) (mRowHeaderWidth - text_w - 4),
            (INT32) (y_pos + (currentH / 2) + 4), label.c_str(),
            label.length());
      }
      y_pos += currentH;
      if(y_pos >= SizeY())
        break;
    }
  }

  void ATable::AddColumn() {
    UINT64 lastK = 0;
    if(mColumns.size() > 0) lastK = mColumns.rbegin()->first + 1;
    mColumns[lastK];
    AUI *cg = AUIPtr();
    if(!mColumnW.contains(lastK)) {
      std::string name = cg->NumberToBaseString(lastK);
      mColumnW[lastK] = { AUI_TABLE_CELL_W, name };
      mTotalContentWidth += AUI_TABLE_CELL_W;
      if(mAutoWiden) {
        XFontStruct *font_info = Font();
        int textW = font_info ? XTextWidth(font_info, name.c_str(), (int)name.length()) : (int)name.length() * 7;
        if((INT64)textW + 20 > AUI_TABLE_CELL_W) {
          INT64 newW = (INT64)textW + 20;
          mTotalContentWidth += (newW - AUI_TABLE_CELL_W);
          mColumnW[lastK].first = newW;
        }
      }
    } else {
      E("Column width info integrity error")
    }
  }
  void ATable::AddRow() {
    UINT64 lastK = 0;
    if(mRows.size() > 0)
      lastK = mRows.rbegin()->first + 1;
    mRows[lastK];
    if(!mRowH.contains(lastK)) {
      std::string label = std::to_string(lastK);
      mRowH[lastK] = { AUI_TABLE_CELL_H, label };
      mTotalContentHeight += AUI_TABLE_CELL_H;
      // Optimization: Only re-calculate header width if the string length increased
      static size_t lastLen = 0;
      if(label.length() > lastLen) {
        XFontStruct *font_info = Font();
        int text_width =
            font_info ?
                XTextWidth(font_info, label.c_str(), label.length()) :
                label.length() * 7;
        mRowHeaderWidth = text_width + 12;
        lastLen = label.length();
      }
    }
  }

  void ATable::ScrollUpPx(INT64 px) {
    mVOffset -= px;
    if(mVOffset < 0)
      mVOffset = 0;
    Draw();
  }

  void ATable::ScrollDownPx(INT64 px) {
    // Prevent scrolling beyond the last row
    INT64 maxScroll = mTotalContentHeight - (SizeY() - mColumnHeaderHeight);
    if(maxScroll < 0)
      maxScroll = 0;
    mVOffset += px;
    if(mVOffset > maxScroll)
      mVOffset = maxScroll;
    Draw();
  }

  void ATable::ScrollLeftPx(INT64 px) {
    mHOffset -= px;
    if(mHOffset < 0)
      mHOffset = 0;
    Draw();
  }

  void ATable::ScrollRightPx(INT64 px) {
    // Prevent scrolling beyond the last column
    INT64 maxScroll = mTotalContentWidth - (SizeX() - mRowHeaderWidth);
    if(maxScroll < 0)
      maxScroll = 0;
    mHOffset += px;
    if(mHOffset > maxScroll)
      mHOffset = maxScroll;
    Draw();
  }

  UINT64 ATable::Rows() {
    D2()
    return mRows.size();
  }

  UINT64 ATable::Columns() {
    D2()
    return mColumns.size();
  }

  void ATable::Insert(INT64 row, INT64 col, AUICellData *cell) {
    if(!cell) return;
    mRows[row][col] = std::move(*cell);
    mColumns[col][row] = &mRows[row][col];
    if(mAutoWiden) {
      XFontStruct *font_info = Font();
      int textW = font_info ? XTextWidth(font_info, mRows[row][col].data.c_str(), (int)mRows[row][col].data.length()) : (int)mRows[row][col].data.length() * 7;
      int padding = 15;
      if((INT64)textW + padding > GetColumnWidth(col)) {
        INT64 newW = (INT64)textW + padding;
        mTotalContentWidth += (newW - mColumnW[col].first);
        mColumnW[col].first = newW;
      }
    }
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

  void ATable::DrawCells(Drawable dest, ATableRangeData2 *rowRange,
      ATableRangeData2 *colRange) {
    AUI *au = AUIPtr();
    Display *d = au->Disp();
    GC gc = GCPtr();
    XFontStruct *font_info = Font();
    // 1. Bulk fill base background
    XSetForeground(d, gc, 0xFFFFFF);
    XFillRectangle(d, dest, gc, mRowHeaderWidth, mColumnHeaderHeight,
        (UINT32) (SizeX() - mRowHeaderWidth),
        (UINT32) (SizeY() - mColumnHeaderHeight));
    INT64 y_pos = (INT64) mColumnHeaderHeight - rowRange->offset;
    for (INT64 r = rowRange->cell; r <= rowRange->cell2; ++r) {
      INT64 currentH = GetRowHeight(r);
      if(currentH <= 0 || y_pos >= SizeY()) {
        y_pos += currentH;
        continue;
      }
      // 2. Row Highlight Mode
      if(mRowSelectMode && r == mSelectedRow) {
        XSetForeground(d, gc, 0xE8F2FF); // Light Selection Blue
        XFillRectangle(d, dest, gc, mRowHeaderWidth, (INT32) y_pos,
            (UINT32) (SizeX() - mRowHeaderWidth), (UINT32) currentH);
      }
      INT64 x_pos = (INT64) mRowHeaderWidth - colRange->offset;
      for (INT64 c = colRange->cell; c <= colRange->cell2; ++c) {
        INT64 currentW = GetColumnWidth(c);
        if(currentW > 0 && x_pos < SizeX()
            && x_pos + currentW > mRowHeaderWidth) {
          // 3. Grid Lines
          XSetForeground(d, gc, 0xDDDDDD);
          XDrawLine(d, dest, gc, (INT32) (x_pos + currentW - 1), (INT32) y_pos,
              (INT32) (x_pos + currentW - 1), (INT32) (y_pos + currentH - 1));
          XDrawLine(d, dest, gc, (INT32) x_pos, (INT32) (y_pos + currentH - 1),
              (INT32) (x_pos + currentW - 1), (INT32) (y_pos + currentH - 1));
          // 4. Content Alignment
          auto itRow = mRows.find(r);
          if(itRow != mRows.end()) {
            auto itCol = itRow->second.find(c);
            if(itCol != itRow->second.end()) {
              AUICellData &cell = itCol->second;
              if(!cell.data.empty()) {
                INT32 textW =
                    font_info ?
                        XTextWidth(font_info, cell.data.c_str(),
                            (INT32) cell.data.length()) :
                        (INT32) cell.data.length() * 7;
                INT32 fontAscent = font_info ? font_info->ascent : 12;
                INT32 fontDescent = font_info ? font_info->descent : 3;
                INT32 textH = fontAscent + fontDescent;
                INT32 tx = (INT32) (x_pos + 4);
                if(cell.hal == AUIHAlign::center)
                  tx = (INT32) (x_pos + (currentW - textW) / 2);
                else if(cell.hal == AUIHAlign::right)
                  tx = (INT32) (x_pos + currentW - textW - 4);
                INT32 ty = (INT32) (y_pos + fontAscent + 2);
                if(cell.val == AUIVAlign::center)
                  ty = (INT32) (y_pos + (currentH - textH) / 2 + fontAscent);
                else if(cell.val == AUIVAlign::bottom)
                  ty = (INT32) (y_pos + currentH - fontDescent - 2);
                INT32 clipX = (INT32) std::max((INT64) mRowHeaderWidth, x_pos);
                XRectangle clip = { clipX, (INT32) y_pos, (UINT32) (x_pos
                    + currentW - clipX), (UINT32) currentH };
                XSetClipRectangles(d, gc, 0, 0, &clip, 1, Unsorted);
                XSetForeground(d, gc, 0x000000);
                XDrawString(d, dest, gc, tx, ty, cell.data.c_str(),
                    (INT32) cell.data.length());
                XSetClipMask(d, gc, None);
              }
            }
          }
          // 5. Cursor with logic to prevent disappearing on left overlap
          if(r == mCursorRow && c == mCursorCol) {
            XSetForeground(d, gc, 0x3399FF);
            XSetLineAttributes(d, gc, 2, LineSolid, CapButt, JoinMiter);
            INT32 clipX = (INT32) std::max((INT64) mRowHeaderWidth, x_pos);
            INT32 clipY = (INT32) std::max((INT64) mColumnHeaderHeight, y_pos);
            UINT32 clipW = (UINT32) (x_pos + currentW - clipX);
            UINT32 clipH = (UINT32) (y_pos + currentH - clipY);
            if(x_pos + currentW > mRowHeaderWidth
                && y_pos + currentH > mColumnHeaderHeight) {
              XRectangle clip = { clipX, clipY, clipW, clipH };
              XSetClipRectangles(d, gc, 0, 0, &clip, 1, Unsorted);
              XDrawRectangle(d, dest, gc, (INT32) x_pos, (INT32) y_pos,
                  (UINT32) currentW - 1, (UINT32) currentH - 1);
              XSetClipMask(d, gc, None);
            }
            XSetLineAttributes(d, gc, 1, LineSolid, CapButt, JoinMiter);
          }
        }
        x_pos += currentW;
      }
      y_pos += currentH;
      if(y_pos >= SizeY())
        break;
    }
  }

  void ATable::OnButtonPress(XEvent *ev) {
    int x = ev->xbutton.x;
    int y = ev->xbutton.y;
    unsigned int button = ev->xbutton.button;
    if(button == Button4) { ScrollUpPx(60); return; }
    if(button == Button5) { ScrollDownPx(60); return; }
    if(button == Button1) {
      if(x >= SizeX() - AUI_TABLE_SCROLL_THICK && y > mColumnHeaderHeight) {
        double viewH = (double) SizeY() - mColumnHeaderHeight - AUI_TABLE_SCROLL_THICK;
        if(mTotalContentHeight > viewH) {
          mCurrentScrollMode = AUIScrollMode::AUIVertical;
          double thumbH = std::max(20.0, (viewH / (double) mTotalContentHeight) * viewH);
          double maxScroll = (double) mTotalContentHeight - viewH;
          double travelTrack = viewH - thumbH;
          int vPos = mColumnHeaderHeight + (int) (((double) mVOffset / maxScroll) * travelTrack);
          if(y >= vPos && y <= vPos + thumbH) mScrollGrabOffset = y - vPos;
          else mScrollGrabOffset = (int) (thumbH / 2.0);
          OnMouseMove(ev);
        }
        return;
      }
      if(y >= SizeY() - AUI_TABLE_SCROLL_THICK && x > mRowHeaderWidth) {
        double viewW = (double) SizeX() - mRowHeaderWidth - AUI_TABLE_SCROLL_THICK;
        if(mTotalContentWidth > viewW) {
          mCurrentScrollMode = AUIScrollMode::AUIHorizontal;
          double thumbW = std::max(20.0, (viewW / (double) mTotalContentWidth) * viewW);
          double maxScroll = (double) mTotalContentWidth - viewW;
          double travelTrack = viewW - thumbW;
          int hPos = mRowHeaderWidth + (int) (((double) mHOffset / maxScroll) * travelTrack);
          if(x >= hPos && x <= hPos + thumbW) mScrollGrabOffset = x - hPos;
          else mScrollGrabOffset = (int) (thumbW / 2.0);
          OnMouseMove(ev);
        }
        return;
      }
      if(std::abs(x - (int)mRowHeaderWidth) < 5) {
        mResizeMode = AUIResizeMode::AUIHeader;
        mResizeBasePos = x;
        mResizeBaseSize = mRowHeaderWidth;
        return;
      }
      if(mAllowColumnResize && y < mColumnHeaderHeight && x > mRowHeaderWidth) {
        ATableRangeData1 colStart = Offset2Column(mHOffset);
        INT64 currX = (INT64) mRowHeaderWidth - colStart.offset;
        for (INT64 i = colStart.cell; i <= Offset2ColumnRange(colStart, SizeX()).cell; i++) {
          currX += GetColumnWidth(i);
          if(std::abs(x - (int) currX) < 5) {
            mResizeMode = AUIResizeMode::AUIColumn;
            mResizeId = i;
            mResizeBasePos = x;
            mResizeBaseSize = GetColumnWidth(i);
            return;
          }
        }
      }
      if(mAllowRowResize && x < mRowHeaderWidth && y > mColumnHeaderHeight) {
        ATableRangeData1 rowStart = Offset2Row(mVOffset);
        INT64 currY = (INT64) mColumnHeaderHeight - rowStart.offset;
        for (INT64 i = rowStart.cell; i <= Offset2RowRange(rowStart, SizeY()).cell; i++) {
          currY += GetRowHeight(i);
          if(std::abs(y - (int) currY) < 5) {
            mResizeMode = AUIResizeMode::AUIRow;
            mResizeId = i;
            mResizeBasePos = y;
            mResizeBaseSize = GetRowHeight(i);
            return;
          }
        }
      }
      auto [r, c] = ScreenToCell(x, y);
      if(r != -1 && c != -1) {
        mCursorRow = r;
        mCursorCol = c;
        if(mRowSelectMode) mSelectedRow = r;
        Draw();
      }
    }
  }

  void ATable::OnButtonRelease(XEvent *ev) {
    mCurrentScrollMode = AUIScrollMode::AUINone;
    mResizeMode = AUIResizeMode::AUINone;
    mResizeId = -1;
  }

  std::pair<INT64, INT64> ATable::ScreenToCell(INT32 x, INT32 y) {
    INT64 targetRow = -1, targetCol = -1;
    // Map Y to Row
    INT64 currY = mColumnHeaderHeight - mVOffset;
    for (auto const& [id, data] : mRowH) {
      if(y >= currY && y < currY + data.first) {
        targetRow = id;
        break;
      }
      currY += data.first;
    }
    // Map X to Column
    INT64 currX = mRowHeaderWidth - mHOffset;
    for (auto const& [id, data] : mColumnW) {
      if(x >= currX && x < currX + data.first) {
        targetCol = id;
        break;
      }
      currX += data.first;
    }
    return {targetRow, targetCol};
  }

  void ATable::DrawScrollbars(Drawable dest) {
    Display *d = AUIPtr()->Disp();
    GC gc = GCPtr();
    // Using a slightly lighter grey for the track and darker for the thumb
    // or just the thumb as per your current style.
    XSetForeground(d, gc, 0x555555);
    // Vertical Scrollbar
    double viewH =
        (double) SizeY() - mColumnHeaderHeight - AUI_TABLE_SCROLL_THICK;
    if(mTotalContentHeight > viewH) {
      double ratio = viewH / (double) mTotalContentHeight;
      double thumbH = std::max(20.0, viewH * ratio);
      double maxScroll = (double) mTotalContentHeight - viewH;
      double travelTrack = viewH - thumbH;
      int vPos = mColumnHeaderHeight
          + (INT32) (((double) mVOffset / maxScroll) * travelTrack);
      // Draw the thumb at the right edge with defined thickness
      XFillRectangle(d, dest, gc, SizeX() - AUI_TABLE_SCROLL_THICK, vPos,
      AUI_TABLE_SCROLL_THICK, (UINT32) thumbH);
    }
    // Horizontal Scrollbar
    double viewW = (double) SizeX() - mRowHeaderWidth - AUI_TABLE_SCROLL_THICK;
    if(mTotalContentWidth > viewW) {
      double ratio = viewW / (double) mTotalContentWidth;
      double thumbW = std::max(20.0, viewW * ratio);
      double maxScroll = (double) mTotalContentWidth - viewW;
      double travelTrack = viewW - thumbW;
      int hPos = mRowHeaderWidth
          + (INT32) (((double) mHOffset / maxScroll) * travelTrack);
      // Draw the thumb at the bottom edge with defined thickness
      XFillRectangle(d, dest, gc, hPos, SizeY() - AUI_TABLE_SCROLL_THICK,
          (UINT32) thumbW,
          AUI_TABLE_SCROLL_THICK);
    }
  }

  void ATable::OnMouseMove(XEvent *ev) {
    Display *d = AUIPtr()->Disp();
    Window w = Wnd();
    int x = ev->xmotion.x;
    int y = ev->xmotion.y;
    // 1. ACTIVE RESIZING
    if(mResizeMode == AUIResizeMode::AUIColumn) {
      int delta = x - mResizeBasePos;
      INT64 newW = std::max((INT64) 30, mResizeBaseSize + delta);
      mTotalContentWidth += (newW - mColumnW[mResizeId].first);
      mColumnW[mResizeId].first = newW;
      // Elastic pull-back
      INT64 viewW = (INT64) SizeX() - mRowHeaderWidth - AUI_TABLE_SCROLL_THICK;
      INT64 maxScroll = std::max((INT64) 0, mTotalContentWidth - viewW);
      if(mHOffset > maxScroll)
        mHOffset = maxScroll;
      Draw();
      return;
    }
    if(mResizeMode == AUIResizeMode::AUIRow) {
      int delta = y - mResizeBasePos;
      INT64 newH = std::max((INT64) 15, mResizeBaseSize + delta);
      mTotalContentHeight += (newH - mRowH[mResizeId].first);
      mRowH[mResizeId].first = newH;
      // Elastic pull-back
      INT64 viewH =
          (INT64) SizeY() - mColumnHeaderHeight - AUI_TABLE_SCROLL_THICK;
      INT64 maxScroll = std::max((INT64) 0, mTotalContentHeight - viewH);
      if(mVOffset > maxScroll)
        mVOffset = maxScroll;
      Draw();
      return;
    }
    if(mResizeMode == AUIResizeMode::AUIHeader) {
      int delta = x - mResizeBasePos;
      mRowHeaderWidth = std::max((INT32)20, (INT32)(mResizeBaseSize + delta));
      Draw();
      return;
    }
    // 2. SCROLLING
    if(mCurrentScrollMode == AUIScrollMode::AUIVertical) {
      double viewH =
          (double) SizeY() - mColumnHeaderHeight - AUI_TABLE_SCROLL_THICK;
      double thumbH = std::max(20.0,
          (viewH / (double) mTotalContentHeight) * viewH);
      double travelTrack = viewH - thumbH;
      double maxScroll = (double) mTotalContentHeight - viewH;

      if(travelTrack > 0) {
        double trackY = (double) y - mColumnHeaderHeight - mScrollGrabOffset;
        double ratio = trackY / travelTrack;
        mVOffset = (INT64) (std::max(0.0, std::min(1.0, ratio)) * maxScroll);
        Draw();
      }
      return;
    }
    if(mCurrentScrollMode == AUIScrollMode::AUIHorizontal) {
      double viewW = (double) SizeX() - mRowHeaderWidth - AUI_TABLE_SCROLL_THICK;
      double thumbW = std::max(20.0,
          (viewW / (double) mTotalContentWidth) * viewW);
      double travelTrack = viewW - thumbW;
      double maxScroll = (double) mTotalContentWidth - viewW;

      if(travelTrack > 0) {
        double trackX = (double) x - mRowHeaderWidth - mScrollGrabOffset;
        double ratio = trackX / travelTrack;
        mHOffset = (INT64) (std::max(0.0, std::min(1.0, ratio)) * maxScroll);
        Draw();
      }
      return;
    }

    // 3. HOVER CURSORS
    bool cursorSet = false;
    if(y < mColumnHeaderHeight && x > mRowHeaderWidth) {
      ATableRangeData1 colStart = Offset2Column(mHOffset);
      INT64 currX = (INT64) mRowHeaderWidth - colStart.offset;
      for (auto const& [id, data] : mColumnW) {
        if(id < colStart.cell)
          continue;
        currX += data.first;
        if(std::abs(x - (int) currX) < 5) {
          XDefineCursor(d, w, mHorizCursor);
          cursorSet = true;
          break;
        }
        if(currX > SizeX())
          break;
      }
    } else if(x < mRowHeaderWidth && y > mColumnHeaderHeight) {
      ATableRangeData1 rowStart = Offset2Row(mVOffset);
      INT64 currY = (INT64) mColumnHeaderHeight - rowStart.offset;
      for (auto const& [id, data] : mRowH) {
        if(id < rowStart.cell)
          continue;
        currY += data.first;
        if(std::abs(y - (int) currY) < 5) {
          XDefineCursor(d, w, mVertCursor);
          cursorSet = true;
          break;
        }
        if(currY > SizeY())
          break;
      }
    }
    if(std::abs(x - (int)mRowHeaderWidth) < 5) {
      XDefineCursor(d, w, mHorizCursor);
      cursorSet = true;
    }
    if(!cursorSet)
      XUndefineCursor(d, w);
  }

  void ATable::RemoveRow(INT64 rowIdx) {
    // 1. Cleanup data and pointers
    // We must iterate through the columns in this row to remove pointers in mColumns
    auto itRow = mRows.find(rowIdx);
    if(itRow != mRows.end()) {
      for (auto& [colIdx, cellData] : itRow->second) {
        mColumns[colIdx].erase(rowIdx);
        // If the column is now completely empty, we keep the column structure
        // but it has no row entries.
      }
      mRows.erase(itRow);
    }
    // 2. Update height cache and dimension map
    auto itH = mRowH.find(rowIdx);
    if(itH != mRowH.end()) {
      mTotalContentHeight -= itH->second.first;
      mRowH.erase(itH);
    }
    // 3. Reset selection/cursor if they pointed to the deleted row
    if(mSelectedRow == rowIdx)
      mSelectedRow = -1;
    if(mCursorRow == rowIdx)
      mCursorRow = -1;
    // 4. Reposition View: Clamp vOffset so we don't show empty space at the bottom
    INT64 viewH = (INT64) SizeY() - mColumnHeaderHeight;
    INT64 maxScroll = mTotalContentHeight - viewH;
    if(maxScroll < 0)
      maxScroll = 0;
    if(mVOffset > maxScroll)
      mVOffset = maxScroll;
    Draw();
  }

  void ATable::RemoveColumn(INT64 colIdx) {
    // 1. Clear data from row-major storage
    for (auto& [rowIdx, rowData] : mRows) {
      rowData.erase(colIdx);
    }
    // 2. Clear from column-major pointer map
    mColumns.erase(colIdx);
    // 3. Update width cache and dimension map
    auto itW = mColumnW.find(colIdx);
    if(itW != mColumnW.end()) {
      mTotalContentWidth -= itW->second.first;
      mColumnW.erase(itW);
    }
    // 4. Reset cursor if it was on the removed column
    if(mCursorCol == colIdx)
      mCursorCol = -1;
    // 5. Elastic Pull-back: Ensure no empty space to the right
    INT64 viewW = (INT64) SizeX() - mRowHeaderWidth;
    INT64 maxScroll = mTotalContentWidth - viewW;
    if(maxScroll < 0)
      maxScroll = 0;
    if(mHOffset > maxScroll)
      mHOffset = maxScroll;
    Draw();
  }

  void ATable::RemoveLastRow() {
    if(mRowH.empty())
      return;
    // std::map is sorted, so rbegin() is the highest index
    RemoveRow(mRowH.rbegin()->first);
  }

  void ATable::RemoveLastColumn() {
    if(mColumnW.empty())
      return;
    // std::map is sorted, so rbegin() is the highest index
    RemoveColumn(mColumnW.rbegin()->first);
  }

  void ATable::PrintDebugState() {
    D1("ATable Debug State");
    D1("Widget Size: %dx%d", SizeX(), SizeY());
    D1("Content Size: TotalW=%ld, TotalH=%ld", mTotalContentWidth,
        mTotalContentHeight);
    D1("Offsets: HOffset=%ld, VOffset=%ld", mHOffset, mVOffset);
    D1("Headers: RowW=%d, ColH=%d", mRowHeaderWidth, mColumnHeaderHeight);
    D1("Scroll Mode: %d", (INT32)mCurrentScrollMode);
    D1("--------------------------");
  }

  void ATable::SetColumnName(INT64 colIdx, std::string &name) {
    // 1. Locate the column in the width/label map
    auto it = mColumnW.find(colIdx);
    if(it != mColumnW.end()) {
      // 2. Update the label string (stored as the second element of the pair)
      it->second.second = name;
      // 3. Optional: Logic to automatically expand column width if name is too long
      // This ensures the header text isn't immediately clipped
      XFontStruct *font_info = Font();
      int text_width =
          font_info ?
              XTextWidth(font_info, name.c_str(), (int) name.length()) :
              (int) name.length() * 7;
      // Add padding (e.g., 20px) and check if it's larger than current width
      if((INT64) text_width + 20 > it->second.first) {
        INT64 newW = (INT64) text_width + 20;
        mTotalContentWidth += (newW - it->second.first);
        it->second.first = newW;
      }
      // 4. Trigger a refresh to show the new name
      Draw();
    } else {
      // If the column doesn't exist in the map yet, you could
      // choose to ignore or log an error.
      E("Attempted to set name for non-existent column: %ld", colIdx);
    }
  }

  void ATable::SetRowName(INT64 rowIdx, const std::string &name) {
    auto it = mRowH.find(rowIdx);
    if(it != mRowH.end()) {
      it->second.second = name;
      XFontStruct *font_info = Font();
      int text_width =
          font_info ?
              XTextWidth(font_info, name.c_str(), (int) name.length()) :
              (int) name.length() * 7;
      if(text_width + 12 > mRowHeaderWidth) {
        mRowHeaderWidth = text_width + 12;
      }
      Draw();
    } else {
      E("Attempted to set name for non-existent row: %ld", rowIdx);
    }
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

  std::string ATable::GetRowName(INT64 rowIdx) {
    auto it = mRowH.find(rowIdx);
    if(it != mRowH.end())
      return it->second.second;
    return "";
  }

  std::string ATable::GetColumnName(INT64 colIdx) {
    auto it = mColumnW.find(colIdx);
    if(it != mColumnW.end())
      return it->second.second;
    return "";
  }

  ATable::~ATable() {
    AUI *au = AUIPtr();
    Display *d = au->Disp();
    // Cleanup X11 Pixmap
    if(mBuffer != None) {
      XFreePixmap(d, mBuffer);
      mBuffer = None;
    }
    // XCB CURSOR CLEANUP
    // We must use xcb_free_cursor with the XCB connection
    xcb_connection_t *conn = XGetXCBConnection(d);
    if(mHorizCursor != 0) {
      D2("freeing horizontal cursor via xcb")
      xcb_free_cursor(conn, (xcb_cursor_t) mHorizCursor);
      mHorizCursor = 0;
    }
    if(mVertCursor != 0) {
      D2("freeing vertical cursor via xcb")
      xcb_free_cursor(conn, (xcb_cursor_t) mVertCursor);
      mVertCursor = 0;
    }
  }

  void ATable::UpdateColumnWidthToFit(INT64 colIdx) {
    auto itCol = mColumns.find(colIdx);
    if(itCol == mColumns.end()) return;
    XFontStruct *font_info = Font();
    INT64 maxW = 0;
    for(auto& [rowIdx, cellPtr] : itCol->second) {
      if(cellPtr) {
        INT32 textW = font_info ? XTextWidth(font_info, cellPtr->data.c_str(), (int)cellPtr->data.length()) : (int)cellPtr->data.length() * 7;
        if(textW > maxW) maxW = textW;
      }
    }
    INT64 finalW = maxW + 15;
    if(finalW > mColumnW[colIdx].first) {
      mTotalContentWidth += (finalW - mColumnW[colIdx].first);
      mColumnW[colIdx].first = finalW;
      Draw();
    }
  }

  void ATable::SetAutoWiden(bool enable) {
    D1()
    mAutoWiden = enable;
  }

}
