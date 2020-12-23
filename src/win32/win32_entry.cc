#include <windows.h>
#include <objbase.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>

#include <gl/gl.h>
#include "ext/wglext.h"
#include "ext/glext.h"


#include <stdio.h>


#include "program_options.h"
#include "extras.h"
#include "maths.h"
#include "maths.cc"
#include "memory.h"
#include "strings.h"
#include "platform.h"
#include "win32_timer.h"
#include "extras.cc"
#include "memory.cc"
#include "strings.cc"
#include "platform.cc"

global char global_executable_path[256];
global char global_executable_directory[256];
global char global_working_directory[256];
global char global_app_dll_path[256];
global char global_temp_app_dll_path[256];
global Platform global_platform;
global HDC global_device_context;
global HINSTANCE global_instance;
global Win32_Timer global_win32_timer = {};


#include "win32_misc.cc"
#include "win32_timer.cc"
#include "win32_io.cc"
#include "win32_reload.cc"
#include "win32_wasapi.cc"
#include "win32_opengl.cc"


enum Win32_CursorStyle {
    CURSOR_DEFAULT,
    CURSOR_HRESIZE,
    CURSOR_VRESIZE,
    CURSOR_IBAR,
};

global Win32_CursorStyle global_cursor_style = CURSOR_DEFAULT;

internal v2f
win32_get_mouse_position(HWND window)
{
    v2f result = {0};
    POINT mouse;
    GetCursorPos(&mouse);
    ScreenToClient(window, &mouse);
    result.x = (f32)(mouse.x);
    RECT rect;
    GetClientRect(window, &rect);
    result.y = rect.bottom - (f32)mouse.y;
    return result;
}


internal LRESULT 
win32_window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam){
    
    LRESULT result = 0;
    
    local_persist b32 mouse_hover_active = 0;
    local_persist b32 mmd = 0;
    
    Key_Modifiers modifiers = {};
    if(GetKeyState(VK_CONTROL) & 0x8000){
        modifiers |= KEY_MOD_CTRL;
    }
    if(GetKeyState(VK_SHIFT) & 0x8000){
        modifiers |= KEY_MOD_SHIFT;
    }
    if(GetKeyState(VK_MENU) & 0x8000){
        modifiers |= KEY_MOD_ALT;
    }
    
    
    if(message == WM_CLOSE || message == WM_DESTROY || message == WM_QUIT){
        global_platform.quit = 1;
        result = 0;
    }
    else if(message == WM_LBUTTONDOWN){
        platform_push_event(platform_mouse_press(MOUSE_BUTTON_LEFT, global_platform.mouse_position));
    }
    else if(message == WM_LBUTTONUP){
        platform_push_event(platform_mouse_release(MOUSE_BUTTON_LEFT, global_platform.mouse_position));
    }
    else if(message == WM_RBUTTONDOWN){
        platform_push_event(platform_mouse_press(MOUSE_BUTTON_RIGHT, global_platform.mouse_position));
    }
    else if(message == WM_RBUTTONUP){
        platform_push_event(platform_mouse_release(MOUSE_BUTTON_RIGHT, global_platform.mouse_position));
    }
    else if(message == WM_MBUTTONDOWN){
        platform_push_event(platform_mouse_press(MOUSE_BUTTON_MIDDLE, global_platform.mouse_position));
        mmd = 1;
    }else if(message == WM_MBUTTONUP){
        platform_push_event(platform_mouse_release(MOUSE_BUTTON_MIDDLE, global_platform.mouse_position));
        mmd = 0;
    }
    else if(message == WM_MOUSEMOVE){
        s16 xpos = LOWORD(lparam);
        s16 ypos = HIWORD(lparam);
        v2f last_mouse = global_platform.mouse_position;
        global_platform.mouse_position = win32_get_mouse_position(hwnd);
        log("move");
        if(1 && mmd){
            
            platform_push_event(platform_mouse_drag(global_platform.mouse_position,
                                                    v2f(global_platform.mouse_position.x - last_mouse.x,
                                                        global_platform.mouse_position.y - last_mouse.y)));
            log("%d", mmd);
            
        }else {
            platform_push_event(platform_mouse_move(global_platform.mouse_position,
                                                    v2f(global_platform.mouse_position.x - last_mouse.x,
                                                        global_platform.mouse_position.y - last_mouse.y)));
        }
        if(!mouse_hover_active){
            mouse_hover_active = 1;
            TRACKMOUSEEVENT track_mouse_event = {};
            {
                track_mouse_event.cbSize = sizeof(track_mouse_event);
                track_mouse_event.dwFlags = TME_LEAVE;
                track_mouse_event.hwndTrack = hwnd;
                track_mouse_event.dwHoverTime = HOVER_DEFAULT;
            }
            TrackMouseEvent(&track_mouse_event);
        }
        
    }
    else if(message == WM_MOUSELEAVE){
        
    }
    else if(message == WM_MOUSEWHEEL){
        s16 wheel_delta = HIWORD(wparam);
        platform_push_event(platform_mouse_scroll(v2f(0, (f32)wheel_delta), modifiers));
    }
    else if(message == WM_MOUSEHWHEEL){
        s16 wheel_delta = HIWORD(wparam);
        platform_push_event(platform_mouse_scroll(v2f((f32)wheel_delta, 0), modifiers));
    }
    else if(message == WM_SETCURSOR){
        if(is_in_rect(global_platform.mouse_position,
                      v4f(1, 1, 
                          global_platform.window_size.x,
                          global_platform.window_size.y))){
            switch(global_cursor_style){
                case CURSOR_HRESIZE:{
                    SetCursor(LoadCursorA(0, IDC_SIZEWE));
                }break;
                
                case CURSOR_VRESIZE:{
                    SetCursor(LoadCursorA(0, IDC_SIZENS));
                }break;
                
                case CURSOR_IBAR:{
                    SetCursor(LoadCursorA(0, IDC_ARROW));
                }break;
                default: break;
            }
        }else {
            result = DefWindowProc(hwnd, message, wparam, lparam);
        }
        
    }else if(message ==  WM_SYSKEYDOWN || message == WM_SYSKEYUP ||
             message == WM_KEYDOWN || message == WM_KEYUP){
        u64 vkey_code = wparam;
        s8 was_down = !!(lparam & (1 << 30));
        s8 is_down = !(lparam & (1 << 31));
        
        u64 key_input = 0;
        
        if((vkey_code >= 'A' && vkey_code <= 'Z') ||
           (vkey_code >= '0' && vkey_code <= '9')){
            
            key_input = (vkey_code >= 'A' && vkey_code <= 'Z') ? KEY_A + 
                (vkey_code - 'A') : KEY_0 + (vkey_code - '0');
        }else {
            if(vkey_code == VK_ESCAPE){
                key_input = KEY_ESC;
            }
            else if(vkey_code >= VK_F1 && vkey_code <= VK_F12){
                key_input = KEY_F1 + vkey_code - VK_F1;
            }
            else if(vkey_code == VK_OEM_3){
                key_input = KEY_GRAVE_ACCENT;
            }
            else if(vkey_code == VK_OEM_MINUS){
                key_input = KEY_MINUS;
            }
            else if(vkey_code == VK_OEM_PLUS){
                key_input = KEY_EQUAL;
            }
            else if(vkey_code == VK_BACK){
                key_input = KEY_BACKSPACE;
            }
            else if(vkey_code == VK_TAB){
                key_input = KEY_TAB;
            }
            else if(vkey_code == VK_SPACE){
                key_input = KEY_SPACE;
            }
            else if(vkey_code == VK_RETURN){
                key_input = KEY_ENTER;
            }
            else if(vkey_code == VK_CONTROL){
                key_input = KEY_CTRL;
                modifiers &= ~KEY_MOD_CTRL;
            }
            else if(vkey_code == VK_SHIFT){
                key_input = KEY_SHIFT;
                modifiers &= ~KEY_MOD_SHIFT;
            }
            else if(vkey_code == VK_MENU){
                key_input = KEY_ALT;
                modifiers &= ~KEY_MOD_ALT;
            }
            else if(vkey_code == VK_UP){
                key_input = KEY_UP;
            }
            else if(vkey_code == VK_LEFT){
                key_input = KEY_LEFT;
            }
            else if(vkey_code == VK_DOWN){
                key_input = KEY_DOWN;
            }
            else if(vkey_code == VK_RIGHT){
                key_input = KEY_RIGHT;
            }
            else if(vkey_code == VK_DELETE){
                key_input = KEY_DELETE;
            }
            else if(vkey_code == VK_PRIOR){
                key_input = KEY_PAGE_UP;
            }
            else if(vkey_code == VK_NEXT){
                key_input = KEY_PAGE_DOWN;
            }
            else if(vkey_code == VK_HOME){
                key_input = KEY_HOME;
            }
            else if(vkey_code == VK_END){
                key_input = KEY_END;
            }
            else if(vkey_code == VK_OEM_2){
                key_input - KEY_FORWARD_SLASH;
            }
            else if(vkey_code == VK_OEM_PERIOD){
                key_input = KEY_FULLSTOP;
            }
            else if(vkey_code == VK_OEM_COMMA){
                key_input = KEY_COMMA;
            }
            else if(vkey_code == VK_OEM_7){
                key_input = KEY_QUOTE;
            }
            else if(vkey_code == VK_OEM_4){
                key_input = KEY_LBRACKET;
            }
            else if(vkey_code == VK_OEM_6){
                key_input = KEY_RBRACKET;
            }
        }
        
        if(is_down){
            platform_push_event(platform_key_press(key_input, modifiers));
        }else {
            platform_push_event(platform_key_release(key_input, modifiers));
        }
        
        result = DefWindowProc(hwnd, message, wparam, lparam);
    }
    else if(message == WM_CHAR){
        u64 char_input = wparam;
        if(char_input >= 32 && char_input != VK_RETURN && char_input != VK_ESCAPE &&
           char_input != 127){
            platform_push_event(platform_character_input(char_input));
        }
    }else {
        result = DefWindowProc(hwnd, message, wparam, lparam);
    }
    
    return result;
}

internal f32
win32_get_time(void) {
    Win32_Timer *timer = &global_win32_timer;
    LARGE_INTEGER current_time;
    QueryPerformanceCounter(&current_time);
    return global_platform.current_time + 
        (f32)(current_time.QuadPart - timer->begin_frame.QuadPart) / (f32)timer->counts_per_second.QuadPart;
}

internal u64
win32_get_cycles(void) {
    u64 result = __rdtsc();
    return result;
}

internal void
win32_reset_cursor(void) {
    global_cursor_style = CURSOR_DEFAULT;
}

internal void
win32_set_cursor_to_horizontal_reisze(void) {
    global_cursor_style = CURSOR_HRESIZE;
}

internal void
win32_set_cursor_to_vertical_resize(void) {
    global_cursor_style = CURSOR_VRESIZE;
}

int
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR lp_cmd_line, int n_show_cmd){
    global_instance = instance;
    
    win32_timer_init(&global_win32_timer);
    
    
    
    {
        DWORD size_of_executable_path =
            GetModuleFileNameA(0, global_executable_path, sizeof(global_executable_path));
        
        {
            memcpy(global_executable_directory, global_executable_path, size_of_executable_path);
            
            char* one_past_last_slash = global_executable_directory;
            for(int i = 0; global_executable_directory[i]; i++){
                if(global_executable_directory[i] == '\\'){
                    one_past_last_slash = global_executable_directory + i + 1;
                }
            }
            *one_past_last_slash = 0;
        }
        
        {
            wsprintf(global_app_dll_path, "%s%s.dll", global_executable_directory, PROGRAM_FILENAME);
            wsprintf(global_temp_app_dll_path, "%stemp_%s.dll", global_executable_directory, PROGRAM_FILENAME);
        }
        
        GetCurrentDirectory(sizeof(global_working_directory), global_working_directory);
    }
    
    WNDCLASS window_class = {};
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = win32_window_proc;
    window_class.hInstance = instance;
    window_class.lpszClassName = "WindowClass";
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    
    
    if(!RegisterClass(&window_class)){
        win32_output_error("Fatal Error", "Window class registration failure");
        goto quit;
    }
    
    HWND hwnd = CreateWindow("WindowClass", WINDOW_TITLE,
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                             DEFAULT_WINDOW_WIDTH,
                             DEFAULT_WINDOW_HEIGHT,
                             0, 0, instance, 0);
    
    
    if(!hwnd)
    {
        // NOTE(rjf): ERROR: Window creation failure
        win32_output_error("Fatal Error", "Window creation failure.");
        goto quit;
    }
    
    Win32_Reload win32_app_code = {0};
    {
        if(!win32_code_load(&win32_app_code))
        {
            // NOTE(rjf): ERROR: Application code load failure
            win32_output_error("Fatal Error", "Application code load failure.");
            goto quit;
        }
    }
    
    f32 refresh_rate = 120.f;
    {
        DEVMODEA device_mode = {0};
        if(EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &device_mode))
        {
            refresh_rate = (float)device_mode.dmDisplayFrequency;
        }
    }
    {
        platform = &global_platform;
        
        global_platform.executable_folder_absolute_path = string_from_cstr(global_executable_directory);
        global_platform.executable_absolute_path = string_from_cstr(global_executable_path);
        global_platform.working_directory_path = string_from_cstr(global_working_directory);
        
        global_platform.quit                      = 0;
        global_platform.vsync                     = 1;
        global_platform.fullscreen                = 0;
        global_platform.window_size.x             = DEFAULT_WINDOW_WIDTH;
        global_platform.window_size.y             = DEFAULT_WINDOW_HEIGHT;
        global_platform.current_time              = 0.f;
        global_platform.target_fps  = refresh_rate;
        
        global_platform.heap_alloc                      = win32_heap_alloc;
        global_platform.reserve                         = win32_reserve;
        global_platform.release                         = win32_release;
        global_platform.commit                          = win32_commit;
        global_platform.uncommit                        = win32_uncommit;
        global_platform.output_error                    = win32_output_error;
        global_platform.save_to_file                    = win32_save_to_file;
        global_platform.append_to_file                  = win32_append_to_file;
        global_platform.load_file                       = win32_load_entire_file;
        global_platform.load_file_and_null_terminate    = win32_load_file_and_null_terminate;
        global_platform.delete_file                     = win32_delete_file;
        global_platform.make_directory                  = win32_make_directory;
        global_platform.does_file_exist                 = win32_does_file_exist;
        global_platform.copy_file                       = win32_copy_file;
        global_platform.get_time                        = win32_get_time;
        global_platform.get_cycles                      = win32_get_cycles;
        global_platform.reset_cursor                    = win32_reset_cursor;
        global_platform.set_cursor_to_horizontal_resize = win32_set_cursor_to_horizontal_reisze;
        global_platform.set_cursor_to_vertical_resize   = win32_set_cursor_to_vertical_resize;
        global_platform.load_opengl_procedure           = win32_load_opengl_proc;
        global_platform.refresh_screen                  = win32_opengl_refresh_screen;
        
        global_platform.permanent_arena = make_arena();
    }
    
    Arena frame_arenas[] = { make_arena(), make_arena()};
    
    global_platform.frame = 0; //NOTE(Oliver): for double buffering the frame arena
    global_platform.frame_arena = frame_arenas[global_platform.frame];
    
    {
        global_device_context = GetDC(hwnd);
        if(!win32_init_opengl(&global_device_context, instance)){
            win32_output_error("Fatal Error", "OpenGL init failed");
            goto quit;
        }
    }
    
    win32_app_code.permanent_load(&global_platform);
    win32_app_code.hot_load(&global_platform);
    
    ShowWindow(hwnd, n_show_cmd);
    UpdateWindow(hwnd);
    
    while(!global_platform.quit){
        global_platform.frame_arena = frame_arenas[global_platform.frame];
        arena_clear(&frame_arenas[global_platform.frame]);
        
        win32_timer_begin_frame(&global_win32_timer);
        
        {
            MSG message;
            
            if(global_platform.wait_for_events_to_update && !global_platform.pump_events){
                WaitMessage();
            }
            
            while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }
        
        {
            RECT client_rect;
            GetClientRect(hwnd, &client_rect);
            global_platform.window_size.x = client_rect.right - client_rect.left;
            global_platform.window_size.y = client_rect.bottom - client_rect.top;
        }
        
        
        platform_begin_frame();
        {
            POINT mouse;
            GetCursorPos(&mouse);
            ScreenToClient(hwnd, &mouse);
            global_platform.pump_events = 0;
        }
        
        
        {
            b32 last_fullscreen = global_platform.fullscreen;
            
            win32_app_code.update();
            
            // NOTE(rjf): Update fullscreen if necessary
            if(last_fullscreen != global_platform.fullscreen)
            {
                win32_toggle_fullscreen(hwnd);
            }
            
        }
        log(" ");
        platform_end_frame();
        
        global_platform.frame = !global_platform.frame;
        
        win32_code_update(&win32_app_code);
        
        win32_timer_end_frame(&global_win32_timer, 1000.0 * (1.0 / (f64)global_platform.target_fps));
        
        
    }
    
    ShowWindow(hwnd, SW_HIDE);
    
    win32_code_unload(&win32_app_code);
    win32_cleanup_opengl(&global_device_context);
    
    quit:;
    
    return 0;
}