#include "../ext/sdl/SDL.h"
#include "../ext/sdl/SDL_opengl.h"
#include "../ext/optick/optick.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "friday.h"
#include "maths.cc"
#include "opengl.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../ext/stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../ext/stb_image.h"


#include <string.h>

#include "graph.cc"

#include "ui.cc"
#include "render.cc"
#include "present.cc"
#include "navigation.cc"

global SDL_Window* global_window;

int
main(int argc, char** args){
#define TITLE "Friday"
    
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    
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
    
    renderer.font = load_sdf_font("../fonts/friday_default.fnt");
    
    init_opengl_renderer();
    init_shaders();
    
    int start = 0;
    
    int end = 0;
    SDL_Event event;
    
    friday.node_pool = make_pool(sizeof(Node) + 256);
    Pool* pool = &friday.node_pool;
    
    
    _u8 = make_node(&friday.node_pool, NODE_DECLARATION, "u8");
    _u16 = make_node(&friday.node_pool, NODE_DECLARATION, "u16");
    _u32 = make_node(&friday.node_pool, NODE_DECLARATION, "u32");
    _u64 = make_node(&friday.node_pool, NODE_DECLARATION, "u64");
    
    Node* global_scope = make_node(pool, NODE_SCOPE, "global");
    global_scope->scope.statements = make_node(pool, NODE_DUMMY);
    global_scope->scope.statements->next = make_struct_node(pool, "mat4x4");
    global_scope->scope.statements->next->next = make_function_node(pool, "entry");
    global_scope->scope.statements->next->next->function.parameters = make_node(pool, NODE_DUMMY, "parameters");
    
    global_scope->scope.statements->next->next->function.scope = make_node(pool, NODE_SCOPE);
    auto function_scope = global_scope->scope.statements->next->next->function.scope;
    
    function_scope->scope.statements = make_node(pool, NODE_DUMMY);
    function_scope->scope.statements->prev = nullptr;
    function_scope->scope.statements->next = make_declaration_node(pool, "test");
    function_scope->scope.statements->next->prev = function_scope->scope.statements->next;
    function_scope->scope.statements->next->next = nullptr;
    
    
    friday.program_root = global_scope;
    friday.selected_node = global_scope->scope.statements;
    
    friday.x = 640;
    friday.y = 100;
    
    Arena test;
    f32* example = (f32*)arena_allocate(&test, 50);
    *example++ = 5;
    
    bitmap = make_bitmap("logo5.png");
    move_icon = make_bitmap("move_icon.png");
    add_icon = make_bitmap("add_icon.png");
    options_icon = make_bitmap("options_icon.png");
    bin_icon = make_bitmap("bin_icon.png");
    
    search_icon = make_bitmap("search_icon.png");
    run_icon = make_bitmap("running_icon.png");
    layers_icon = make_bitmap("layers_icon.png");
    document_icon = make_bitmap("document_icon.png");
    
    cursor_bitmap = make_bitmap("arrow.png");
    
    load_theme_ayu();
    SDL_StartTextInput();
    
    ui_state.frame_arena = subdivide_arena(&platform.temporary_arena, 8192*4);
    
    Presenter presenter = {};
    presenter.root = global_scope;
    presenter.active_node = global_scope->scope.statements->next->next;
    presenter.node_pool = make_pool(sizeof(Present_Node));
    
    
    Present_Node start_node = {};
    start_node.type = PRESENT_NODE;
    start_node.text = make_string(&platform.permanent_arena, " ");
    
    presenter.node_list = &start_node;
    presenter.node_list_tail = &presenter.node_list;
    presenter.active_string = &start_node.text;
    presenter.active_present_node = &start_node;
    
    Panel* root = (Panel*)arena_allocate(&platform.permanent_arena, sizeof(Panel));
    root->presenter = &presenter;
    root->split_ratio = 1.0f;
    root->type = PANEL_EDITOR;
    
    split_panel(root, 0.75, PANEL_SPLIT_VERTICAL, PANEL_PROPERTIES);
    
    bool previous_mouse_left_clicked = 0;
    
    u32 start_time = 0;
    u32 end_time = 0;
    while(running){
        start_time = SDL_GetTicks();
        
        OPTICK_FRAME("MainThread");
        
        opengl_start_frame();
        
        SDL_GetWindowSize(global_window, (int*)&platform.width, (int*)&platform.height);
        
        {
            SDL_Cursor* cursor;
            cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
            SDL_SetCursor(cursor);
        }
        f32 offset = 5;
        //code panel
        
        draw_panels(root, 0, 40, platform.width, platform.height-80);
        friday.cursor_x += lerp(friday.cursor_x, friday.cursor_target_x, 0.1f);
        friday.cursor_y += lerp(friday.cursor_y, friday.cursor_target_y, 0.1f);
        
        //push_rectangle_textured(friday.cursor_x, friday.cursor_y-30/2, 30,30,0, cursor_bitmap);
        
        
        //present_graph(global_scope->scope.statements->next);
        
        draw_status_bar(&presenter);
        
        display_modes();
        navigate_graph(&presenter);
        
        
        if(ui_state.menu_open){
            right_click_menu(ui_state.active_panel, "rcm");
        }
        draw_menu_bar();
        
        process_widgets_and_handle_events();
        opengl_end_frame();
        
        platform.mouse_drag = 0;
        platform.has_text_input = 0;
        platform.mouse_left_clicked = 0;
        
        free(platform.text_input);
        
        platform.text_input = (char*)calloc(1, 256);
        char* text_input = platform.text_input;
        for(int i = 0; i < INPUT_COUNT; i++){
            input.actions[i].half_transition_count = 0;
        }
        platform.mouse_scroll_delta = 0;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                running = false;
            }
            if(event.type == SDL_MOUSEBUTTONDOWN){
                if(event.button.button == SDL_BUTTON_LEFT){
                    if(!platform.mouse_drag){
                        platform.mouse_drag_x = platform.mouse_x;
                        platform.mouse_drag_y = platform.mouse_y;
                    }
                    
                    platform.mouse_left_down = 1;
                }
                
                if(event.button.button == SDL_BUTTON_RIGHT){
                    platform.mouse_right_down = 1;
                }
                
                if(event.button.button == SDL_BUTTON_MIDDLE){
                    if(!platform.mouse_drag){
                        
                        platform.mouse_drag_x = platform.mouse_x;
                        platform.mouse_drag_y = platform.mouse_y;
                    }
                    
                    platform.mouse_middle_down = 1;
                    
                }
            }
            
            if(event.type == SDL_MOUSEBUTTONUP){
                if(event.button.button == SDL_BUTTON_LEFT){
                    if(platform.mouse_left_down){
                        platform.mouse_left_clicked = 1;
                    }
                    platform.mouse_left_up = 1;
                    platform.mouse_left_down = 0;
                    platform.mouse_left_double_clicked = 0;
                }
                if(event.button.button == SDL_BUTTON_RIGHT){
                    if(platform.mouse_right_down){
                        platform.mouse_right_clicked = 1;
                    }
                    platform.mouse_right_up = 1;
                    platform.mouse_right_down = 0;
                }
                if(event.button.button == SDL_BUTTON_MIDDLE){
                    if(platform.mouse_middle_down){
                        platform.mouse_middle_clicked = 1;
                    }
                    platform.mouse_middle_up = 1;
                    platform.mouse_middle_down = 0;
                    if(platform.mouse_drag){
                        
                        friday.pan_offset_x += friday.delta_x;
                        friday.pan_offset_y += friday.delta_y;
                        friday.delta_x = 0;
                        friday.delta_y = 0;
                        
                    }
                    
                }
                platform.mouse_drag = 0;
            }
            
            if(event.type == SDL_TEXTINPUT){
                platform.has_text_input = 1;
                *text_input++ = *event.text.text;
            }
            
            if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP){
                SDL_Keycode sdl_key = event.key.keysym.sym;
                u16 mod = event.key.keysym.mod;
                u64 key = 0;
                bool is_down = (event.key.state == SDL_PRESSED);
                
                if(event.key.repeat == 0){
                    if((sdl_key >= 'a' && sdl_key <= 'z') || (sdl_key >= '0' && sdl_key <= '9')){
                        key = (sdl_key >= 'a' && sdl_key <= 'z') ? KEY_A + (sdl_key-'a') : KEY_0 + (sdl_key-'0');
                    } else {
                        if(sdl_key == SDLK_ESCAPE){
                            key = KEY_ESC;
                        }
                        else if(sdl_key >= VK_F1 && sdl_key <= VK_F12) {
                            key = KEY_F1 + sdl_key - VK_F1;
                        }
                        else if(sdl_key == SDLK_BACKQUOTE) {
                            key = KEY_GRAVE_ACCENT;
                        }
                        else if(sdl_key == SDLK_MINUS) {
                            key = KEY_MINUS;
                        }
                        else if(sdl_key == SDLK_PLUS) {
                            key = KEY_EQUAL;
                        }
                        else if(sdl_key == SDLK_BACKSPACE) {
                            key = KEY_BACKSPACE;
                        }
                        else if(sdl_key == SDLK_TAB) {
                            key = KEY_TAB;
                        }
                        else if(sdl_key == SDLK_SPACE) {
                            key = KEY_SPACE;
                        }
                        else if(sdl_key == SDLK_RETURN) {
                            key = KEY_ENTER;
                        }
                        else if(sdl_key == SDLK_LCTRL) {
                            key = KEY_CTRL;
                        }
                        else if(sdl_key == SDLK_LSHIFT) {
                            key = KEY_SHIFT;
                        }
                        else if(sdl_key == SDLK_MENU) {
                            key = KEY_ALT;
                        }
                        else if(sdl_key == SDLK_UP) {
                            key = KEY_UP;
                        }
                        else if(sdl_key == SDLK_LEFT) {
                            key = KEY_LEFT;
                        }
                        else if(sdl_key == SDLK_DOWN) {
                            key = KEY_DOWN;
                        }
                        else if(sdl_key == SDLK_RIGHT) {
                            key = KEY_RIGHT;
                        }
                        else if(sdl_key == SDLK_DELETE) {
                            key = KEY_DELETE;
                        }
                        else if(sdl_key == SDLK_PRIOR) {
                            key = KEY_PAGE_UP;
                        }
                        else if(sdl_key == SDLK_PAGEDOWN) {
                            key = KEY_PAGE_DOWN;
                        }
                        else if(sdl_key = SDLK_LEFTBRACKET){
                            key = KEY_LBRACKET;
                        }
                        else if(sdl_key == SDLK_SEMICOLON){
                            key = KEY_SEMICOLON;
                        }
                    }
                    if(key == KEY_ENTER){
                        process_keyboard_event(&input.enter_make_mode, is_down);
                    }
                    if(key == KEY_J){
                        process_keyboard_event(&input.navigate_down, is_down);
                    }
                    if(key == KEY_K){
                        process_keyboard_event(&input.navigate_up, is_down);
                    }
                    if(key == KEY_I){
                        process_keyboard_event(&input.enter_text_edit_mode, is_down);
                    }
                    if(key == KEY_ENTER){
                        process_keyboard_event(&input.enter_command_mode, is_down);
                    }
                    if(key == KEY_BACKSPACE){
                        process_keyboard_event(&input.backspace, is_down);
                    }
                    if(key == KEY_H){
                        process_keyboard_event(&input.navigate_left, is_down);
                    }
                    if(key == KEY_L){
                        process_keyboard_event(&input.navigate_right, is_down);
                        
                    }
                    if(key == KEY_A){
                        process_keyboard_event(&input.make_arg, is_down);
                    }
                    if(key == KEY_D){
                        process_keyboard_event(&input.make_decl, is_down);
                    }
                    if(key == KEY_F){
                        process_keyboard_event(&input.make_func, is_down);
                    }
                    if(key == KEY_L){
                        process_keyboard_event(&input.make_loop, is_down);
                    }
                    if(key == KEY_C){
                        process_keyboard_event(&input.make_cond, is_down);
                    }
                    if(key == KEY_CTRL){
                        process_keyboard_event(&input.editor_zoom, is_down);
                    }
                    if(key == KEY_ENTER){
                        process_keyboard_event(&input.enter_colon, is_down);
                    }
                    if(key == KEY_S){
                        process_keyboard_event(&input.enter_struct, is_down);
                    }
                }
                
            }
            if(event.type == SDL_MOUSEMOTION){
                
                if(platform.mouse_left_down){
                    platform.mouse_drag = true;
                }
                if(platform.mouse_middle_down){
                    platform.mouse_drag = true;
                }
                
            }
            if(event.type == SDL_MOUSEWHEEL){
                f32 temp_scroll = platform.mouse_scroll_target;
                platform.mouse_scroll_target += event.wheel.y*50;
                platform.mouse_scroll_delta = event.wheel.y*50;
            }
        }
        platform.mouse_scroll_source += lerp(platform.mouse_scroll_source, platform.mouse_scroll_target, 0.1f);
        
        int x, y;
        u32 mouse_state = SDL_GetMouseState(&x, &y);
        platform.mouse_delta_x = x - platform.mouse_x;
        platform.mouse_delta_y = (platform.height - y) - platform.mouse_y;
        platform.mouse_x = x;
        platform.mouse_y = platform.height - y;
        SDL_GL_SwapWindow(global_window);
        platform.tick++;
        end_time = SDL_GetTicks();
        platform.delta_time = ((f32)end_time - (f32)start_time)/1000.0f;
    }
    
    return 0;
}