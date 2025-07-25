/*  JS + WASM WRAPPERS  */
#include "bru.h"



#define v_page_header internal_bruh_page_header
typedef struct v_page_header {
    uint size;
    struct v_page_header* next;
} v_page_header;


#define v internal_bruh
struct {
    //  file_

    //  directory_
    //  Mem
    const uint page_size;
    //                   0x100000 - 1 MB
    v_page_header memory[0x110000 / sizeof(v_page_header) + sizeof(v_page_header)];
    //  bruh_
    bruh bruh;
    sprite screen;
    bruh_settings set;
    //sint msg_to_user;   //  not used currently
} v = {
    .page_size = 0x110,
    .memory = {
        [0] = {.next = &v.memory[1]},
        [1] = {.size = sizeof(v.memory) - sizeof(v_page_header)},
    },
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

    static sint state = 0;
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
    {   //  run user code
    v.bruh.screen = v.screen;
    sint state_out = 0;
    /*if(v.msg_to_user) {
        state_out = bruh_main(&v.bruh, v.msg_to_user);
        v.msg_to_user = 0;
    }*/
    if(!state_out)
        state_out = bruh_main(&v.bruh, state);
    state = state_out;
    }
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
    }
    return state;
}



//  function definitions
void bruh_set(bruh* bruh, bruh_settings settings) {
    {   //  set defaults
    #define DEFAULT(setting_name, default) if(!settings.setting_name) v.set.setting_name = default

    v.set = settings;

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
bruh_settings bruh_available_settings() {
    return (bruh_settings){0};
}




void* MemGet(uint size) {
    if(size == 0)
        return NULL;

    v_page_header *priv, *curr = v.memory;
    size += sizeof(v_page_header);
    size = ((size - 1) / v.page_size + 1) * v.page_size;    //  round up size to v.page_size

    while(curr->size < size) {  //  find node which can fit the allocation
        priv = curr;
        if(curr)
            curr = curr->next;
        else
            return NULL;
    }

    priv->next += size / sizeof(v_page_header);
    curr->size -= size;
    if(curr->size > 1)
        *(priv->next) = *curr;  //  make new node
    else
        priv->next = curr->next;

    *curr = (v_page_header){.size = size};
    return curr + 1;
}
void  MemFree(void* memory) {
    if(!memory)
        return;
    
    v_page_header* curr = memory;
    curr -= 1;

    v_page_header *priv, *next = v.memory;
    while(curr > next) {    //  find nodes surrounding current node
        priv = next;
        if(next->next)
            next = next->next;
        else
            next = curr;
    }

    //  insert current node
    priv->next = curr;
    curr->next = next;
    if(curr + curr->size / sizeof(v_page_header) == next) {   //  merge with next node if adjacent
        curr->size += next->size;
        curr->next = next->next;

        *next = (v_page_header){0};
    }
    if(curr == priv + priv->size / sizeof(v_page_header)) {   //  merge with previous node if adjacent
        priv->size += curr->size;
        priv->next = curr->next;

        *curr = (v_page_header){0};
    }
}
void* MemTemp(uint size) {
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
