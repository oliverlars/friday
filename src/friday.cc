#include "../ext/sdl/SDL.h"
#include "../ext/sdl/SDL_opengl.h"
#include "../ext/optick/optick.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <new>
#include "friday.h"
#include "maths.cc"
#include "opengl.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../ext/stb_truetype.h"
#include "render.cc"

global SDL_Window* global_window;


int 
main(int argc, char** args){
#define TITLE "Friday"
    
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    
    platform.width = 1280;
    platform.height = 720;
    
    b32 running = true;
    
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    
    int context_flag = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, context_flag);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    
    global_window = 
        SDL_CreateWindow(TITLE,
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         platform.width, platform.height,
                         SDL_WINDOW_RESIZABLE | 
                         SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);
    
    
    SDL_GL_CreateContext(global_window);
    
    SDL_GL_SetSwapInterval(1);
    
    load_opengl();
    
    SDL_ShowWindow(global_window);
    
    init_opengl_renderer();
    init_shaders();
    u64 tick = 0;
    int start = 0;
    int end = 0;
    FILE* file = fopen("frames.txt", "w");
    char buffer[4096];
    u64 buffer_index = 0;
    SDL_Event event;
    
    while(running){
        OPTICK_FRAME("MainThread");
        opengl_start_frame();
        
        {
            OPTICK_EVENT("Window Size");
            SDL_GetWindowSize(global_window, (int*)&platform.width, (int*)&platform.height);
        }
        
        {
            OPTICK_EVENT("Draw Scope");
            
            
            int x, y;
            SDL_GetMouseState(&x, &y);
            
            push_circle(x, platform.height - y, 200);
            
            push_circle(sinf(tick/20)*20, 360, 200);
            
        }
        opengl_end_frame();
        
        // NOTE(Oliver): supposedley this goes here
        // to avoid weird frame spikes
        // it helps a little but ultimately
        // disabling threaded optimisation
        // in the nvidia control panel is what
        // fixed it
        // fuck you opengl
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                running = false;
            }
        }
        
        {
            OPTICK_EVENT("Swap Window");
            SDL_GL_SwapWindow(global_window);
        }
        
        tick++;
    }
    
    return 0;
}