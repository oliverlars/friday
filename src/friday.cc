#include "extras.h"
#include "maths.h"
#include "memory.h"
#include "strings.h"
#include "platform.h"
#include "opengl.h"
#include "render.h"
#include "ui.h"
#include "widgets.h"
#include "graph.h"
#include "present.h"
#include "friday.h"

#include "extras.cc"
#include "maths.cc"
#include "memory.cc"
#include "strings.cc"
#include "platform.cc"


#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

#include "render.cc"
#include "ui.cc"
#include "widgets.cc"
#include "present.cc"
#include "graph.cc"

global Friday_Globals* globals = 0;

internal void
initialise_globals(){
    globals = (Friday_Globals*)platform->globals;
    renderer = globals->renderer;
    ui = globals->ui;
    editor = globals->editor;
    presenter = globals->presenter;
}

internal void
start_frame(){
    ui->root = nullptr;
    ui->layout_stack = nullptr;
    
    ui->last_widget_table = ui->widget_table;
    ui->widget_table = (Widget**)push_size_zero(&platform->frame_arena, MAX_TABLE_WIDGETS*sizeof(Widget*));
    
    // TODO(Oliver): tidy this up
    presenter->last_table = presenter->table;
    presenter->table = (Present_Node**)push_size_zero(&platform->frame_arena, MAX_TABLE_WIDGETS*sizeof(Present_Node*));
    presenter->last_lines = presenter->lines;
    presenter->lines = 0;
    presenter->line = 0;
    
    opengl_start_frame();
}

internal void
end_frame(){
    
    opengl_end_frame();
}

#define FRAME defer_loop(start_frame(), end_frame())

BEGIN_C_EXPORT

PERMANENT_LOAD {
    platform = platform_;
    
    platform->globals = push_type_zero(&platform->permanent_arena, Friday_Globals);
    globals = (Friday_Globals*)platform->globals;
    globals->renderer = push_type_zero(&platform->permanent_arena, Renderer_State);
    globals->ui = push_type_zero(&platform->permanent_arena, UI_State);
    globals->editor = push_type_zero(&platform->permanent_arena, Editor_State);
    globals->presenter = push_type_zero(&platform->permanent_arena, Presenter_State);
    
    load_all_opengl_procs();
    globals->renderer->font = load_sdf_font("../fonts/friday_default.fnt");
    initialise_globals();
    init_opengl_renderer();
    init_shaders();
    load_theme_blender();
    
    editor->ast_pool = make_pool(sizeof(Ast_Node));
    editor->arc_pool = make_pool(sizeof(Arc_Node));
    editor->block_pool = make_pool(sizeof(Block));
    
    ui->panel = (Panel*)push_type_zero(&platform->permanent_arena, Panel);
    ui->panel->split_ratio = 1.0f;
    ui->panel->type = PANEL_EDITOR;
    
    ui->editing_string.text = (char*)push_size_zero(&platform->permanent_arena, 8192); //big boi string
    ui->editing_string.length = 0;
    ui->editing_string.capacity = 8192;
    
    ui->zoom_level = 1;
    ui->target_zoom_level = 1;
    
    split_panel(ui->panel, 0.6, PANEL_SPLIT_VERTICAL, PANEL_PROPERTIES);
    split_panel(ui->panel->first, 0.9, PANEL_SPLIT_HORIZONTAL, PANEL_STATUS);
    
    editor->string_pool = make_pool(256); //node strings are capped at 256 chars
    
    auto pool = &editor->ast_pool;
    auto global_scope = make_scope_node(pool); 
    
    _s64 = make_declaration_node(pool, "s64");
    
    {
        global_scope->scope.statements->next = make_function_node(pool, "entry");
        
        auto function_scope = global_scope->scope.statements->next->function.scope;
        
        function_scope->scope.statements->next = make_declaration_node(pool, "test");
        function_scope->scope.statements->next->prev = function_scope->scope.statements;
    }
    
    {
        global_scope->scope.statements->next->next = make_function_node(pool, "add");
        global_scope->scope.statements->next->next->prev = global_scope->scope.statements;
        
        auto function = &global_scope->scope.statements->next->next->function;
        function->parameters->next = make_declaration_node(pool, "a");
        function->parameters->next->prev = function->parameters;
        function->parameters->next->next = make_declaration_node(pool, "b");
        function->parameters->next->next->prev = function->parameters->next;
        
        function->parameters->next->next->next = make_declaration_node(pool, "arg 3");
        function->parameters->next->next->next->prev = function->parameters->next->next;
        
        function->scope->scope.statements->next = make_declaration_node(pool, "Hey Friday!");
        function->scope->scope.statements->next->prev = function->scope->scope.statements;
        serialise_to_disk(function->parameters);
    }
    
    
    
    editor->program = global_scope;
    editor->root = make_arc_node(&editor->arc_pool);
    cursor.arc = editor->root;
    cursor.string = &editor->root->string;
    
    editor->block_start = make_block(&editor->block_pool, "Hello");
    make_child_block(editor->block_start, "Parallel!");
}

HOT_LOAD {
    platform = platform_;
    load_all_opengl_procs();
    initialise_globals();
    load_theme_blender();
}

HOT_UNLOAD {
    
}


UPDATE {
    FRAME
    {
        f32 start = platform->get_time();
        
        draw_backdrop_grid();
        
        {
            v2f delta = {};
            if(has_mouse_scrolled(&delta)){
                ui->target_zoom_level += delta.y*0.005f;
            }
            lerp(&ui->zoom_level, ui->target_zoom_level, 0.1);
        }
        {
            v2f delta = {};
            if(has_mouse_dragged(MOUSE_BUTTON_LEFT, &delta)){
                ui->offset = ui->offset + delta;
            }
        }
        if(has_pressed_key_modified(KEY_L, KEY_MOD_SHIFT)){
            if(ui->active_block){
                ui->active_block = make_child_block(ui->active_block, "test");
            }
        }
        
        if(has_pressed_key_modified(KEY_K, KEY_MOD_SHIFT)){
            if(ui->active_block){
                ui->active_block = make_sibling_block_before(ui->active_block, "test");
            }
        }
        
        if(has_pressed_key_modified(KEY_J, KEY_MOD_SHIFT)){
            if(ui->active_block){
                ui->active_block = make_sibling_block_after(ui->active_block, "test");
            }
        }
        
        if(has_pressed_key(KEY_K)){
            if(ui->active_block){
                if(ui->active_block->next_sibling){
                    ui->active_block = ui->active_block->next_sibling;
                }
            }
        }
        
        if(has_pressed_key(KEY_J)){
            if(ui->active_block){
                if(ui->active_block->prev_sibling){
                    ui->active_block = ui->active_block->prev_sibling;
                }
            }
        }
        
        if(has_pressed_key(KEY_H)){
            if(ui->active_block){
                if(ui->active_block->parent){
                    ui->active_block = ui->active_block->parent;
                }
            }
        }
        
        if(has_pressed_key(KEY_L)){
            if(ui->active_block){
                if(ui->active_block->first_child){
                    ui->active_block = ui->active_block->first_child;
                }
            }
        }
        
        draw_blocks(ui->offset + v2f(platform->window_size.width/2.0,
                                     platform->window_size.height/2.0), editor->block_start);
        ui->hot_block = nullptr;
        platform->refresh_screen();
    }
}

END_C_EXPORT