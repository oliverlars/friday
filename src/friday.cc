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
#include <string.h>
#include "graph.cc"
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
    //SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);
    
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
    renderer.fonts.insert(init_font("../fonts/Inconsolata-Regular.ttf", 30));
    
    init_opengl_renderer();
    init_shaders();
    u64 tick = 0;
    int start = 0;
    int end = 0;
    SDL_Event event;
    
    auto binary = new Node_Binary;
    binary->op_type = OP_MULTIPLY;
    binary->left = new Node_Literal;
    auto left = reinterpret_cast<Node_Literal*>(binary->left);
    left->lit_type = LIT_INTEGER;
    left->_int = 2;
    
    binary->right = new Node_Literal;
    auto right = reinterpret_cast<Node_Literal*>(binary->right);
    right->lit_type = LIT_INTEGER;
    right->_int = 20;
    
    auto _struct = new Node_Struct;
    _struct->members = binary;
    char name[32] = "speed racer";
    _struct->name = name;
    
    friday.x = 640;
    friday.y = 360;
    
    Pool pool(16);
    u8* random_mem = nullptr;
    for(int i = 0; i < 1000; i++){
        u8* memory = (u8*)pool.allocate();
        if(i == 32){
            random_mem = memory;
        }
    }
    pool.clear(random_mem);
    
    while(running){
        OPTICK_FRAME("MainThread");
        
        opengl_start_frame();
        
        SDL_GetWindowSize(global_window, (int*)&platform.width, (int*)&platform.height);
        
        int x, y;
        SDL_GetMouseState(&x, &y);
        
        
        f32 offset = 5;
        left->_int = tick;
        right->_int = tick;
        push_rectangle(offset, offset, 
                       platform.width-offset*2, platform.height-offset*2, 0.1,
                       0x00101010);
        
        render_graph(_struct);
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
        
        SDL_GL_SwapWindow(global_window);
        
        tick++;
    }
    
    return 0;
}