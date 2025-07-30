/*  C GAME LIBRARY   */
//#include <stdint.h>


#ifndef CGL_LIB
#define CGL_LIB

//  macros
#define CGL_MAIN(cgl) cgl_main(cgl, sint cgl_state) { switch(cgl_state) case 0:
#define CGL_YIELD do {return __LINE__; case __LINE__:} while(0)
#define CGL_END return -1; }

#define CGL_ON_CLOSE case -1

#define CGL_ACKNOWLEDGE return 0
#define CGL_CLOSE return -1


#define UNUSED(var) (void)var
#define GET_PIXEL(spr, x, y) ((spr).buffer[(x) + (y) * (spr).real_width])

#define v2di(x, y) (v2di){{(x), (y)}}
#define v2diVV(a, op, b) (v2di){{(a.x) op (b.x), (a.y) op (b.y)}}
#define v2diVN(a, op, b) (v2di){{(a.x) op (b), (a.y) op (b)}}

#define StrCat(alloc, ...) StrCatList(alloc, sizeof((string[]){__VA_ARGS__})/sizeof(string), (string[]){__VA_ARGS__})

//#define v2df(x, y) (v2f){{x, y}}
//#define v2dfVV(a, op, b) v2f(a.x op b.x, a.y op b.y)
//#define v2dfVN(a, op, b) v2f(a.x op b, a.y op b)


#define NULL (void*)0

//  data types
typedef unsigned char       uchar;
typedef unsigned short      ushort;
typedef unsigned int        uint;
typedef unsigned long long  ulong;
typedef signed   char       schar;
typedef signed   short      sshort;
typedef signed   int        sint;
typedef signed   long long  slong;

typedef void*(*alloc)(ulong size);

typedef char*   cstr;



typedef struct {uint raw;} pixel;

typedef union {
    struct { sint x; sint y; };
    struct { sint w; sint h; };
    struct { sint width; sint height; };
    ulong all_bits;
} v2di;
typedef struct {
    char* buffer;
    sint  buffer_size;
    sint  length;
} string;

typedef struct {
    pixel* buffer;
    v2di   start;
    v2di   size;
    sint   real_width;
} sprite;
typedef struct {
    sint line_height;
    struct {
        sint width;
        sint offset_y;
        ulong data;
    } letter['~' - ' ' + 1];
} font;
typedef struct {
    float* buffer;
    uint buffer_size;
} sound;

typedef struct {
    cstr  window_title;
    v2di  resolution;
    float scale;
    uint  fps_cap;
    bool  hide_cursor;
    bool  debug_color;
} cgl_set;
typedef struct {
    font font;
    pixel color;
    bool wrap;
    bool process_special;
} text_set;


typedef struct cgl {
    //  outputs
    sprite screen;
    sound audio[3];

    //  inputs
    uint delta_ms;
    sint in[128];
    v2di mouse;
} cgl;

enum input_key_values {
    KEY_Text,   //  typed in char
    KEY_Mod,    //  input[KEY_Mod] == KEY_Shift | KEY_Ctrl | KEY_Alt

    KEY_MouseLeft, KEY_MouseMiddle, KEY_MouseRight,
    //KEY_MouseBack,
    //KEY_MouseForward,     //  6

    KEY_Backspace = '\b',   //  8
    KEY_Tab = '\t',         //  9
    KEY_Enter = '\n',       //  10
    KEY_F1 = 11, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_PrintScreen,        //  23
    KEY_ScrollLock,
    KEY_PauseBreak,
    KEY_CapsLock,
    KEY_Esc,
    KEY_Windows,            //  28

    KEY_Space = ' ',        //  32
    KEY_Insert,
    KEY_Home,
    KEY_PageUp,
    KEY_Delete,
    KEY_End,
    KEY_PageDown,
    KEY_NumLock,            //  39

    KEY_Shift = 0x29,       //  41  0b00101001
    KEY_Ctrl = 0x2a,        //  42  0b00101010
    KEY_Alt = 0x2c,         //  44  0b00101100
                            //  '-' 45 -> 'A' 65 -> 'Z' 90 -> ']' 93
                            //  '.' 46
                            //  '/' 47
                            //  '0' - '9' 48 - 57

                            //  ';' 59
    KEY_ArrowLeft = '<',    //  60
                            //  '=' 61
    KEY_ArrowRight = '>',   //  62
    KEY_Pressed = '?',      //  63 id of a pressed key

                            //  'A' - 'Z' 65 - 90
                            //  '[' 91
                            //  '\' 92
                            //  ']' 93
    KEY_ArrowUp = '^',      //  94
    KEY_NumDiv = 95,
    KEY_NumMul,
    KEY_NumSub,
    KEY_NumAdd,
    KEY_NumDel,             //  99
    KEY_Num0 = 100, KEY_Num1, KEY_Num2, KEY_Num3, KEY_Num4, KEY_Num5, KEY_Num6, KEY_Num7, KEY_Num8, KEY_Num9,
                            //  110

    KEY_ArrowDown = 'v',    //  118
};



//  cgl_os.c
void cgl_settings(cgl* cgl, cgl_set settings);
//cgl_set cgl_available_settings();  //  implement somthing to allow for platform dependent bechavior



void* MemGet(ulong size);
void  MemFree(void* memory);
void* MemTemp(ulong size);


bool directory_create(cstr path);
bool directory_delete(cstr path);

bool directory_open(cstr path);
uint directory_list(uint size, cstr buffer);


bool file_create(cstr path);
bool file_delete(cstr path);

bool file_open(cstr path);
void file_close(void);
void file_swap(void** file);

void  file_move(slong offset);
void* file_read(ulong size, void* buffer);
bool  file_write(ulong size, void* buffer);

ulong FilePos(void);
ulong FileSize(void);


pixel Rgb3(uchar r, uchar g, uchar b);
uint  RgbFrom(pixel p);


double time_between_calls_ms(bool set_zero);



//  cgl.c
pixel Rgb(uint rgb);


string StrC(char* cstr);
string StrInt(slong n);
string StrFloat(double f);
string StrV2di(v2di p);

string StrNew(alloc alloc, int size);
string StrAppend(string* out, string append);
string StrCatList(alloc alloc, uint count, string* str);
string StrCut(string* str, sint pos);
string StrSub(string str, sint start, sint length);

sint StrFind(string str, string look_for);
bool StrIsEqual(string a, string b);


slong IntMin(slong a, slong b);
slong IntMax(slong a, slong b);
slong IntClamp(slong min, slong val, slong max);


v2di v2diMin(v2di a, v2di b);
v2di v2diMax(v2di a, v2di b);
v2di v2diClamp(v2di min, v2di val, v2di max);
bool v2diIsEqual(v2di a, v2di b);
bool v2diIsInside(v2di p, v2di pos, v2di size);


sprite SprMove(sprite spr, v2di pos);
sprite SprSize(sprite spr, v2di size);
sprite SprWin(sprite spr, v2di pos, v2di size);

sprite SprCutL(sprite* spr, sint cut_by);
sprite SprCutR(sprite* spr, sint cut_by);
sprite SprCutT(sprite* spr, sint cut_by);
sprite SprCutB(sprite* spr, sint cut_by);


void draw_pixel(sprite out, v2di pos, pixel color);
void draw_fill(sprite out, pixel color);
void draw_spr(sprite out, sprite in);
v2di draw_text(sprite out, cstr text, text_set* settings);
sint draw_char(sprite out, char ch, pixel color, font* used_font);



//  user entry point
sint cgl_main(cgl* cgl, sint state);

#endif
