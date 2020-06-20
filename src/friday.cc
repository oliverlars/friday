#include "../ext/sdl/SDL.h"
#include "../ext/sdl/SDL_opengl.h"
#include "../ext/optick/optick.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "friday.h"
#include "maths.cc"
#include "opengl.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../ext/stb_truetype.h"
#include <string.h>
#include "graph.cc"
#include "ui.cc"
#include "render.cc"

global SDL_Window* global_window;

#define Kilobytes(x) (1024*x)
#define Megabytes(x) (Kilobytes(x)*1024)
#define Gigabytes(x) (Megabytes(x)*1024)

int 
main(int argc, char** args){
#define TITLE "Friday"
    
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    
    platform.width = 1280;
    platform.height = 720;
    
    void* permanent_memory = VirtualAlloc(0, Gigabytes(1), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    platform.permanent_arena = make_arena(Gigabytes(1), permanent_memory);
    
    void* temporary_memory = VirtualAlloc(0, Megabytes(64), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    platform.temporary_arena = make_arena(Megabytes(64), temporary_memory);
    
    
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
    renderer.fonts.insert(init_font("../fonts/JetBrainsMono-Regular.ttf", 30));
    
    //glEnable(GL_FRAMEBUFFER_SRGB);
    init_opengl_renderer();
    init_shaders();
    u64 tick = 0;
    int start = 0;
    int end = 0;
    SDL_Event event;
    
    friday.node_pool = make_pool(sizeof(Node));
    Pool* pool = &friday.node_pool;
    
    Node* decl = make_node(pool, NODE_DECLARATION);
    
    Node* binary = make_node(pool, NODE_BINARY);
    binary->binary.op_type = OP_PLUS;
    binary->binary.left = make_node(pool, NODE_LITERAL);
    binary->binary.right = make_node(pool, NODE_LITERAL);
    binary->binary.left->literal._int = 10;
    binary->binary.right->literal._int = 10;
    auto left = &binary->binary.left->literal;
    auto right = &binary->binary.right->literal;
    decl->declaration.declaration = binary;
    decl->name = make_string(&platform.permanent_arena, "potato");
    decl->declaration.is_initialised = true;
    
    Node* _struct = make_node(pool, NODE_STRUCT);
    _struct->name = make_string(&platform.permanent_arena, "mat4x4");
    _struct->_struct.members = nullptr;
    
    Node* _struct2 = make_node(pool, NODE_STRUCT);
    _struct2->name = make_string(&platform.permanent_arena, "vec3");
    _struct2->_struct.members = nullptr;
    
    if(_struct->name.text - decl->name.text == 256){
        OutputDebugStringA("test");
    }
    
    Node* scope = make_node(pool, NODE_SCOPE);
    scope->scope.statements = _struct;
    scope->scope.statements->next = _struct2;
    scope->scope.statements->next->next = decl;
    
    friday.program_root = scope;
    
    friday.x = 640;
    friday.y = 100;
    
    Arena test;
    f32* example = (f32*)arena_allocate(&test, 50);
    *example++ = 5;
    
    load_theme_ayu();
    assert(theme.base.r == 0x0F);
    assert(theme.base.g == 0x14);
    assert(theme.base.b == 0x19);
    assert(theme.base.a == 0xFF);
    SDL_StartTextInput();
    platform.text_input = (char*)arena_allocate(&platform.temporary_arena,
                                                256);
    
    ui_state.frame_arena = subdivide_arena(&platform.temporary_arena, 8192);
    
    bool previous_mouse_left_clicked = 0;
    while(running){
        OPTICK_FRAME("MainThread");
        
        opengl_start_frame();
        
        SDL_GetWindowSize(global_window, (int*)&platform.width, (int*)&platform.height);
        
        int x, y;
        u32 mouse_state = SDL_GetMouseState(&x, &y);
        platform.mouse_x = x;
        platform.mouse_y = platform.height - y;
        
        f32 offset = 5;
        left->_int = tick;
        right->_int = tick;
        
        //background
        push_rectangle(offset, offset, 
                       platform.width-offset*2, platform.height-offset*2, 0.1,
                       theme.base.packed);
        
        global u32 box1 = 0xFF0000FF;
        global u32 box2 = 0x00FF00FF;
        
        button(350, 350, 100, 100,box1, []{
                   box1 = 0xFF0000FF;
                   box2 = 0x00FF00FF;
                   OutputDebugStringA("BOX1\n");
               });
        
        
        button(350, 350, 50, 50, box2, []{
                   box1 = 0x00FF00FF;
                   box2 = 0xFF0000FF;
                   OutputDebugStringA("BOX2\n");
               });
        process_widgets_and_handle_events();
        render_graph(scope);
        opengl_end_frame();
        
        // NOTE(Oliver): supposedley this goes here
        // to avoid weird frame spikes
        // it helps a little but ultimately
        // disabling threaded optimisation
        // in the nvidia control panel is what
        // fixed it
        // fuck you opengl
        platform.mouse_move = 0;
        platform.has_text_input = 0;
        
        
        char* text_input = platform.text_input;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                running = false;
            }
            if(event.type == SDL_MOUSEBUTTONDOWN){
                if(event.button.button == SDL_BUTTON_LEFT){
                    if(event.button.clicks == 1){
                        if(1 || !previous_mouse_left_clicked){
                            platform.mouse_left_clicked = 1;
                            OutputDebugStringA("Clicked\n");
                        }
                    }else if(event.button.clicks == 2){
                        platform.mouse_left_double_clicked = 1;
                    }
                    
                }
                if(event.button.button == SDL_BUTTON_RIGHT){
                    if(event.button.clicks == 1){
                        platform.mouse_right_clicked = 1;
                    }
                }
            }
            if(event.type == SDL_MOUSEBUTTONUP){
                if(event.button.button == SDL_BUTTON_LEFT){
                    platform.mouse_left_clicked = 0;
                    platform.mouse_left_down = 0;
                    platform.mouse_left_double_clicked = 0;
                }
                if(event.button.button == SDL_BUTTON_RIGHT){
                }
            }
            if(event.type == SDL_KEYDOWN){
                SDL_Keycode key = event.key.keysym.sym;
                platform.keys_pressed[SDL_GetScancodeFromKey(key)] = 1;
            }
            if(event.type == SDL_TEXTINPUT){
                platform.has_text_input = 1;
                *text_input++ = *event.text.text;
            }
            if(event.type == SDL_MOUSEMOTION){
                platform.mouse_move = 1;
                
            }
        }
        previous_mouse_left_clicked = platform.mouse_left_clicked;
        
        SDL_GL_SwapWindow(global_window);
        
        tick++;
    }
    
    return 0;
}