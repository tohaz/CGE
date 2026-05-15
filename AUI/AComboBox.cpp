#include "AComboBox.h"
#include "AUILib.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>

namespace aui {

// Private constructor implementing internal sub-widget layout composition (Read-Only always)
  AComboBox::AComboBox(AWidget *wParent, std::string inText) :
      AWidget(), mSelectedIndex(-1), mIsPopupOpen(false), mIsEditable(false) {
    SetAUIPtr(wParent->AUIPtr());
    this->SetWndParent(wParent);
    SetType(AUIWidgetType::defaultComboBox);
    Display *d = AUIPtr()->Disp();
    UINT32 initialW = 200;
    UINT32 initialH = 28;
    SetSize(initialW, initialH);
    Window w = XCreateSimpleWindow(d, wParent->Wnd(), 0, 0, initialW, initialH, 0, BlackPixel(d, 0),
        wParent->BGColor());
// Enable ButtonPressMask on the root ComboBox window to handle manual arrow region mouse clicks
    XSelectInput(d, w, ExposureMask | StructureNotifyMask | ButtonPressMask);
    InitWidgetProps(w);
    UpdateBuffer();
    AUIPtr()->AddWidget(this);
// Composite single text box child component. Arrow button is discarded to optimize GC context matching
    mInputBox = AInputBox::AttachTo(this, inText);
    mInputBox->SetOnButtonPressCB(AComboBox::OnInputClick, this);
    Resize(initialW, initialH);
    XMapWindow(d, w);
  }

// Factory generation access boundary entry-point
  AComboBox* AComboBox::AttachTo(AWidget *wParent, std::string inText) {
    if(!wParent)
[[unlikely]] {
            E("Cannot instantiate AComboBox container under a null parent widget profile context!");
      return nullptr;
    }
    return new AComboBox(wParent, inText);
  }

// --- COMPOSITION LAYOUT RULES SECTION ---

  void AComboBox::Resize(UINT32 w, UINT32 h) {
    AWidget::Resize(w, h);
    UINT32 arrowW = h;// Arrow button canvas is always a square block matching widget height
    if(w <= arrowW)
      [[unlikely]] return;
    UINT32 inputW = w - arrowW;
    if(mInputBox) {
      mInputBox->Move(0, 0);
      mInputBox->Resize(inputW, h);
    }
    UpdateBuffer();
  }

  void AComboBox::Move(UINT32 x, UINT32 y) {
    AWidget::Move(x, y);
    Display *d = AUIPtr()->Disp();
    if(d && Wnd()) {
      XMoveWindow(d, Wnd(), static_cast<int>(x), static_cast<int>(y));
      XFlush(d);
    }
  }

/**
   * Recursive Drawing Pipeline:
   * Renders the native background, delegates text box draws, and places the hardware triangle arrow
   * directly inside the native back-buffer to guarantee perfect color context sync without X11 conflicts.
   */
  void AComboBox::Draw() {
    Display *d = AUIPtr()->Disp();
    Pixmap buffer = BB();
    GC gc = GCPtr();
    if(!d || !buffer || !gc)
      return;
    UINT32 w = SizeXUI32();
    UINT32 h = SizeYUI32();
    UINT32 arrowW = h;
    UINT32 inputW = w - arrowW;
// 1. Clear container root background layer cleanly
    XSetForeground(d, gc, BGColor());
    XFillRectangle(d, buffer, gc, 0, 0, w, h);
// 2. Instruct sub-widgets to draw their core design primitives
    if(mInputBox)
      mInputBox->Draw();
// 3. HARDWARE X11 GEOMETRIC ARROW DRAWING STAGE:
// Step A: Paint the rectangular bounding box button background placeholder on the right side
    XSetForeground(d, gc, 0xD0D0D0);// Set standard frame button background light-gray hex profile
    XFillRectangle(d, buffer, gc, static_cast<int>(inputW), 0, arrowW, h);
// Step B: Draw a crisp black border around the arrow button space area
    XSetForeground(d, gc, BlackPixel(d, 0));
    XDrawRectangle(d, buffer, gc, static_cast<int>(inputW), 0, arrowW - 1U, h - 1U);
// Step C: Calculate a perfectly scaled bounding triangle dimensions matrix inside the button bounds
    INT32 centerX = static_cast<int>(inputW + (arrowW / 2U));
    INT32 centerY = static_cast<int>(h / 2U);
    INT32 baseHalfWidth = static_cast<int>(h / 6U);
    INT32 arrowHeight = static_cast<int>(h / 7U);
    if(baseHalfWidth < 3)
      baseHalfWidth = 3;
    if(arrowHeight < 2)
      arrowHeight = 2;
// Define 3 explicit corner vertices for a sharp downward triangle arrow orientation
    XPoint trianglePoints[3];
// Vertex 1: Left Top corner
    trianglePoints[0].x = static_cast<short>(centerX - baseHalfWidth);
    trianglePoints[0].y = static_cast<short>(centerY - (arrowHeight / 2));
// Vertex 2: Right Top corner
    trianglePoints[1].x = static_cast<short>(centerX + baseHalfWidth);
    trianglePoints[1].y = static_cast<short>(centerY - (arrowHeight / 2));
// Vertex 3: Center Bottom point (The tip pointing down)
    trianglePoints[2].x = static_cast<short>(centerX);
    trianglePoints[2].y = static_cast<short>(centerY + (arrowHeight / 2) + 1);
// Execute hardware raster fill optimization natively to the unified back-buffer canvas frame
    XFillPolygon(d, buffer, gc, trianglePoints, 3, Complex, CoordModeOrigin);
// 4. Output the fully assembled composited back-buffer onto the real hardware X11 window surface
    XCopyArea(d, buffer, Wnd(), gc, 0, 0, w, h, 0, 0);
    XFlush(d);
  }

  void AComboBox::PositionPopup() {
    if(!mListView || !Wnd() || !AUIPtr())
      return;
    Display *d = AUIPtr()->Disp();
    int absX = 0;
    int absY = 0;
    Window dummyChild;

// Translate coordinates from ComboBox local space directly to the screen RootWindow workspace
    XTranslateCoordinates(d, Wnd(), XRootWindow(d, AUIPtr()->Scr()), 0, static_cast<int>(SizeYUI32()), &absX, &absY,
        &dummyChild);
    UINT32 popupW = SizeXUI32();
    UINT32 popupH = 150;
    mListView->Move(static_cast<UINT32>(absX), static_cast<UINT32>(absY));
    mListView->Resize(popupW, popupH);
  }

  void AComboBox::OpenDropDown() {
    D2("AComboBox::OpenDropDown() -> Entering unified flat window execution path");
    if(mIsPopupOpen)
      return;
    Display *d = AUIPtr()->Disp();
    if(!d)
      return;
    if(mListView && !AUIPtr()->IsWindowRegistered(mListView->Wnd())) {
      mListView = nullptr;
    }

    if(!mListView) {
      D2("AComboBox::OpenDropDown() -> Creating flat AList dropdown window");
      AWidgetSettings listSettings;
      listSettings.width = SizeXUI32();
      listSettings.height = 150;
      listSettings.backgroundColor = AUI_LIST_BG;
// CRITICAL FIX: Set the type identifier explicitly to defaultComboBox!
// This tells the AUI::ProcessMessages() event loop loop modal barrier that
// this specific list behaves exactly like a combo popup layer, routing routing
// the click-outside events into the clean CloseDropDown() collapse path path instead of crashing.
      listSettings.type = AUIWidgetType::defaultComboBox;
      listSettings.startVisible = false;// Hold invisible during setup layout transforms

      mListView = AList::AttachTo(this, listSettings);
      mListView->SetOnButtonPressCB(AComboBox::OnItemSelect, this);

      XSetWindowAttributes attr;
      attr.override_redirect = True;
      XChangeWindowAttributes(d, mListView->Wnd(), CWOverrideRedirect, &attr);

      Atom netWmWindowType = XInternAtom(d, "_NET_WM_WINDOW_TYPE", False);
      Atom netWmWindowTypeCombo = XInternAtom(d, "_NET_WM_WINDOW_TYPE_COMBO", False);
      XChangeProperty(d, mListView->Wnd(), netWmWindowType, XA_ATOM, 32,
      PropModeReplace, reinterpret_cast<unsigned char*>(&netWmWindowTypeCombo), 1);
    }
    mListView->Clear();
    for (const auto &item : mItems) {
      mListView->AddItem(item);
    }
    mIsPopupOpen = true;
    PositionPopup();
    D2("AComboBox::OpenDropDown() -> Committing list visibility directly onto screen");
    XMapWindow(d, mListView->Wnd());
    XRaiseWindow(d, mListView->Wnd());
    mListView->Draw();
    XFlush(d);
    AUIPtr()->PushModal(mListView);
  }

  void AComboBox::CloseDropDown() {
    D2("AComboBox::CloseDropDown() -> Collapsing menu layer");
    if(!mIsPopupOpen)
      return;

    Display *d = AUIPtr()->Disp();
    if(!d)
      return;
    AUIPtr()->PopModal(mListView);
    if(mListView && AUIPtr()->IsWindowRegistered(mListView->Wnd())) {
      Window listHandle = mListView->Wnd();
      D2("AComboBox::CloseDropDown() -> Clearing active X11 input focus barriers");
// HARDWARE WORKAROUND: Forcefully revert the input focus back to the parent window tree.
// This cleanly breaks the active X11 pointer grab execution loop state,
// allowing XUnmapWindow to process immediately without safe asynchronous delays.
      XSetInputFocus(d, PointerRoot, RevertToParent, CurrentTime);
      D2("AComboBox::CloseDropDown() -> Executing hardware XUnmapWindow");
      XUnmapWindow(d, listHandle);
      XFlush(d);
    }
    mIsPopupOpen = false;
    D2("AComboBox::CloseDropDown() -> Complete");
  }

  void AComboBox::OnButtonPress(XEvent *ev) {
    D2()
    if(ev->type != ButtonPress)
      return;
    UINT32 w = SizeXUI32();
    UINT32 h = SizeYUI32();
    UINT32 arrowW = h;
    UINT32 inputW = w - arrowW;
    INT32 clickX = ev->xbutton.x;
    UNUSED INT32 clickY = ev->xbutton.y;// Declared to fix compilation error
// Safe logging via std::println-compatible macro syntax instead of operator+ string building
// Using explicit casting/integer evaluation for boolean state to suppress -Wsign-promo
    D2("AComboBox::OnButtonPress() -> Coordinates clickX: {}, clickY: {}, Open State: {}", clickX, clickY,
        mIsPopupOpen ? 1 : 0);
    bool clickedArrow = (clickX >= static_cast<int>(inputW) && clickX <= static_cast<int>(w));
    if(clickedArrow || !mIsEditable) {
      if(mIsPopupOpen) {
        D1("AComboBox::OnButtonPress() -> Path: CloseDropDown");
        CloseDropDown();
      } else {
        D2("AComboBox::OnButtonPress() -> Path: OpenDropDown");
        OpenDropDown();
      }
    }
  }

// 2. WIPE / RESET MEMORY AND WINDOW STATES
  void AComboBox::Clear() {
    D1("AComboBox::Clear() -> Flushing item vectors and collapsing states");
    mItems.clear();
    mSelectedIndex = -1;

    if(mListView) {
      mListView->Clear();
    }
    if(mInputBox) {
      mInputBox->SetText("");
    }
    if(mIsPopupOpen) {
      CloseDropDown();
    }
  }

// 3. READ ACTIVE SELECTION INDEX
  INT64 AComboBox::GetSelectedIndex() const {
    return mSelectedIndex;
  }

// 4. WRITE / FORCE SELECTION STATE MUTATION
  void AComboBox::SetSelectedIndex(INT32 index) {
    if(index < 0 || index >= static_cast<INT32>(mItems.size())) {
      E("AComboBox::SetSelectedIndex -> Index %d out of bounds (Size: %zu)", index, mItems.size());
      return;
    }
    mSelectedIndex = index;

// Sync the presentation text layer inside the input box context
    if(mInputBox) {
      mInputBox->SetText(mItems[static_cast<size_t>(mSelectedIndex)]);
    }
// Sync the list pointer highlight bounds if the widget is mapped
    if(mIsPopupOpen && mListView) {
      mListView->SetSelectedIndex(static_cast<UINT64>(mSelectedIndex + 1));
    }
  }

// 5. EXTRACT TARGET STRING BY INDEX PRIVILEGES
  std::string AComboBox::GetItemText(INT32 index) const {
    if(index < 0 || index >= static_cast<INT32>(mItems.size())) {
      E("AComboBox::GetItemText -> Request index %d is out of bounds", index);
      return "";
    }
    return mItems[static_cast<size_t>(index)];
  }

  void AComboBox::AddItem(std::string item) {
    mItems.push_back(item);
// If list is currently live on screen, dynamically push the new entry
    if(mIsPopupOpen && mListView) {
      mListView->AddItem(item);
    }
  }

  void AComboBox::OnInputClick(XEvent *ev, UNUSED AWidget *w, void *data) {
    auto *self = static_cast<AComboBox*>(data);
    if(!self || ev->type != ButtonPress)
      return;
    D1("AComboBox::OnInputClick() -> Input box click caught");
// If the combo box is not editable, a click anywhere on the text region
// acts as a trigger to toggle the dropdown list state.
    if(!self->mIsEditable) {
      if(self->mIsPopupOpen) {
        self->CloseDropDown();
      } else {
        self->OpenDropDown();
      }
    }
  }

  void AComboBox::OnItemSelect(XEvent *ev, UNUSED AWidget *w, void *data) {
    auto *self = static_cast<AComboBox*>(data);

// FACT TRACING: Log immediately upon entering the function
    D2("AComboBox::OnItemSelect ENTRY -> Event type: %d, Window in event: 0x%lx", ev->type,
        (unsigned long)ev->xbutton.window);

    if(!self) {
      E("AComboBox::OnItemSelect -> Critical: self pointer context is null!");
      return;
    }
    if(!self->mListView) {
      E("AComboBox::OnItemSelect -> Critical: mListView is null inside combo context!");
      return;
    }
// HARDWARE FIX 2: Check for ButtonPress event type instead of ButtonRelease
    if(ev->type != ButtonPress) {
      D1("AComboBox::OnItemSelect -> Bypassing event because type is not ButtonPress (type=%d)", ev->type);
      return;
    }
    Display *d = self->AUIPtr()->Disp();
    INT32 listX = 0, listY = 0;
    Window dummyChild;
// Log the raw hardware click coordinates before translation
    D2("AComboBox::OnItemSelect -> Raw X11 coordinates: x={}, y={}", ev->xbutton.x, ev->xbutton.y);
    XTranslateCoordinates(d, ev->xbutton.window, self->mListView->Wnd(), ev->xbutton.x, ev->xbutton.y, &listX, &listY,
        &dummyChild);
// Log the coordinates after conversion into the local AList workspace
    D2("AComboBox::OnItemSelect -> Synchronized local AList coordinates: x={}, y={}", listX, listY);
    UINT32 clickY = 0;
    if(listY >= 0) {
      clickY = static_cast<UINT32>(listY);
    } else {
      D1("AComboBox::OnItemSelect -> Warning: Synchronized clickY is negative ({}). Truncating to 0.", listY);
    }
    UINT64 calculatedIndex = self->mListView->IndexFromY(clickY);
// FACT TRACING: Log what index AList computed from this coordinate
    D2("AComboBox::OnItemSelect -> AList::IndexFromY returned index value: {} (mData size is {})",
        (UINT64)calculatedIndex, self->mItems.size());

    if(calculatedIndex > 0) {
      INT32 finalTargetIndex = static_cast<INT32>(calculatedIndex - 1);
      D2("AComboBox::OnItemSelect -> SUCCESS ROUTINE -> Executing SetSelectedIndex({})", finalTargetIndex);
      self->SetSelectedIndex(finalTargetIndex);
      if(self->mOnSelectionChangeCB) {
        D1("AComboBox::OnItemSelect -> Triggering external user callback integration");
        self->mOnSelectionChangeCB(self, SafeINT32(self->mSelectedIndex), self->mUserCallbackData);
      }
    } else {
      D1("AComboBox::OnItemSelect -> Bypassed state mutation because calculatedIndex is 0 (Out of Bounds click)");
    }

    D2("AComboBox::OnItemSelect -> Initiating CloseDropDown() closure pipeline");
    self->CloseDropDown();
    self->Draw();
  }
  void AComboBox::SetOnSelectionChangeCB(std::function<void(AWidget *w, INT32 selectedIndex, void *arbData)> func,
      void *data) {
    D()
    mOnSelectionChangeCB = func;
    mUserCallbackData = data;
  }

  AComboBox::~AComboBox() {
    if(mIsPopupOpen) {
      CloseDropDown();
    }
    D3("AComboBox destructor layout unlinked cleanly.");
  }

}// namespace aui
