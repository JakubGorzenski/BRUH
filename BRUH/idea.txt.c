
       |\
     <(.;)
~<,;7  //_)~

    TODO:

->  better file system
->  change key value from frames to ms
->  reimplement file listing
->  fix StrCatList() size calculation
->  make -1 default message response, and -2 close;
->  change v2diIsInside() args order ?
->  error stack
->  audio_synth()
    > noise / sine / triangle / square / saw tooth
    > LFO ?
    > volume / frequency
    > fade-in / -out
    > //www.youtube.com/watch?v=Le7OHZJ20pE&ab_channel=RyanHedgecock
->  parallel(func, void* arg)
->  order functions logically
->  cleanup function names
->  link with libc / use asm for faster math
->  add static settings
->  change main() to MainCRTStartup()
->  return flow control to the user ?
->  change type* x; to type *x;

X>  make file io async
X>  add file_transfer_remaining()

    IDEAS:

typedef struct {
    sint id;
    string temp_error_desc;
} error;

error_start_tracking();

while(error e = error_pop()) {
    
}




string *file = file_load(MemTemp, "file_name", size);

file = &StrC("Fetching...");

file.buffer_size == 0
file.length >= 0

... some yields later ...

    on success:
file.buffer_size > 0
file.length > 0

    on file not found:
file.buffer_size >= 0
file.length == 0


file_save(file, StrC("file1.txt"));

file_save_copy(string file, string file_name);

file_transfer_status(void);
-1 => error found
 0 => no files in transfer
 1 => files in transfer



sint internal_bruh_register_handler(sint loc) {
    static sint saved_loc;

    sint priv_loc = saved_loc;
    saved_loc = loc;

    return priv_loc;
}

bool internal_bruh_once(void) {
    static bool state;

    state = !state;
    return state;
}

//  https://www.chiark.greenend.org.uk/~sgtatham/mp/
#define FILE_LOAD(file, perm_alloc, min_size, file_name)
        if(1) {
            internal_bruh_register_handler(__LINE__);
            file_load(perm_alloc, min_size, file_name);
        } else
            case __LINE__:
            for(file = internal_bruh_get_file(); internal_bruh_once(); )


static string file;
FILE_LOAD(file, MemGet, 0, StrC("test.txt"));


string *file1 = file_load(...);
string *file2 = file_load(...);
string *file3 = file_load(...);
string *file4 = file_load(...);
string *file5 = file_load(...);

while(file_in_transfer() > 0) BRUH_YIELD;


//  MemArenaMake()
#define INTERNAL_BRUH_X_16 X(00) X(01) X(02) X(03) X(04) X(05) X(06) X(07) X(08) X(09) X(10) X(11) X(12) X(13) X(14) X(15)

#define X(num) void* internal_bruh_MemArena_ ## num(sint size) { return internal_bruh_MemArena(num, size); }

INTERNAL_BRUH_X_16
#undef X
#define X(num)  if(!v.MemPass[num].size) {\
                    v.MemPass[num].buffer = memory;\
                    v.MemPass[num].size = size;\
                    v.MemPass[num].used = 0;\
                    return internal_bruh_MemArena_ ## num; }

alloc MemArenaMake(void* memory, sint size) {
    if(!memory)
        return MemNull;
    INTERNAL_BRUH_X_16
    return MemNull;
}
#undef X
#define X(num)  if(MemAreana == internal_bruh_MemArena_ ## num) {\
                    v.MemPass[num].size = 0;\
                    return v.MemPass[num].used; }

sint MemArenaDestroy(alloc MemArena, void* memory) {
    if(memory)
        for(sint i = 0; i < 16; i++)
            if(v.MemPass[i].buffor == memory) {
                v.MemPass[i].size = 0;
                return v.MemPass[i].used;
            }
    INTERNAL_BRUH_X_16
    return -1;
}
#undef X

void* MemNull(sint size) {
    (void)size;
    return NULL;
}

void* internal_bruh_MemArena(sint num, sint size) {
    if(size < 0)
        return NULL;
    if(size & 0x07)
        size = (size & 0xfffffff8) + 8

    if(v.MemPass[num].used > 0x0fffffff)
        return NULL;

    void* ret = v.MemPass[num].buffer + v.MemPass[num].used;
    v.MemPass[num].used += size;

    if(v.MemPass[num].used + (unsigned)size > v.MemPass[num].size)
        return NULL;
    return ret;
}

#undef INTERNAL_BRUH_X_16

//  bruh_win.c
internal_file_backend() {
    type.file_load
    type.file_save
    type.file_save_copy
    type.file_append
    type.file_append_copy
}


/*

file structure:

ROM/    <- cannot list this directory, blame js
    main.wasm
    manual_config.txt
    other_stuff...
saves/
    save1.game
    save2.game
    save3.game
settings.txt
mods/
    mod_a.ld
    mod_b.ld
    mod_c.ld

*/


