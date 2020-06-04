#include "../ext/sdl/SDL.h"
#include "../ext/sdl/SDL_opengl.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <new>
#include "friday.h"
#include "opengl.h"
#include "render.cc"

global SDL_Window* global_window;


int 
main(int argc, char** args){
#define TITLE "Friday"
    
    init_opengl();
    
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_EnableScreenSaver();
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    
    int width = 1280;
    int height = 720;
    
    b32 running = true;
    
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    
    int context_flag = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
#if DEBUG
    context_flag |= SDL_GL_CONTEXT_DEBUG_FLAG;
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, context_flag);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    
    global_window = 
        SDL_CreateWindow(TITLE,
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         width, height,
                         SDL_WINDOW_RESIZABLE | 
                         SDL_WINDOW_HIDDEN |
                         SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
    
    SDL_GL_CreateContext(global_window);
    SDL_ShowWindow(global_window);
    
    while(running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                running = false;
            }
            
        }
        
        glViewport(0, 0, 1280, 720);
        glClearColor(1,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
        
        SDL_GL_SwapWindow(global_window);
    }
    
    return 0;
}