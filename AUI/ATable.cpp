#include "AUILib.h"
#include "ATable.h"

#include "defaults.h"

namespace aui {

  ATable::ATable(AWidget *wParent) {
    D2();
    AUI *cg = wParent->AUIPtr();
    Display *d = cg->Disp();
    INT32 scr = cg->Scr();
    SetType(AUIWidgetType::defaultTable);
    SetBGColor(AUI_TABLE_BG);
    SetXY(AUI_TABLE_X, AUI_TABLE_Y);
    SetSizeXY(AUI_TABLE_SZX, AUI_TABLE_SZY);
    SetAUIPtr(cg);
    SetWndParent(wParent);
    SetBB(None);
    InitWidgetProps(
        XCreateSimpleWindow(d, wParent->Wnd(), SafeINT32(X()), SafeINT32(Y()),
            SafeUINT32(SizeX()), SafeUINT32(SizeY()), 1, BlackPixel(d, scr),
            BGColor()));
    Window w = Wnd();
    XSelectInput(d, w,
    ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
    XMapWindow(d, w);
    xcb_connection_t *conn = XGetXCBConnection(d);
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
    if(Wnd() == 0)
      return;
    AUI *au = AUIPtr();
    GC gc = GCPtr();
    Display *d = au->Disp();
    // 1. Calculate column ranges
    ATableRangeData1 colStart = Offset2Column(mHOffset);
    ATableRangeData1 colEnd = Offset2ColumnRange(colStart, (INT64) SizeX());
    // 2. Calculate row ranges
    ATableRangeData1 rowStart = Offset2Row(mVOffset);
    ATableRangeData1 rowEnd = Offset2RowRange(rowStart, (INT64) SizeY());
    // 3. Populate ATableRangeData2 explicitly to avoid uninitialized jumps
    ATableRangeData2 colRange = { colStart.cell, colStart.offset, colEnd.cell,
        colEnd.offset };
    ATableRangeData2 rowRange = { rowStart.cell, rowStart.offset, rowEnd.cell,
        rowEnd.offset };
    // 4. Update the BackBuffer if needed
    UpdateBuffer();
    Drawable dest = BB();
    XSetForeground(d, gc, 0xFFFFFF);
    // Clear ALL widget space before any
    XFillRectangle(d, dest, gc, 0, 0, (UINT32) (SizeX()), (UINT32) (SizeY()));
    if(dest == 0)
      dest = Wnd();
    // 5. Execute Render layers
    // Prepare Background
    DrawCells(dest, &rowRange, &colRange);
    DrawColumnHeader(dest, colStart, colEnd);
    DrawRowHeader(dest, rowStart, rowEnd);
    DrawIntersectionBox(dest);
    DrawScrollbars(dest);
    // 6. Flip the buffer to the screen
    if(dest != Wnd()) {
      XCopyArea(d, dest, Wnd(), GCPtr(), 0, 0, (UINT32) SizeX(),
          (UINT32) SizeY(), 0, 0);
    }
    XSync(d, False);
    D2("draw {}", mDebugDrawCounter++)
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

  ATableRangeData1 ATable::Offset2Row(INT64 offset) {
    if(mRowH.empty())
      return {-1, -1};
    // Constant time calculation instead of loop
    INT64 rowIdx = offset / AUI_TABLE_CELL_H;
    INT64 internalOffset = offset % AUI_TABLE_CELL_H;
    // Safety check against total rows
    if(rowIdx >= SafeINT64(mRowH.size())) {
      rowIdx = SafeINT64(mRowH.size()) - 1;
      internalOffset = AUI_TABLE_CELL_H;
    }
    return {(INT64)rowIdx, internalOffset};
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
      endr.cell = SafeINT64(mRowH.size()) - 1;
    }
    return endr;
  }

  void ATable::DrawRowHeader(Drawable dest, ATableRangeData1 rowStartData,
      ATableRangeData1 rowEndData) {
    Display *d = AUIPtr()->Disp();
    GC gc = GCPtr();
    XFontStruct *font_info = Font();
    UINT64 y_pos = SafeUINT64(
        static_cast<INT64>(mColumnHeaderHeight) - rowStartData.offset);
    for (INT64 i = rowStartData.cell; i <= rowEndData.cell; i++) {
      UINT64 currentH = SafeUINT64(RowHeight(i));
      if(currentH > 0 && y_pos + currentH > mColumnHeaderHeight
          && y_pos < SizeY()) {
        XSetForeground(d, gc, 0xCCCCCC);
        XFillRectangle(d, dest, gc, 0, (INT32) y_pos, (UINT32) mRowHeaderWidth,
            (UINT32) currentH);
        XSetForeground(d, gc, 0x000000);
        XDrawRectangle(d, dest, gc, 0, (INT32) y_pos, (UINT32) mRowHeaderWidth,
            (UINT32) currentH);
        std::string label = mRowH[i].second;
        INT32 len = SafeINT32(label.length());
        INT32 text_w =
            font_info ?
                XTextWidth(font_info, label.c_str(), len) :
                (INT32) label.length() * 6;
        XDrawString(d, dest, gc,
            SafeINT32(static_cast<INT64>(mRowHeaderWidth) - text_w - 4),
            SafeINT32(
                static_cast<INT64>(y_pos) + (static_cast<INT64>(currentH) / 2)
                    + 4), label.c_str(), len);
      }
      y_pos += currentH;
      if(y_pos >= SizeY())
        break;
    }
  }

  INT64 ATable::AddRow() {
    UINT64 lastK = 0;
    if(!mRows.empty()) {
      lastK = static_cast<UINT64>(mRows.rbegin()->first) + 1;
    }
    INT64 signedKey = SafeINT64(lastK);
    mRows[signedKey];
    if(!mRowH.contains(signedKey)) {
      std::string label = std::to_string(lastK);
      mRowH[signedKey] = { AUI_TABLE_CELL_H, label };
      mTotalContentHeight += AUI_TABLE_CELL_H;
      static size_t lastLen = 0;
      if(label.length() > lastLen) {
        XFontStruct *font_info = Font();
        INT32 len = SafeINT32(label.length());
        INT32 text_width =
            font_info ? XTextWidth(font_info, label.c_str(), len) : len * 7;
        mRowHeaderWidth = SafeUINT32(text_width + 12);
        lastLen = label.length();
      }
    }
    return signedKey;
  }

  void ATable::ScrollUpPx(INT64 px) {
    mVOffset -= px;
    if(mVOffset < 0)
      mVOffset = 0;
    Draw();
  }

  void ATable::ScrollDownPx(INT64 px) {
    INT64 visibleArea = SafeINT64(SizeY()) - mColumnHeaderHeight;
    INT64 maxScroll = mTotalContentHeight - visibleArea;
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

  void ATable::Insert(INT64 row, INT64 col, AUICellData *cell) {
    if(!cell)
      return;
    mRows[row][col] = std::move(*cell);
    mColumns[col][row] = &mRows[row][col];
    if(mAutoWiden) {
      XFontStruct *font_info = Font();
      INT32 textW =
          font_info ?
              XTextWidth(font_info, mRows[row][col].data.c_str(),
                  (INT32) mRows[row][col].data.length()) :
              (INT32) mRows[row][col].data.length() * 7;
      INT32 padding = 15;
      if((INT64) textW + padding > ColumnWidth(col)) {
        INT64 newW = (INT64) textW + padding;
        mTotalContentWidth += (newW - mColumnW[col].first);
        mColumnW[col].first = newW;
      }
    }
  }

  void ATable::DrawCells(Drawable dest, ATableRangeData2 *rowRange,
      ATableRangeData2 *colRange) {
    AUI *au = AUIPtr();
    Display *d = au->Disp();
    GC gc = GCPtr();
    XFontStruct *font_info = Font();
    // The data area clipping rectangle
    XRectangle globalClip = { SafeINT16(mRowHeaderWidth), SafeINT16(
        mColumnHeaderHeight), SafeUINT16(
        (INT64) SizeX() - (INT64) mRowHeaderWidth), SafeUINT16(
        (INT64) SizeY() - (INT64) mColumnHeaderHeight) };
    INT64 y_pos = (INT64) mColumnHeaderHeight - rowRange->offset;
    for (INT64 r = rowRange->cell; r <= rowRange->cell2; ++r) {
      INT64 currentH = RowHeight(r);
      if(currentH <= 0)
        continue;
      if(y_pos >= (INT64) SizeY())
        break;
      if(r == mCursorRow) {
        XSetClipRectangles(d, gc, 0, 0, &globalClip, 1, Unsorted);
        // Light blue: usually 0xADD8E6 or similar; using a clear sky blue here
        XSetForeground(d, gc, 0xCCE5FF);
        XFillRectangle(d, dest, gc, SafeINT16(mRowHeaderWidth),
            SafeINT16(y_pos),
            SafeUINT16((INT64) SizeX() - (INT64) mRowHeaderWidth),
            SafeUINT16(currentH));
      }
      INT64 x_pos = (INT64) mRowHeaderWidth - colRange->offset;
      for (INT64 c = colRange->cell; c <= colRange->cell2; ++c) {
        INT64 currentW = ColumnWidth(c);
        if(currentW <= 0) {
          x_pos += currentW;
          continue;
        }
        if(x_pos >= (INT64) SizeX())
          break;
        if(x_pos + currentW > (INT64) mRowHeaderWidth
            && y_pos + currentH > (INT64) mColumnHeaderHeight) {
          // 1. Set per-cell clip for text
          XRectangle cellClip = { SafeINT16(x_pos), SafeINT16(y_pos),
              SafeUINT16(currentW - 1), SafeUINT16(currentH - 1) };
          XSetClipRectangles(d, gc, 0, 0, &cellClip, 1, Unsorted);
          auto itRow = mRows.find(r);
          if(itRow != mRows.end()) {
            auto itCol = itRow->second.find(c);
            if(itCol != itRow->second.end()) {
              AUICellData &cell = itCol->second;
              if(!cell.data.empty()) {
                INT32 textW =
                    font_info ?
                        (INT32) XTextWidth(font_info, cell.data.c_str(),
                            (int) cell.data.length()) :
                        SafeINT32((UINT64) cell.data.length() * 7);
                INT32 fontAscent = font_info ? (INT32) font_info->ascent : 12;
                INT32 tx = SafeINT32(x_pos + 4);
                if(cell.hAlign == AUIHAlign::center) {
                  tx = SafeINT32(x_pos + (currentW - (INT64) textW) / 2);
                } else if(cell.hAlign == AUIHAlign::right) {
                  tx = SafeINT32(x_pos + currentW - (INT64) textW - 4);
                }
                INT32 ty = SafeINT32(y_pos + (INT64) fontAscent + 2);
                XSetForeground(d, gc, 0x000000); // Black text
                XDrawString(d, dest, gc, tx, ty, cell.data.c_str(),
                    (int) cell.data.length());
              }
            }
          }
          // 2. Restore global clip for grid lines and cursor
          XSetClipRectangles(d, gc, 0, 0, &globalClip, 1, Unsorted);
          // Grid Lines
          XSetForeground(d, gc, 0xDDDDDD);
          XDrawLine(d, dest, gc, SafeINT32(x_pos + currentW - 1),
              SafeINT32(y_pos), SafeINT32(x_pos + currentW - 1),
              SafeINT32(y_pos + currentH - 1));
          XDrawLine(d, dest, gc, SafeINT32(x_pos),
              SafeINT32(y_pos + currentH - 1), SafeINT32(x_pos + currentW - 1),
              SafeINT32(y_pos + currentH - 1));
          // Selected Cell Cursor (The dark blue border)
          if(r == mCursorRow && c == mCursorCol) {
            XSetForeground(d, gc, 0x3399FF);
            XSetLineAttributes(d, gc, 2, LineSolid, CapButt, JoinMiter);
            XDrawRectangle(d, dest, gc, SafeINT32(x_pos), SafeINT32(y_pos),
                SafeUINT16(currentW - 1), SafeUINT16(currentH - 1));
            XSetLineAttributes(d, gc, 1, LineSolid, CapButt, JoinMiter);
          }
        }
        x_pos += currentW;
      }
      y_pos += currentH;
    }
    XSetClipMask(d, gc, None);
  }

  void ATable::OnButtonPress(XEvent *ev) {
    INT32 x = ev->xbutton.x;
    INT32 y = ev->xbutton.y;
    UINT32 button = ev->xbutton.button;
    Display *d = AUIPtr()->Disp();
    Window w = Wnd();
    if(button == Button4) {
      ScrollUpPx(60);
      return;
    }
    if(button == Button5) {
      ScrollDownPx(60);
      return;
    }
    if(button == Button1) {
      if(y < SafeINT32(mColumnHeaderHeight) && x > SafeINT32(mRowHeaderWidth)) {
        ATableRangeData1 colStart = Offset2Column(mHOffset);
        INT64 currX = mRowHeaderWidth - colStart.offset;
        for (auto const& [id, data] : mColumnW) {
          if(id < colStart.cell)
            continue;
          currX += data.first;
          if(std::abs(x - SafeINT32(currX)) < 5) {
            mResizeMode = AUIResizeMode::AUIColumn;
            mResizeId = id;
            mResizeBasePos = x;
            mResizeBaseSize = data.first;
            XDefineCursor(d, w, mHorizCursor);
            return;
          }
        }
      }
      if(mRowHeaderResizeEnabled
          && std::abs(x - SafeINT32(mRowHeaderWidth)) < 5) {
        mResizeMode = AUIResizeMode::AUIHeader;
        mResizeBasePos = x;
        mResizeBaseSize = (INT64) mRowHeaderWidth;
        XDefineCursor(d, w, mHorizCursor);
        return;
      }
      if(mRowHeightResizeEnabled && x < SafeINT32(mRowHeaderWidth)
          && y > SafeINT32(mColumnHeaderHeight)) {
        ATableRangeData1 rowStart = Offset2Row(mVOffset);
        INT64 currY = mColumnHeaderHeight - rowStart.offset;
        for (auto const& [id, data] : mRowH) {
          if(id < rowStart.cell)
            continue;
          currY += data.first;
          if(std::abs(y - SafeINT32(currY)) < 5) {
            mResizeMode = AUIResizeMode::AUIRow;
            mResizeId = id;
            mResizeBasePos = y;
            mResizeBaseSize = data.first;
            XDefineCursor(d, w, mVertCursor);
            return;
          }
        }
      }
      if(SafeUINT64(x) >= SizeX() - SafeUINT64(AUI_TABLE_SCROLL_THICK)
          && SafeUINT64(y) > SafeUINT64(mColumnHeaderHeight)) {
        double viewH =
            (double) SizeY() - (double)mColumnHeaderHeight - (double)AUI_TABLE_SCROLL_THICK;
        if((double) mTotalContentHeight > viewH) {
          mCurrentScrollMode = AUIScrollMode::AUIVertical;
          double thumbH = std::max(20.0,
              (viewH / (double) mTotalContentHeight) * viewH);
          double maxScroll = (double) mTotalContentHeight - viewH;
          double travelTrack = viewH - thumbH;
          INT32 vPos = SafeINT32(mColumnHeaderHeight)
              + (INT32) (((double) mVOffset / maxScroll) * travelTrack);
          if(y >= vPos && y <= vPos + (INT32) thumbH)
            mScrollGrabOffset = y - vPos;
          else
            mScrollGrabOffset = (INT32) (thumbH / 2.0);
          OnMouseMove(ev);
        }
        return;
      }
      if(SafeUINT64(y) >= SizeY() - SafeUINT64(AUI_TABLE_SCROLL_THICK)
          && SafeUINT64(x) > SafeUINT64(mRowHeaderWidth)) {
        double viewW =
            (double) SizeX() - (double)mRowHeaderWidth - (double)AUI_TABLE_SCROLL_THICK;
        if((double) mTotalContentWidth > viewW) {
          mCurrentScrollMode = AUIScrollMode::AUIHorizontal;
          double thumbW = std::max(20.0,
              (viewW / (double) mTotalContentWidth) * viewW);
          double maxScroll = (double) mTotalContentWidth - viewW;
          double travelTrack = viewW - thumbW;
          INT32 hPos = SafeINT32(mRowHeaderWidth)
              + (INT32) (((double) mHOffset / maxScroll) * travelTrack);
          if(x >= hPos && x <= hPos + (INT32) thumbW)
            mScrollGrabOffset = x - hPos;
          else
            mScrollGrabOffset = (INT32) (thumbW / 2.0);
          OnMouseMove(ev);
        }
        return;
      }
      auto [row, col] = ScreenToCell(x, y);
      if(row != -1 && col != -1) {
        D1("setting cursor RC[{},{}]", row, col)
        mCursorRow = row;
        mCursorCol = col;
        mSelectedRow = row;
        Draw();
      }
    }
  }

  void ATable::OnButtonRelease(XEvent *ev) {
    mCurrentScrollMode = AUIScrollMode::AUINone;
    mResizeMode = AUIResizeMode::AUINone;
    mResizeId = -1;
    XUndefineCursor(AUIPtr()->Disp(), Wnd());
    OnMouseMove(ev);
  }

  std::pair<INT64, INT64> ATable::ScreenToCell(INT32 x, INT32 y) {
    // 1. Basic boundary check: Ignore clicks on headers or scrollbar areas
    if(x < (INT32) mRowHeaderWidth || y < (INT32) mColumnHeaderHeight
        || x > (INT32) (SizeX() - AUI_TABLE_SCROLL_THICK)
        || y > (INT32) (SizeY() - AUI_TABLE_SCROLL_THICK)) {
      return {-1, -1};
    }
    // 2. Get the first visible cell indices based on current scroll offsets
    // This prevents iterating through thousands of off-screen cells
    auto colStart = Offset2Column(mHOffset);
    auto rowStart = Offset2Row(mVOffset);
    INT64 targetRow = -1, targetCol = -1;
    // 3. Find Row: Start searching from the first visible row index
    INT64 currY = (INT64) mColumnHeaderHeight - rowStart.offset;
    auto itRow = mRowH.lower_bound(rowStart.cell);
    for (; itRow != mRowH.end(); ++itRow) {
      if(y >= currY && y < currY + itRow->second.first) {
        targetRow = itRow->first;
        break;
      }
      currY += itRow->second.first;
      // Optimization: stop if we've passed the click point
      if(currY > y)
        break;
    }
    // 4. Find Column: Start searching from the first visible column index
    INT64 currX = (INT64) mRowHeaderWidth - colStart.offset;
    auto itCol = mColumnW.lower_bound(colStart.cell);
    for (; itCol != mColumnW.end(); ++itCol) {
      if(x >= currX && x < currX + itCol->second.first) {
        targetCol = itCol->first;
        break;
      }
      currX += itCol->second.first;
      // Optimization: stop if we've passed the click point
      if(currX > x)
        break;
    }
    return {targetRow, targetCol};
  }

  void ATable::DrawScrollbars(Drawable dest) {
    Display *d = AUIPtr()->Disp();
    GC gc = GCPtr();
    double viewH =
        (double) SizeY() - (double)mColumnHeaderHeight - (double)AUI_TABLE_SCROLL_THICK;
    double viewW =
        (double) SizeX() - (double)mRowHeaderWidth - (double)AUI_TABLE_SCROLL_THICK;
    if((double) mTotalContentHeight > viewH) {
      double thumbH = std::max(20.0,
          (viewH / (double) mTotalContentHeight) * viewH);
      double maxScroll = (double) mTotalContentHeight - viewH;
      double travelTrack = viewH - thumbH;
      INT32 vPos = SafeINT32(mColumnHeaderHeight)
          + (INT32) (((double) mVOffset / maxScroll) * travelTrack);
      XSetForeground(d, gc, 0x888888);
      XFillRectangle(d, dest, gc,
          SafeINT32(SizeX() - SafeUINT64(AUI_TABLE_SCROLL_THICK)), vPos,
          SafeUINT32(AUI_TABLE_SCROLL_THICK), static_cast<UINT32>(thumbH));
    }
    if((double) mTotalContentWidth > viewW) {
      double thumbW = std::max(20.0,
          (viewW / (double) mTotalContentWidth) * viewW);
      double maxScroll = (double) mTotalContentWidth - viewW;
      double travelTrack = viewW - thumbW;
      INT32 hPos = SafeINT32(mRowHeaderWidth)
          + (INT32) (((double) mHOffset / maxScroll) * travelTrack);
      XSetForeground(d, gc, 0x888888);
      XFillRectangle(d, dest, gc, hPos,
          SafeINT32(SizeY() - SafeUINT64(AUI_TABLE_SCROLL_THICK)),
          static_cast<UINT32>(thumbW), SafeUINT32(AUI_TABLE_SCROLL_THICK));
    }
  }

  void ATable::OnMouseMove(XEvent *ev) {
    Display *d = AUIPtr()->Disp();
    Window w = Wnd();
    INT32 x = ev->xmotion.x;
    INT32 y = ev->xmotion.y;
    if(mResizeMode == AUIResizeMode::AUIColumn) {
      XDefineCursor(d, w, mHorizCursor);
      INT32 delta = x - mResizeBasePos;
      XFontStruct *font_info = Font();
      std::string label = mColumnW[mResizeId].second;
      INT32 textW =
          font_info ?
              XTextWidth(font_info, label.c_str(), (INT32) label.length()) :
              (INT32) label.length() * 7;
      INT64 minAllowedW = (INT64) textW + 20;

      INT64 newW = std::max(minAllowedW, mResizeBaseSize + delta);
      mTotalContentWidth += (newW - mColumnW[mResizeId].first);
      mColumnW[mResizeId].first = newW;
      INT64 viewW = (INT64) SizeX() - mRowHeaderWidth - AUI_TABLE_SCROLL_THICK;
      INT64 maxScroll = std::max((INT64) 0, mTotalContentWidth - viewW);
      if(mHOffset > maxScroll)
        mHOffset = maxScroll;
      Draw();
      return;
    }
    if(mResizeMode == AUIResizeMode::AUIRow) {
      XDefineCursor(d, w, mVertCursor);
      INT32 delta = y - mResizeBasePos;
      INT64 newH = std::max((INT64) 15, mResizeBaseSize + delta);
      mTotalContentHeight += (newH - mRowH[mResizeId].first);
      mRowH[mResizeId].first = newH;
      INT64 viewH =
          (INT64) SizeY() - mColumnHeaderHeight - AUI_TABLE_SCROLL_THICK;
      INT64 maxScroll = std::max((INT64) 0, mTotalContentHeight - viewH);
      if(mVOffset > maxScroll)
        mVOffset = maxScroll;
      Draw();
      return;
    }
    if(mResizeMode == AUIResizeMode::AUIHeader) {
      XDefineCursor(d, w, mHorizCursor);
      INT32 delta = x - mResizeBasePos;
      mRowHeaderWidth = SafeUINT32(
          std::max((INT64) 20, (INT64) mResizeBaseSize + (INT64) delta));
      Draw();
      return;
    }
    if(mCurrentScrollMode == AUIScrollMode::AUIVertical) {
      double viewH =
          (double) SizeY() - mColumnHeaderHeight - AUI_TABLE_SCROLL_THICK;
      double thumbH = std::max(20.0,
          (viewH / (double) mTotalContentHeight) * viewH);
      double travelTrack = viewH - thumbH;
      double maxScroll = (double) mTotalContentHeight - viewH;
      if(travelTrack > 0) {
        double trackY = (double) y - (double) mColumnHeaderHeight
            - (double) mScrollGrabOffset;
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
        double trackX = (double) x - (double) mRowHeaderWidth
            - (double) mScrollGrabOffset;
        double ratio = trackX / travelTrack;
        mHOffset = (INT64) (std::max(0.0, std::min(1.0, ratio)) * maxScroll);
        Draw();
      }
      return;
    }
    bool cursorSet = false;
    if(y < SafeINT32(mColumnHeaderHeight) && x > SafeINT32(mRowHeaderWidth)) {
      ATableRangeData1 colStart = Offset2Column(mHOffset);
      INT64 currX = mRowHeaderWidth - colStart.offset;
      for (auto const& [id, data] : mColumnW) {
        if(id < colStart.cell)
          continue;
        currX += data.first;
        if(std::abs(x - (INT32) currX) < 5) {
          XDefineCursor(d, w, mHorizCursor);
          cursorSet = true;
          break;
        }
        if(currX > SafeINT64(SizeX()))
          break;
      }
    } else if(mRowHeightResizeEnabled && x < SafeINT32(mRowHeaderWidth)
        && y > SafeINT32(mColumnHeaderHeight)) {
      ATableRangeData1 rowStart = Offset2Row(mVOffset);
      INT64 currY = mColumnHeaderHeight - rowStart.offset;
      for (auto const& [id, data] : mRowH) {
        if(id < rowStart.cell)
          continue;
        currY += data.first;
        if(std::abs(y - (INT32) currY) < 5) {
          XDefineCursor(d, w, mVertCursor);
          cursorSet = true;
          break;
        }
        if(currY > SafeINT64(SizeY()))
          break;
      }
    }
    if(mRowHeaderResizeEnabled && std::abs(x - (INT32) mRowHeaderWidth) < 5) {
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

  void ATable::RemoveLastColumn() {
    if(mColumnW.empty())
      return;
    // std::map is sorted, so rbegin() is the highest index
    RemoveColumn(mColumnW.rbegin()->first);
  }

  void ATable::UpdateColumnWidthToFit(INT64 colIdx) {
    auto itCol = mColumns.find(colIdx);
    if(itCol == mColumns.end())
      return;
    XFontStruct *font_info = Font();
    INT64 maxCellContentW = 0;
    for (auto& [rowIdx, cellPtr] : itCol->second) {
      if(cellPtr) {
        INT32 textW =
            font_info ?
                XTextWidth(font_info, cellPtr->data.c_str(),
                    (INT32) cellPtr->data.length()) :
                (INT32) cellPtr->data.length() * 7;
        if(textW > maxCellContentW)
          maxCellContentW = textW;
      }
    }
    std::string headerLabel = mColumnW[colIdx].second;
    INT32 headerW =
        font_info ?
            XTextWidth(font_info, headerLabel.c_str(),
                (INT32) headerLabel.length()) :
            (INT32) headerLabel.length() * 7;
    INT64 finalW = std::max((INT64) maxCellContentW + 15, (INT64) headerW + 20);
    if(finalW != mColumnW[colIdx].first) {
      mTotalContentWidth += (finalW - mColumnW[colIdx].first);
      mColumnW[colIdx].first = finalW;
      Draw();
    }
  }

  ATableRangeData1 ATable::Offset2ColumnRange(ATableRangeData1 startColumn,
  INT64 width) {
    ATableRangeData1 endc = startColumn;
    auto it = mColumnW.find(startColumn.cell);
    if(it != mColumnW.end()) {
      INT64 remainingWidth = width - (INT64) mRowHeaderWidth;
      INT64 firstCellVisibleW = it->second.first - startColumn.offset;
      remainingWidth -= firstCellVisibleW;
      while (++it != mColumnW.end() && remainingWidth > 0) {
        endc.cell = it->first;
        remainingWidth -= it->second.first;
      }
    }
    return endc;
  }

  void ATable::DrawColumnHeader(Drawable dest, ATableRangeData1 colStartData,
      ATableRangeData1 colEndData) {
    Display *d = AUIPtr()->Disp();
    GC gc = GCPtr();
    XFontStruct *font_info = Font();
    // Start drawing at mRowHeaderWidth, adjusted back by the internal cell offset
    INT64 x_pos = (INT64) mRowHeaderWidth - colStartData.offset;
    XRectangle clip = { SafeINT16(mRowHeaderWidth), 0, SafeUINT16(
        SizeXUI32() - mRowHeaderWidth), SafeUINT16(mColumnHeaderHeight) };
    XSetClipRectangles(d, gc, 0, 0, &clip, 1, Unsorted);
    for (INT64 i = colStartData.cell; i <= colEndData.cell; i++) {
      INT64 currentW = ColumnWidth(i);
      if(currentW > 0 && x_pos < (INT64) SizeX()) {
        XSetForeground(d, gc, 0xCCCCCC);
        XFillRectangle(d, dest, gc, (INT32) x_pos, 0, (UINT32) currentW,
            (UINT32) mColumnHeaderHeight);
        XSetForeground(d, gc, 0x000000);
        XDrawRectangle(d, dest, gc, (INT32) x_pos, 0, (UINT32) currentW,
            (UINT32) mColumnHeaderHeight);
        std::string label = mColumnW[i].second;
        INT32 text_width =
            font_info ?
                XTextWidth(font_info, label.c_str(), (INT32) label.length()) :
                (INT32) label.length() * 6;
        INT32 text_x = (INT32) (x_pos + (currentW - text_width) / 2);
        XDrawString(d, dest, gc, text_x,
            (INT32) ((mColumnHeaderHeight / 2) + 4), label.c_str(),
            (INT32) label.length());
      }
      x_pos += currentW;
      if(x_pos > (INT64) SizeX())
        break;
    }
    XSetClipMask(d, gc, None);
  }

  void ATable::ScrollRightPx(INT64 px) {
    INT64 visibleDataArea =
        (INT64) SizeX() - (INT64)mRowHeaderWidth - (INT64)AUI_TABLE_SCROLL_THICK;
    INT64 maxScroll = mTotalContentWidth - visibleDataArea;
    if(maxScroll < 0)
      maxScroll = 0;
    mHOffset += px;
    if(mHOffset > maxScroll)
      mHOffset = maxScroll;
    Draw();
  }

  ATableRangeData1 ATable::Offset2Column(INT64 offset) {
    if(mColumnW.empty())
      return {-1, -1};
    INT64 accumulated = 0;
    for (const auto& [id, data] : mColumnW) {
      INT64 colW = data.first;
      // If the scroll offset falls within this column's span
      if(offset < accumulated + colW) {
        return {id, offset - accumulated};
      }
      accumulated += colW;
    }
    // If scrolled to the very end
    auto lastIt = mColumnW.rbegin();
    return {lastIt->first, lastIt->second.first};
  }

  void ATable::AddColumn() {
    UINT64 lastK = 0;
    if(!mColumns.empty()) {
      lastK = SafeUINT64(mColumns.rbegin()->first + 1);
    }
    INT64 colIdx = SafeINT64(lastK);
    mColumns[colIdx] = std::map<INT64, AUICellData*>();
    AUI *au = AUIPtr();
    if(!mColumnW.contains(colIdx)) {
      std::string name = au->NumberToBaseString(lastK);
      XFontStruct *font_info = Font();
      INT32 headerTextW =
          font_info ?
              XTextWidth(font_info, name.c_str(), (INT32) name.length()) :
              (INT32) name.length() * 7;
      INT64 minHeaderW = (INT64) headerTextW + 20; // Text + padding
      INT64 initialW = std::max((INT64) AUI_TABLE_CELL_W, minHeaderW);
      for (auto& [rowIdx, rowData] : mRows) {
        AUICellData &cell = rowData[colIdx];
        mColumns[colIdx][rowIdx] = &cell;
        if(mAutoWiden) {
          INT32 cellTextW =
              font_info ?
                  XTextWidth(font_info, cell.data.c_str(),
                      (INT32) cell.data.length()) :
                  (INT32) cell.data.length() * 7;
          INT32 padding = 15;
          if((INT64) cellTextW + padding > initialW) {
            initialW = (INT64) cellTextW + padding;
          }
        }
      }
      mColumnW[colIdx] = { initialW, name };
      mTotalContentWidth += initialW;
      Draw();
    } else {
      E("Column width info integrity error")
    }
  }

  void ATable::SetCursorPosition(INT64 row, INT64 column) {
    mCursorRow = row;
    mCursorCol = column;
    Draw();
  }

  INT64 ATable::CursorRow() {
    return mCursorRow;
  }

  INT64 ATable::CursorColumn() {
    return mCursorCol;
  }

  std::string ATable::DataAt(INT64 row, INT64 col) {
    D1("get data for RC[{},{}]", row, col);
    auto itRow = mRows.find(row);
    if(itRow == mRows.end()) {
      D1("Not found row {}", row);
      return "";
    }
    auto itCol = itRow->second.find(col);
    if(itCol == itRow->second.end()) {
      D1("Not found column {}", col);
      return "";
    }
    return itCol->second.data;
  }

  std::string ATable::DataAtCursor() {
    D1("get data for RC cursor [{},{}]", mCursorRow, mCursorCol);
    auto itRow = mRows.find(mCursorRow);
    if(itRow == mRows.end()) {
      D1("Not found cursor row {}", mCursorRow);
      return "";
    }
    auto itCol = itRow->second.find(mCursorCol);
    if(itCol == itRow->second.end()) {
      D1("Not found cursor column {}", mCursorCol);
      return "";
    }
    return itCol->second.data;
  }

  ATable::~ATable() {
    AUI *au = AUIPtr();
    UNUSED Display *d = au->Disp();
    // XCB CURSOR CLEANUP
    // We must use xcb_free_cursor with the XCB connection
    if(!au || !au->Disp()) {
      E("something wrong with ATable destruction")
    }
    xcb_connection_t *conn = XGetXCBConnection(d);
    if(conn && !xcb_connection_has_error(conn)) {
      D2("========xcb connected for deallocation")
    } else {
      E("========xcb is not avaliable for deallocation")
    }
    if(mHorizCursor != 0) {
      D2("freeing horizontal cursor via xcb")
      xcb_free_cursor(conn, (xcb_cursor_t) mHorizCursor);
      mHorizCursor = 0;
    } else {
      D3("not freeing horizontal cursor via xcb")
    }
    if(mVertCursor != 0) {
      D2("freeing vertical cursor via xcb")
      xcb_free_cursor(conn, (xcb_cursor_t) mVertCursor);
      mVertCursor = 0;
    } else {
      D3("not freeing vertical cursor via xcb")
    }
    xcb_flush(conn);
    mRows.clear();
    mColumns.clear();
    mRowH.clear();
    mColumnW.clear();
  }
}

