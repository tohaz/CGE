#ifndef DEFAULTS_H_
#define DEFAULTS_H_

#define DEBUG_LEVEL 1

static_assert(sizeof(char) == 1, "char must be 1 bytes");
static_assert(sizeof(short) == 2, "short must be 2 bytes");
static_assert(sizeof(int) == 4, "integer must be 4 bytes");
static_assert(sizeof(long) == 8, "long must be 8 bytes");
#define UCHAR8 unsigned char
#define CHAR8 char
#define UINT16 unsigned short
#define INT16 short
#define UINT32 unsigned int
#define INT32 int
#define UINT64 unsigned long
#define INT64 long

union RGBAColor {
    UINT32 value;
    struct {
        UCHAR8 b;
        UCHAR8 g;
        UCHAR8 r;
        UCHAR8 a;
    };
};

// CGWindow
#define AUI_DEFAULT_WINDOW_TITLE "aui dummy title, set me plz"
#define AUI_DEFAULT_WINDOW_SZX 500
#define AUI_DEFAULT_WINDOW_SZY 300
#define AUI_DEFAULT_WINDOW_BG 0xAAAAAA
// CGButton
#define AUI_DEFAULT_BUTTON_X 10
#define AUI_DEFAULT_BUTTON_Y 10
#define AUI_DEFAULT_BUTTON_SZX 80
#define AUI_DEFAULT_BUTTON_SZY 30
#define AUI_DEFAULT_BUTTON_BORDERW 3
#define AUI_DEFAULT_BUTTON_BG 0xCCCCCC
// CGInput
#define AUI_DEFAULT_INPUT_X 15
#define AUI_DEFAULT_INPUT_Y 15
#define AUI_DEFAULT_INPUT_SZX 90
#define AUI_DEFAULT_INPUT_SZY 25
#define AUI_DEFAULT_INPUT_BG 0xAACCAA
#define AUI_DEFAULT_INPUT_FG 0x000000
#define AUI_DEFAULT_INPUT_CURSORW 2
#define AUI_DEFAULT_INPUT_CURSORH 10
#define AUI_DEFAULT_INPUT_BORDERW 2

// CGLabel
#define AUI_DEFAULT_LABEL_BG 0xBBBBBB
#define AUI_DEFAULT_LABEL_SZX 120
#define AUI_DEFAULT_LABEL_SZY 30
#define AUI_DEFAULT_LABEL_BORDERW 0
// CGList
#define AUI_LIST_X 20
#define AUI_LIST_Y 20
#define AUI_LIST_FG_COLOR 0x333333
#define AUI_LIST_SZX 300
#define AUI_LIST_SZY 200
#define AUI_LIST_ARROWSZ1 15
#define AUI_LIST_ARROWSZ2 15
#define AUI_LIST_BG 0x999999
// CGTable
#define AUI_TABLE_X 20
#define AUI_TABLE_Y 20
#define AUI_TABLE_SZX 300
#define AUI_TABLE_SZY 200
#define AUI_TABLE_CELL_W 50
#define AUI_TABLE_CELL_H 18
#define AUI_TABLE_BG 0x999999
#define AUI_TABLE_INTERSEC_BG 0x888888
#define AUI_TABLE_SCROLL_THICK 5

// how much color will attempt to darken/lighten on highlight operation
// value must be less than 0x80
#define AUI_HL_SHIFT 20
//#define CG_DEFAULT_FONT "-*-helvetica-medium-r-*--*-120-100-100-*-*-iso8859-1"
#define AUI_DEFAULT_FONT "-*-*-*-R-Normal--18-*-100-100-*-*-iso8859-1"

enum class AUIWidgetType {
  unset = 0,
  defaultWindow = 1001,
  defaultButton = 1002,
  defaultList = 1003,
  defaultLabel = 1004,
  defaultInputBox = 1005,
  defaultTable = 1006
};

enum class AUIHAlign {
  center = 1,
  left = 2,
  right = 3
};

enum class AUIVAlign {
  center = 1,
  top = 2,
  bottom = 3
};

#define E(format, ...) {printf("Error|%d%s|%s(%d):" format "\n", 0, __FILE__, __func__, __LINE__, ##__VA_ARGS__);exit(1);}

#define D(format, ...) printf("%s|%s(%d):" format "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__);

#if DEBUG_LEVEL > 0
#define W() printf("W%s|%s(%d)\n", __FILE__, __func__, __LINE__);
#endif

#if DEBUG_LEVEL == 0
#define D1(...) {}
#define D2(...) {}
#define D3(...) {}
#define DS1(...) {}
#define DS2(...) {}
#define DS3(...) {}
#define W() {}
#endif

#if DEBUG_LEVEL == 1
#define D1(format, ...) printf("D%d%s|%s(%d):" format "\n", 1, __FILE__, __func__, __LINE__, ##__VA_ARGS__);
#define D2(...) {}
#define D3(...) {}
#define DS1(...) {D("\n---Trace at %s:%d---", __FILE__, __LINE__);print_stack();}
#define DS2(...) {}
#define DS3(...) {}
#endif

#if DEBUG_LEVEL == 2
#define D1(format, ...) printf("D%d%s|%s(%d):" format "\n", 1, __FILE__, __func__, __LINE__, ##__VA_ARGS__);
#define D2(format, ...) printf("D%d%s|%s(%d):" format "\n", 2, __FILE__, __func__, __LINE__, ##__VA_ARGS__);
#define D3(...) {}
#define DS1(...) {D("\n---Trace at %s:%d---", __FILE__, __LINE__);print_stack();}
#define DS2(...) {D("\n---Trace at %s:%d---", __FILE__, __LINE__);print_stack();}
#define DS3(...) {}
#endif

#if DEBUG_LEVEL == 3
#define D1(format, ...) printf("D%d%s|%s(%d):" format "\n", 1, __FILE__, __func__, __LINE__, ##__VA_ARGS__);
#define D2(format, ...) printf("D%d%s|%s(%d):" format "\n", 2, __FILE__, __func__, __LINE__, ##__VA_ARGS__);
#define D3(format, ...) printf("D%d%s|%s(%d):" format "\n", 3, __FILE__, __func__, __LINE__, ##__VA_ARGS__);
#define DS1(...) {D("\n---Trace at %s:%d---", __FILE__, __LINE__);print_stack();}
#define DS2(...) {D("\n---Trace at %s:%d---", __FILE__, __LINE__);print_stack();}
#define DS3(...) {D("\n---Trace at %s:%d---", __FILE__, __LINE__);print_stack();}
#endif

// -rdynamic must be added to linker options(and -g to compiler)
void print_stack();
#define DS() {D("\n---Trace at %s:%d---", __FILE__, __LINE__);print_stack();}

const static std::unordered_map<std::string,UINT64> string_to_case{
   {"BackSpace", 1},
   {"space", 2},
   {"Return", 3},
   {"KP_Enter", 4},
   {"Left", 5},
   {"Right", 6},
   {"Delete", 7}
};

static std::string BaseAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#endif
