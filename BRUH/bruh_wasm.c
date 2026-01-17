/*  JS + WASM WRAPPERS  */
#include "bru.h"



#define v_PAGE_HEADER internal_bruh_page_header
typedef struct v_PAGE_HEADER {
    struct v_PAGE_HEADER* next;
    uint size;
} v_PAGE_HEADER;


#define v internal_bruh_platform
struct {
    //  file_

    //  directory_

    //  Mem
    const uint page_size;
    v_PAGE_HEADER* memory;

    uchar* MemTemp_buffer;
    uint   MemTemp_size;
    uint   MemTemp_ptr;
    //  bruh_
    bruh bruh;
    sprite screen;
    bruh_set set;
    //sint msg_to_user;   //  not used currently
} v = {
    .page_size = 0x110,
};







//  functions from JS
void internal_bruh_print(char* str, sint length);

void internal_bruh_js_init(sint* buffer, sint buffer_size);

int internal_bruh_file_set_target(char* name, sint length);
int internal_bruh_file_load(char* buffer);
int internal_bruh_file_save(char* save, sint length);
int internal_bruh_file_append(char* append, sint length);
int internal_bruh_file_delete(void);

void internal_bruh_output(pixel* buffer, sint w, sint h);



//  functions to JS
#define v_EXPORT __attribute__((visibility("default")))

v_EXPORT
void internal_bruh_wasm_init(int heap_start_in_pages, int heap_size_in_pages) {
    v.memory = (v_PAGE_HEADER*)(heap_start_in_pages * 65536);

    v.memory[0] = (v_PAGE_HEADER){.next = &v.memory[1]};
    v.memory[1] = (v_PAGE_HEADER){.size = heap_size_in_pages * 65536 - sizeof(v_PAGE_HEADER)};
    
    internal_bruh_js_init(v.bruh.in, 128);

    //  1 GB for scratch memory (maybe make it dynamically allocated, since we only have 4 GB on wasm)
    v.MemTemp_buffer = MemGet(1024 * 1024);
    v.MemTemp_size = 1024 * 1024;

    internal_bruh_init();
}
v_EXPORT
int _start(int delta_ms) {
    static sint state = 0;
    v.MemTemp_ptr = 0;

    v.bruh.delta_ms = delta_ms;
    {   //  key_mod update
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
    {   //  keyboard update
    for(uint i = KEY_MouseLeft; i <= KEY_ArrowDown; i++) {
        if(v.bruh.in[i] > 0)
            v.bruh.in[i] += delta_ms;
        else
            v.bruh.in[i] = 0;
    }

    v.bruh.in[KEY_Pressed] = 0;
    v.bruh.in[KEY_Text] = 0;
    }

    internal_bruh_output(v.screen.buffer, v.screen.size.width, v.screen.size.height);

    return state;
}
#undef v_EXPORT



//  function definitions
void bruh_settings(bruh* bruh, bruh_set settings) {
    {   //  set defaults
    #define DEFAULT(setting_name, default) if(!settings.setting_name) v.set.setting_name = default

    v.set = settings;

    #undef DEFAULT
    }
    //  prep time keeping 2
    {   //  update screen size
    if(v.set.resolution.all_bits) {
        MemFree(v.screen.buffer);
        v.screen = SprNew(MemGet, v.set.resolution);
    }
    bruh->screen = v.screen;
    }
}
bruh_set bruh_available_settings() {
    return (bruh_set){0};
}



void* MemGet(sint size) {
    if(size <= 0)
        return NULL;

    v_PAGE_HEADER *priv, *curr = v.memory;
    size += sizeof(v_PAGE_HEADER);
    size = ((size - 1) / v.page_size + 1) * v.page_size;    //  round up size to v.page_size

    while(curr->size < (uint)size) {  //  find node which can fit the allocation
        priv = curr;
        if(curr->next)
            curr = curr->next;
        else
            return NULL;
    }

    priv->next += size / sizeof(v_PAGE_HEADER);
    curr->size -= size;
    if(curr->size > 1)
        *(priv->next) = *curr;  //  make new node
    else
        priv->next = curr->next;

    *curr = (v_PAGE_HEADER){.size = size};
    return curr + 1;
}
void  MemFree(void* memory) {
    if(!memory)
        return;
    
    v_PAGE_HEADER* curr = memory;
    curr -= 1;

    v_PAGE_HEADER *priv, *next = v.memory;
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
    if(curr + curr->size / sizeof(v_PAGE_HEADER) == next) {   //  merge with next node if adjacent
        curr->size += next->size;
        curr->next = next->next;

        *next = (v_PAGE_HEADER){0};
    }
    if(curr == priv + priv->size / sizeof(v_PAGE_HEADER)) {   //  merge with previous node if adjacent
        priv->size += curr->size;
        priv->next = curr->next;

        *curr = (v_PAGE_HEADER){0};
    }
}
void* MemTemp(sint size) {
    if(size <= 0)
        return NULL;

    size = ((size - 1) / sizeof(ulong) + 1) * sizeof(ulong);    //  round up to ulong aligment

    if(v.MemTemp_ptr + size > v.MemTemp_size)
        return NULL;

    uchar* ret = v.MemTemp_buffer + v.MemTemp_ptr;
    v.MemTemp_ptr += size;
    return ret;
}


string file_load(alloc alloc, sint min_size, string file_name) {
    sint size = internal_bruh_file_set_target(file_name.buffer, file_name.length);
    if(size < 0)
        return StrC("");
    
    string ret = StrNew(alloc, IntMax(size, min_size));

    if(internal_bruh_file_load(ret.buffer))
        ret.length = size;
    return ret;
}
bool   file_save(string save, string file_name) {
    sint size = internal_bruh_file_set_target(file_name.buffer, file_name.length);
    if(size < 0)
        return false;
    return !!internal_bruh_file_save(save.buffer, save.length);
}
bool   file_append(string append, string file_name) {
    sint size = internal_bruh_file_set_target(file_name.buffer, file_name.length);
    if(size < 0)
        return false;
    return !!internal_bruh_file_append(append.buffer, append.length);
}
bool   file_delete(string file_name) {
    sint size = internal_bruh_file_set_target(file_name.buffer, file_name.length);
    if(size < 0)
        return false;
    return !!internal_bruh_file_delete();
}


pixel Rgb3(uchar r, uchar g, uchar b) {
    return (pixel){(255 << 24) + (b << 16) + (g << 8) + (r << 0)};
}
uint  RgbFrom(pixel p) {
    uchar r = p.raw >> 0, g = p.raw >> 8, b = p.raw >> 16;
    return (r << 16) + (g << 8) + (b << 0);
}


double time_between_calls(bool set_zero);
void   debug_print(string str) {
    internal_bruh_print(str.buffer, str.length);
}


#undef v
#undef v_PAGE_HEADER
