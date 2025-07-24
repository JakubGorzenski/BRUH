/*  JS + WASM WRAPPERS  */
#include "bru.h"


#define v internal_bruh
struct {
    //  file_

    //  directory_

    //  bruh_
    bruh bruh;
    sprite screen;
    bruh_settings set;
} v = {
};



//  builtin compiler functions
void* memset(void* ptr, int value, unsigned long num) {
    for(ulong i = 0; i < num; i++) {
        ((char*)ptr)[i] = value;
    }
    return ptr;
}



//  functions from JS
void internal_bruh_test(int log);

void internal_bruh_startup_js(sint* buffer, int buffer_size);

void internal_bruh_resize(sint w, sint h);
sint internal_bruh_in_buffer_fill(void);

void internal_bruh_output(pixel* buffer, sint w, sint h);



//  functions to JS
int _start(int ms_time) {
    UNUSED(ms_time);

    static int state = 0;
    if(state == 0) {    //  set up
        internal_bruh_startup_js(v.bruh.in, 128);
    }
    {   //  generate frame
    {   //  keyboard
    for(uint i = KEY_MouseLeft; i <= KEY_ArrowDown; i++)
        v.bruh.in[i] += v.bruh.in[i] != 0;

    v.bruh.in[KEY_Mod] = 0;
    v.bruh.in[KEY_Mod] |= v.bruh.in[KEY_Shift] > 0 ? KEY_Shift : 0;
    v.bruh.in[KEY_Mod] |= v.bruh.in[KEY_Ctrl] > 0 ? KEY_Ctrl : 0;
    v.bruh.in[KEY_Mod] |= v.bruh.in[KEY_Alt] > 0 ? KEY_Alt : 0;
    }
    
    v.bruh.screen = v.screen;
    state = bruh_main(&v.bruh, state);  //  <== user code

    {   //  keyboard cleanup
    v.bruh.in[KEY_Pressed] = 0;
    v.bruh.in[KEY_Text] = 0;
    }
    }

    internal_bruh_output(v.screen.buffer, v.screen.size.width, v.screen.size.height);

    if(state == -1) {   //  clean up
        MemFree(v.bruh.screen.buffer);

        MemFree(v.bruh.audio[0].buffer);
        MemFree(v.bruh.audio[1].buffer);
        MemFree(v.bruh.audio[2].buffer);

        v.bruh = (bruh){0};
    }
    return state;
}



//  function definitions
void bruh_set(bruh* bruh, bruh_settings settings) {
    {   //  set defaults
    #define DEFAULT(setting_name, default) if(!settings.setting_name) v.set.setting_name = default

    v.set = settings;
    DEFAULT(scale, 1);
    DEFAULT(fps_cap, 30);

    #undef DEFAULT
    }
    //  prep time keeping 2
    {   //  update screen size
    if(v.set.resolution.all_bits) {
        MemFree(v.screen.buffer);
        v.screen.buffer = MemGet(v.set.resolution.w * v.set.resolution.h * sizeof(uint));
        if(v.screen.buffer)
            v.screen.size = v.set.resolution;
        else
            v.screen.size = v2di(0, 0);
        v.screen.real_width = v.screen.size.w;
    }
    bruh->screen = v.screen;

    internal_bruh_resize(v.set.resolution.width, v.set.resolution.height);
    }
}


void* MemGet(ulong size) {
    #define MAX_ALLOC (1024 * 1024 * 10)   //  10 MB of memory
    static uchar memory[MAX_ALLOC];
    static ulong ptr = 0;
    ulong old_ptr = ptr;
    ptr += size;
    if(ptr >= MAX_ALLOC)
        return NULL;
    return memory + old_ptr;
}
void  MemFree(void* memory) {
    UNUSED(memory);
}
void* MemTemp(ulong size) {
    UNUSED(size);
    return NULL;
}


bool directory_create(cstr name);
bool directory_delete(cstr name);

bool directory_open(cstr path);
uint directory_list(uint size, cstr buffer);


bool file_create(cstr name);
bool file_delete(cstr name);

bool file_open(cstr name);
void file_close(void);
//void file_swap(void** file);

void  file_move(slong offset);
void* file_read(ulong size, void* buffer);
bool  file_write(ulong size, void* buffer);

ulong FilePos(void);
ulong FileSize(void);


pixel Rgb3(uchar r, uchar g, uchar b) {
    return (pixel){(255 << 24) + (b << 16) + (g << 8) + (r << 0)};
}
uint  Color(pixel p) {
    uchar r = p.raw >> 0, g = p.raw >> 8, b = p.raw >> 16;
    return (r << 16) + (g << 8) + (b << 0);
}


double time_between_calls(bool set_zero);

#undef v
