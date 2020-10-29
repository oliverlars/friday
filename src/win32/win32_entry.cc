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

int
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR lp_cmd_line, int n_show_cmd){
    
}