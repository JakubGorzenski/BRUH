/*  WINDOWS WRAPPERS  */
#include <windows.h>
#include <stdio.h>
#include "bru.h"



#define v internal_bruh_platform
static struct {
    //  file_
    cstr exe_path;

    //  MemTemp
    uchar* MemTemp_buffer;
    ulong  MemTemp_size;
    ulong  MemTemp_ptr;

    //  bruh_
    bruh bruh;
    bruh_set set;
    sint msg_to_user;

    bool cold_start;
    HWND window;

    ulong counter_freq;
    ulong frame_delta;
    sprite screen;

    v2di margin;
    uchar key_translation[256];
} v = {
    .bruh = (bruh){0},
    .cold_start = true,
    .key_translation = {
        [VK_OEM_3] = '`',
        [VK_OEM_MINUS] = '-',
        [VK_OEM_PLUS] = '=',    //  ?
        [VK_OEM_4] = '[',
        [VK_OEM_6] = ']',
        [VK_OEM_5] = '\\',
        [VK_OEM_1] = ';',
        [VK_OEM_7] = '\'',
        [VK_OEM_COMMA] = ',',
        [VK_OEM_PERIOD] = '.',
        [VK_OEM_2] = '/',

        [VK_RETURN] = KEY_Enter,
        [VK_SPACE] = KEY_Space,

        [VK_SHIFT] = KEY_Shift,
        [VK_CONTROL] = KEY_Ctrl,
        [VK_MENU] = KEY_Alt,
        [VK_LWIN] = KEY_Windows,

        [VK_F1] = KEY_F1,
        [VK_F2] = KEY_F2,
        [VK_F3] = KEY_F3,
        [VK_F4] = KEY_F4,
        [VK_F5] = KEY_F5,
        [VK_F6] = KEY_F6,
        [VK_F7] = KEY_F7,
        [VK_F8] = KEY_F8,
        [VK_F9] = KEY_F9,
        [VK_F10] = KEY_F10,
        [VK_F11] = KEY_F11,
        [VK_F12] = KEY_F12,

        [VK_SNAPSHOT] = KEY_PrintScreen,    //  ?   windows doesn't pass this key
        [VK_SCROLL] = KEY_ScrollLock,
        [VK_PAUSE] = KEY_PauseBreak,

        [VK_CAPITAL] = KEY_CapsLock,
        [VK_ESCAPE] = KEY_Esc,

        [VK_INSERT] = KEY_Insert,
        [VK_HOME] = KEY_Home,
        [VK_PRIOR] = KEY_PageUp,
        [VK_DELETE] = KEY_Delete,
        [VK_END] = KEY_End,
        [VK_NEXT] = KEY_PageDown,

        [VK_LEFT] = KEY_ArrowLeft,
        [VK_RIGHT] = KEY_ArrowRight,
        [VK_UP] = KEY_ArrowUp,
        [VK_DOWN] = KEY_ArrowDown,

        [VK_NUMPAD0] = KEY_Num0,
        [VK_NUMPAD1] = KEY_Num1,
        [VK_NUMPAD2] = KEY_Num2,
        [VK_NUMPAD3] = KEY_Num3,
        [VK_NUMPAD4] = KEY_Num4,
        [VK_NUMPAD5] = KEY_Num5,
        [VK_NUMPAD6] = KEY_Num6,
        [VK_NUMPAD7] = KEY_Num7,
        [VK_NUMPAD8] = KEY_Num8,
        [VK_NUMPAD9] = KEY_Num9,

        [VK_NUMLOCK] = KEY_NumLock,
        [VK_DIVIDE] = KEY_NumDiv,
        [VK_MULTIPLY] = KEY_NumMul,
        [VK_SUBTRACT] = KEY_NumSub,
        [VK_ADD] = KEY_NumAdd,
        [VK_DECIMAL] = KEY_NumDel,
    },
};

bool dbg = false;

static LRESULT internal_bruh_win_proc(HWND win, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if(dbg)
        printf("%x\n", Msg);
    switch (Msg) {
      case WM_SIZE: {
        if(v.set.resolution.all_bits) {
            float w_scale = (float)LOWORD(lParam) / (float)v.set.resolution.width;
            float h_scale = (float)HIWORD(lParam) / (float)v.set.resolution.height;
            v.set.scale = w_scale > h_scale ? h_scale : w_scale;
        } else {
            v2di new_size = {
                .w = LOWORD(lParam) / v.set.scale,
                .h = HIWORD(lParam) / v.set.scale,
            };
            MemFree(v.screen.buffer);
            v.screen = SprNew(MemGet, new_size);
        }
        v.margin.x = (LOWORD(lParam) - v.screen.size.width * v.set.scale) / 2;
        v.margin.y = (HIWORD(lParam) - v.screen.size.height * v.set.scale) / 2;
        return 0;
    } case WM_LBUTTONDOWN: {
        v.bruh.in[KEY_MouseLeft] = 1;
        v.bruh.in[KEY_Pressed] = KEY_MouseLeft;
        return 0;
      case WM_LBUTTONUP:
        v.bruh.in[KEY_MouseLeft] = -1;
        return 0;
      case WM_MBUTTONDOWN:
        v.bruh.in[KEY_MouseMiddle] = 1;
        v.bruh.in[KEY_Pressed] = KEY_MouseMiddle;
        return 0;
      case WM_MBUTTONUP:
        v.bruh.in[KEY_MouseMiddle] = -1;
        return 0;
      case WM_RBUTTONDOWN:
        v.bruh.in[KEY_MouseRight] = 1;
        v.bruh.in[KEY_Pressed] = KEY_MouseRight;
        return 0;
      case WM_RBUTTONUP:
        v.bruh.in[KEY_MouseRight] = -1;
        return 0;
    } case WM_KEYDOWN: {
      case WM_SYSKEYDOWN:
        uchar key = v.key_translation[wParam];
        if(lParam & 1 << 30)
            return 0;
        switch(key) {
        case 0:
            return 0;
        case KEY_Alt:
        case KEY_Ctrl:
        case KEY_Shift:
            v.bruh.in[KEY_Mod] |= key;
            break;
        default:
            break;
        }
        v.bruh.in[key] = 1;
        v.bruh.in[KEY_Pressed] = key;
        return 0;
    } case WM_KEYUP: {
      case WM_SYSKEYUP:
        uchar key = v.key_translation[wParam];
        switch(key) {
        case 0:
            return 0;
        case KEY_Alt:
        case KEY_Ctrl:
        case KEY_Shift:
            v.bruh.in[KEY_Mod] &= ~key;
            if(v.bruh.in[KEY_Mod])
                v.bruh.in[KEY_Mod] |= 0x70;
            break;
        default:
            break;
        }
        v.bruh.in[key] = -1;
        return 0;
    } case WM_CHAR: {
        switch(wParam) {
        case '\b':
            v.bruh.in[KEY_Text] = KEY_Backspace;
            break;
        case '\t':
            v.bruh.in[KEY_Text] = KEY_Tab;
            break;
        case '\r':
            v.bruh.in[KEY_Text] = KEY_Enter;
            break;
        default:
            if(wParam >= ' ' && wParam <= '~')
                v.bruh.in[KEY_Text] = wParam;
            break;
        }
        return 0;
    } case WM_CLOSE: {
        v.msg_to_user = -1;
        return 0;
    } default: {
        return DefWindowProcA(win, Msg, wParam, lParam);
    }}
}



int main() {
    {   //  start
    LARGE_INTEGER counter_freq = {0};
    QueryPerformanceFrequency(&counter_freq);
    v.counter_freq = counter_freq.QuadPart;


    DWORD file_name_size = GetCurrentDirectory(0, NULL);
    v.exe_path = MemGet(file_name_size);
    GetCurrentDirectory(file_name_size, v.exe_path);


    for(int i = '\b'; i <= ']'; i++) {
        if(!v.key_translation[i])
            v.key_translation[i] = i;
    }


    WNDCLASSA win_class = {
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = &internal_bruh_win_proc,
        .hInstance      = GetModuleHandleA(NULL),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = CreateSolidBrush(RGB(0, 0, 0)),
        .lpszClassName  = "win_class",
    };
    RegisterClassA(&win_class);   // can error

    win_class.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
    win_class.lpszClassName = "win_class_dbg";
    RegisterClassA(&win_class);   // can error

    //  1 GB for scratch memory
    v.MemTemp_buffer = MemGet(1024 * 1024);
    v.MemTemp_size = 1024 * 1024;

    internal_bruh_init();
    }
    sint state = 0;
    while(state != -1) {
        v.MemTemp_ptr = 0;
        {   //  keyboard + msg
        v.bruh.in[KEY_Pressed] = 0;
        v.bruh.in[KEY_Text] = 0;

        for(uint i = KEY_MouseLeft; i <= KEY_ArrowDown; i++) {
            if(v.bruh.in[i] > 0)
                v.bruh.in[i] += v.bruh.delta_ms;
            else
                v.bruh.in[i] = 0;
        }

        MSG msg;
        while(PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        v.bruh.screen = v.screen;
        }
        {   //  mouse position
        POINT p;
        GetCursorPos(&p);
        ScreenToClient(v.window, &p);

        v2di pc_mouse = v2diVV(p, -, v.margin);
        v.bruh.mouse = v2diVN(pc_mouse, /, v.set.scale);
        v.bruh.mouse.x -= pc_mouse.x < 0;
        v.bruh.mouse.y -= pc_mouse.y < 0;


        bool show_cursor = !v.set.hide_cursor;
        if(v2diVN(pc_mouse, <, 0).all_bits)
            show_cursor = true;
        if(v2diVV(v.bruh.mouse, >=, v.screen.size).all_bits)
            show_cursor = true;

        static sint cursor = 0;
        if(cursor + 1 != show_cursor)
            cursor = ShowCursor(show_cursor);
        }
        {   //  run user code
        sint state_out = 0;
        if(v.msg_to_user) {
            state_out = bruh_main(&v.bruh, v.msg_to_user);
            v.msg_to_user = 0;
        }
        if(!state_out)
            state_out = bruh_main(&v.bruh, state);
        state = state_out;
        }
        {   //  wait for frame
        static ulong past_time = 0;
        static ulong dt_error = 0;

        LARGE_INTEGER current_time = {0};
        QueryPerformanceCounter(&current_time);

        ulong target_time = past_time + v.frame_delta;

        slong sleep_ms = (slong)(target_time - current_time.QuadPart) * 1000.0 / v.counter_freq;

        if(sleep_ms > 1)
            Sleep(sleep_ms - 1);
        while((ulong)current_time.QuadPart <= target_time)
            QueryPerformanceCounter(&current_time);

        v.bruh.delta_ms = (slong)(current_time.QuadPart - past_time + dt_error) * 1000.0 / v.counter_freq;
        dt_error += current_time.QuadPart - past_time;
        dt_error -= v.bruh.delta_ms * v.counter_freq / 1000;

        past_time = current_time.QuadPart;
        }
        {   //  output
        BITMAPINFO bm_info = {
            .bmiHeader = {
                .biSize = sizeof(bm_info.bmiHeader),
                .biWidth = v.screen.size.width,
                .biHeight = -v.screen.size.height,  //  negative bc windows wierd
                .biBitCount = 32,
                .biCompression = BI_RGB,
                .biPlanes = 1,
        }};
        HDC DC = GetDC(v.window);

        StretchDIBits(
            DC,
            v.margin.x, v.margin.y, v.screen.size.w * v.set.scale, v.screen.size.h * v.set.scale,
            0, 0, v.screen.size.w, v.screen.size.h,
            v.screen.buffer, &bm_info, DIB_RGB_COLORS, SRCCOPY);
        ReleaseDC(v.window, DC);
        }
    }
    {   //  start clean up
    DestroyWindow(v.window);    //  can error

    UnregisterClassA("win_class", GetModuleHandleA(NULL));  //  can error
    UnregisterClassA("win_class_dbg", GetModuleHandleA(NULL));  //  can error
    }
    return 0;
}



void bruh_settings(bruh* bruh, bruh_set settings) {
    {   //  load settings
    #define DEFAULT(setting_name, default) if(!settings.setting_name) v.set.setting_name = default

    v.set = settings;
    DEFAULT(window_title, "bruh_application");
    DEFAULT(scale, 1);
    DEFAULT(fps_cap, 60);

    #undef DEFAULT
    }
    {   //  warm start
    v.frame_delta = v.counter_freq / v.set.fps_cap;


    if(v.set.resolution.all_bits) {
        MemFree(v.screen.buffer);
        v.screen = SprNew(MemGet, v.set.resolution);
    }
    bruh->screen = v.screen;


    if(v.window)
        DestroyWindow(v.window);    //  can error
    v.window = CreateWindowExA(    // can error
        0,
        v.set.debug_color ? "win_class_dbg" : "win_class",
        v.set.window_title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,   //WS_CHILD | WS_VISIBLE

        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,
        NULL,
        GetModuleHandleA(NULL),
        NULL);
    }
}



void* MemGet(sint size) {
    if(size <= 0)
        return NULL;
    return malloc(size);
}
void  MemFree(void* memory) {
    free(memory);
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
    if(file_name.length + 1 >= MAX_PATH)
        return StrC("");

    while(*file_name.buffer == '/') {
        file_name.buffer++;
        file_name.length--;
        file_name.buffer_size--;
    }

    if(file_name.length <= 0 || file_name.buffer[file_name.length - 1] == '/') {
        sint length = -1;
        cstr find_path = STR_CAT(MemTemp, file_name, StrC("*")).buffer;

        WIN32_FIND_DATA find = {0};
        HANDLE find_handle = FindFirstFileA(find_path, &find);
        if(find_handle == INVALID_HANDLE_VALUE)
            return StrC("");

        do {
            if(find.cFileName[0] == '.')
                continue;

            length += file_name.length;
            length += StrC(find.cFileName).length;
            if(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                length += 2;
            else
                length += 1;
        } while(FindNextFileA(find_handle, &find));

        string ret = StrNew(alloc, IntMax(length, min_size));

        find_handle = FindFirstFileA(find_path, &find);
        if(find_handle == INVALID_HANDLE_VALUE)
            return StrC("");

        do {
            if(find.cFileName[0] == '.')
                continue;

            StrAppend(&ret, file_name);
            StrAppend(&ret, StrC(find.cFileName));
            if(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                StrAppend(&ret, (string){.buffer = "/", .length = 2});
            else
                StrAppend(&ret, (string){.buffer = "", .length = 1});
        } while(FindNextFileA(find_handle, &find));

        return ret;
    } else {    //  file read
        HANDLE file = CreateFileA(StrToCstr(file_name), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if(file == INVALID_HANDLE_VALUE)
            return StrC("");

        LARGE_INTEGER size = {0};
        if(!GetFileSizeEx(file, &size))
            return StrC("");
        if(size.QuadPart >= 0x7fffffff)
            return StrC("");

        string ret = StrNew(alloc, IntMax(size.QuadPart, min_size));
        DWORD len;
        if(ReadFile(file, ret.buffer, ret.buffer_size, &len, NULL))
            len = 0;
        
        ret.length = len;
        return ret;
    }
}
bool   file_save(string save, string file_name) {
    if(file_name.length + 1 >= MAX_PATH)
        return false;

    HANDLE file = CreateFileA(StrToCstr(file_name), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file == INVALID_HANDLE_VALUE)
        return false;

    DWORD bytes_written;
    WriteFile(file, save.buffer, save.length, &bytes_written, NULL);
    CloseHandle(file);
    return (slong)bytes_written == (slong)save.length;
}
bool   file_append(string append, string file_name) {   //  slow implementation for now
    if(file_name.length + 1 >= MAX_PATH)
        return false;

    HANDLE file = CreateFileA(StrToCstr(file_name), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file == INVALID_HANDLE_VALUE)
        return false;

    SetFilePointer(file, 0, NULL, FILE_END);
    DWORD bytes_written;
    WriteFile(file, append.buffer, append.length, &bytes_written, NULL);
    CloseHandle(file);
    return (slong)bytes_written == (slong)append.length;
}
bool   file_delete(string file_name) {
    if(file_name.length + 1 >= MAX_PATH)
        return false;

    return DeleteFileA(StrToCstr(file_name)) != 0;
}


pixel Rgb3(uchar r, uchar g, uchar b) {
    return (pixel){(r << 16) + (g << 8) + (b << 0)};
}
uint  RgbFrom(pixel p) {
    return p.raw;
}


double time_between_calls_ms(bool set_zero) {
    static LARGE_INTEGER start_tick = {0};
    LARGE_INTEGER tick = {0};
    QueryPerformanceCounter(&tick);

    double ret = (ulong)tick.QuadPart - (ulong)start_tick.QuadPart;
    ret *= 1000.0 / (double)v.counter_freq;

    if(set_zero)
        QueryPerformanceCounter(&start_tick);

    return ret;
}



#undef v
