/*  WINDOWS WRAPPERS  */
#include "cgl.h"
#include <windows.h>
#include <stdio.h>



#define v internal_cgl
struct {
    //  file_
    HANDLE open_file;

    //  directory_
    HANDLE find_handle;
    cstr exe_path;

    //  MemTemp
    uchar* MemTemp_buffer;
    ulong  MemTemp_size;
    ulong  MemTemp_ptr;

    //  cgl_
    cgl cgl;
    cgl_set set;
    sint msg_to_user;

    bool cold_start;
    HWND window;

    ulong counter_freq;
    ulong frame_delta;
    sprite screen;

    v2di margin;
    uchar key_translation[256];
} v = {
    .cgl = (cgl){0},
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

LRESULT internal_cgl_win_proc(HWND win, UINT Msg, WPARAM wParam, LPARAM lParam) {
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
            v.screen.buffer = MemGet(new_size.w * new_size.h * sizeof(uint));
            if(v.screen.buffer)
                v.screen.size = new_size;
            else
                v.screen.size = v2di(0, 0);
            v.screen.real_width = v.screen.size.w;
        }
        v.margin.x = (LOWORD(lParam) - v.screen.size.width * v.set.scale) / 2;
        v.margin.y = (HIWORD(lParam) - v.screen.size.height * v.set.scale) / 2;
        return 0;
    } case WM_LBUTTONDOWN: {
        v.cgl.in[KEY_MouseLeft] = 1;
        v.cgl.in[KEY_Pressed] = KEY_MouseLeft;
        return 0;
    case WM_LBUTTONUP:
        v.cgl.in[KEY_MouseLeft] = -1;
        return 0;
    case WM_MBUTTONDOWN:
        v.cgl.in[KEY_MouseMiddle] = 1;
        v.cgl.in[KEY_Pressed] = KEY_MouseMiddle;
        return 0;
    case WM_MBUTTONUP:
        v.cgl.in[KEY_MouseMiddle] = -1;
        return 0;
    case WM_RBUTTONDOWN:
        v.cgl.in[KEY_MouseRight] = 1;
        v.cgl.in[KEY_Pressed] = KEY_MouseRight;
        return 0;
    case WM_RBUTTONUP:
        v.cgl.in[KEY_MouseRight] = -1;
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
            v.cgl.in[KEY_Mod] |= key;
            break;
        default:
            break;
        }
        v.cgl.in[key] = 1;
        v.cgl.in[KEY_Pressed] = key;
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
            v.cgl.in[KEY_Mod] &= ~key;
            if(v.cgl.in[KEY_Mod])
                v.cgl.in[KEY_Mod] |= 0x70;
            break;
        default:
            break;
        }
        v.cgl.in[key] = -1;
        return 0;
    } case WM_CHAR: {
        switch(wParam) {
        case '\b':
            v.cgl.in[KEY_Text] = KEY_Backspace;
            break;
        case '\t':
            v.cgl.in[KEY_Text] = KEY_Tab;
            break;
        case '\r':
            v.cgl.in[KEY_Text] = KEY_Enter;
            break;
        default:
            if(wParam >= ' ' && wParam <= '~')
                v.cgl.in[KEY_Text] = wParam;
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
        .lpfnWndProc    = &internal_cgl_win_proc,
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
    }
    sint state = 0;
    while(state != -1) {
        v.MemTemp_ptr = 0;
        {   //  keyboard + msg
        v.cgl.in[KEY_Pressed] = 0;
        v.cgl.in[KEY_Text] = 0;

        for(uint i = KEY_MouseLeft; i <= KEY_ArrowDown; i++) {
            if(v.cgl.in[i] > 0)
                v.cgl.in[i] += v.cgl.delta_ms;
            else
                v.cgl.in[i] = 0;
        }

        MSG msg;
        while(PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        v.cgl.screen = v.screen;
        }
        {   //  mouse position
        POINT p;
        GetCursorPos(&p);
        ScreenToClient(v.window, &p);

        v2di pc_mouse = v2diVV(p, -, v.margin);
        v.cgl.mouse = v2diVN(pc_mouse, /, v.set.scale);
        v.cgl.mouse.x -= pc_mouse.x < 0;
        v.cgl.mouse.y -= pc_mouse.y < 0;


        bool show_cursor = !v.set.hide_cursor;
        if(v2diVN(pc_mouse, <, 0).all_bits)
            show_cursor = true;
        if(v2diVV(v.cgl.mouse, >=, v.screen.size).all_bits)
            show_cursor = true;

        static sint cursor = 0;
        if(cursor + 1 != show_cursor)
            cursor = ShowCursor(show_cursor);
        }
        {   //  run user code
        sint state_out = 0;
        if(v.msg_to_user) {
            state_out = cgl_main(&v.cgl, v.msg_to_user);
            v.msg_to_user = 0;
        }
        if(!state_out)
            state_out = cgl_main(&v.cgl, state);
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

        v.cgl.delta_ms = (slong)(current_time.QuadPart - past_time + dt_error) * 1000.0 / v.counter_freq;
        dt_error += current_time.QuadPart - past_time;
        dt_error -= v.cgl.delta_ms * v.counter_freq / 1000;

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
    {   //  cgl clean up
    MemFree(v.cgl.screen.buffer);

    MemFree(v.cgl.audio[0].buffer);
    MemFree(v.cgl.audio[1].buffer);
    MemFree(v.cgl.audio[2].buffer);
    }
    {   //  start clean up
    DestroyWindow(v.window);    //  can error

    UnregisterClassA("win_class", GetModuleHandleA(NULL));  //  can error
    UnregisterClassA("win_class_dbg", GetModuleHandleA(NULL));  //  can error

    MemFree(v.exe_path);

    MemFree(v.MemTemp_buffer);
    v.MemTemp_size = 0;
    }
    return 0;
}



void cgl_settings(cgl* cgl, cgl_set settings) {
    {   //  load settings
    #define DEFAULT(setting_name, default) if(!settings.setting_name) v.set.setting_name = default

    v.set = settings;
    DEFAULT(window_title, "cgl_application");
    DEFAULT(scale, 1);
    DEFAULT(fps_cap, 60);

    #undef DEFAULT
    }
    {   //  warm start
    v.frame_delta = v.counter_freq / v.set.fps_cap;


    if(v.set.resolution.all_bits) {
        MemFree(v.screen.buffer);
        v.screen.buffer = MemGet(v.set.resolution.w * v.set.resolution.h * sizeof(uint));
        if(v.screen.buffer)
            v.screen.size = v.set.resolution;
        else
            v.screen.size = v2di(0, 0);
        v.screen.real_width = v.screen.size.w;
    }
    cgl->screen = v.screen;


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



void* MemGet(ulong size) {
    return malloc(size);
}
void  MemFree(void* memory) {
    free(memory);
}
void* MemTemp(ulong size) {
    size = ((size - 1) / sizeof(ulong) + 1) * sizeof(ulong);    //  round up to ulong aligment

    if(v.MemTemp_ptr + size > v.MemTemp_size)
        return NULL;

    uchar* ret = v.MemTemp_buffer + v.MemTemp_ptr;
    v.MemTemp_ptr += size;
    return ret;
}


bool directory_create(cstr name) {
    return CreateDirectory(name, NULL) != 0;
}
bool directory_delete(cstr name) {
    return RemoveDirectory(name) != 0;
}

bool directory_open(cstr path) {
    v.find_handle = NULL;
    if(path && *path)
        return SetCurrentDirectory(path);
    else
        return SetCurrentDirectory(v.exe_path);
}
uint directory_list(uint size, cstr buffer) {
    WIN32_FIND_DATA find = {0};
    bool is_file_present;

    if(!v.find_handle) {
        v.find_handle = FindFirstFile("*", &find);
        is_file_present = v.find_handle != INVALID_HANDLE_VALUE;
    } else
        is_file_present = FindNextFileA(v.find_handle, &find);

    if(!is_file_present)
        find.cFileName[0] = '\0';

    uint i = 0;
    do {
        if(i < size)
            buffer[i] = find.cFileName[i];
    } while(find.cFileName[i++]);
    buffer[size - 1] = '\0';

    if(i > size)
        return i;
    else
        return 0;
}


bool file_create(cstr name) {
    v.open_file = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    return v.open_file != INVALID_HANDLE_VALUE;
}
bool file_delete(cstr name) {
    return DeleteFile(name) != 0;
}

bool file_open(cstr name) {
    v.open_file = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);  //  can error
    return v.open_file != INVALID_HANDLE_VALUE;
}
void file_close(void) {
    CloseHandle(v.open_file);
}
//void file_swap(void** file);

void  file_move(slong offset) {
    SetFilePointerEx(v.open_file, (LARGE_INTEGER){.QuadPart = offset}, NULL, FILE_CURRENT);   //  can error
}
void* file_read(ulong size, void* buffer) {
    DWORD bytes_read;
    ReadFile(v.open_file, buffer, size, &bytes_read, NULL);    //  can error
    return buffer;
}
bool  file_write(ulong size, void* buffer) {
    DWORD bytes_written;
    return WriteFile(v.open_file, buffer, size, &bytes_written, NULL);    //  can error
}

ulong FilePos(void) {
    LARGE_INTEGER pos = {0};
    SetFilePointerEx(v.open_file, (LARGE_INTEGER){ 0 }, &pos, FILE_CURRENT);
    return pos.QuadPart;
}
ulong FileSize(void) {
    LARGE_INTEGER size = {0};
    GetFileSizeEx(v.open_file, &size);
    return size.QuadPart;
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
    ret /= (double)v.counter_freq * 1000.0;

    if(set_zero)
        QueryPerformanceCounter(&start_tick);

    return ret;
}



#undef v
