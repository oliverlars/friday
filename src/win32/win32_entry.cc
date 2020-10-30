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
    result.y = (f32)(mouse.y);
    return result;
}

internal LRESULT
win32_window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam){
    
    LRESULT result = 0;
    
    local_persist b32 mouse_hover_active = 0;
    
    Key_Modifiers modifiers;
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
    else if(message = WM_LBUTTONDOWN){
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
    else if(message == WM_MOUSEMOVE){
        s16 xpos = LOWORD(lparam);
        s16 ypos = HIWORD(lparam);
        v2f last_mouse = global_platform.mouse_position;
        global_platform.mouse_position = win32_get_mouse_position(hwnd);
        platform_push_event(platform_mouse_move(global_platform.mouse_position,
                                                v2f(global_platform.mouse_position.x - last_mouse.x,
                                                    global_platform.mouse_position.y - last_mouse.y)));
        
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
    else if(message = WM_SETCURSOR){
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
    else if(message = WM_CHAR){
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

int
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR lp_cmd_line, int n_show_cmd){
    
}