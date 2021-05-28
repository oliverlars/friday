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
    load_theme_dots();
    
    editor->arc_pool = make_pool(sizeof(Arc_Node));
    
    ui->panel = (Panel*)push_type_zero(&platform->permanent_arena, Panel);
    ui->panel->split_ratio = 1.0f;
    ui->panel->type = PANEL_EDITOR;
    
    ui->editing_string.text = (char*)push_size_zero(&platform->permanent_arena, 8192); //big boi string
    ui->editing_string.length = 0;
    ui->editing_string.capacity = 8192;
    
    split_panel(ui->panel, 0.6, PANEL_SPLIT_VERTICAL, PANEL_PROPERTIES);
    split_panel(ui->panel->first, 0.9, PANEL_SPLIT_HORIZONTAL, PANEL_STATUS);
    
    editor->string_pool = make_pool(256); //node strings are capped at 256 chars
    
    
    auto scope = make_scope(&editor->arc_pool);
    set_as_ast(scope, AST_SCOPE);
    arc_set_property(scope, AP_LIST);
    auto first = make_selectable_arc_node(&editor->arc_pool);
    insert_arc_node_as_child(scope, first);
    
    editor->root = scope;
    cursor.at = first;
    cursor.string = &first->string;
    
}

HOT_LOAD {
    platform = platform_;
    load_all_opengl_procs();
    initialise_globals();
    load_theme_dots();
}

HOT_UNLOAD {
    
}



UPDATE {
    FRAME
    {
        
        presenter->buffer_pos = 0;
        presenter->direction = CURSOR_NONE;
        presenter->buffer = (Present_Node*)push_size_zero(&platform->frame_arena, sizeof(Present_Node)*8192);
        
        presenter->line_pos = 0;
        presenter->lines = (Line_Info*)push_size_zero(&platform->frame_arena, sizeof(Line_Info)*8192);
        
        f32 start = platform->get_time();
        //update_panel_split(ui->panel, platform->mouse_position.x/platform->window_size.width);
        render_panels(ui->panel, v4f(0,platform->window_size.height, 
                                     platform->window_size.width, platform->window_size.height));
        
        ForEachWidgetSibling(ui->root){
            layout_widgets(it);
            render_widgets(it);
        }
        //present_debug_arc(v2f(100, platform->window_size.height - 500), editor->root);
        f32 end = platform->get_time();
        time_per_gui_update = end - start;
        
        //NOTE(Oliver): handle input for presenter
        // put this somewhere else
        
        if(has_pressed_key_modified(KEY_J, KEY_MOD_CTRL)){
            jump_to_declaration();
        }
        
        if(presenter->mode == P_EDIT){
            
            if(has_pressed_key(KEY_ENTER)){
                presenter->mode = P_CREATE;
            }
        }
        
        if(presenter->mode == P_CREATE){
            
            Arc_Node* result;
            if(is_sub_node_of_ast_type(cursor.at, AST_DECLARATION, &result)){
                auto decl = result;
                if(cursor.at->ast_type == AST_TYPE_USAGE){
                    auto next = make_selectable_arc_node(&editor->arc_pool);
                    insert_arc_node_as_child(decl->last_child, next);
                    cursor.at = next;
                    set_as_ast(next, AST_TOKEN);
                    presenter->mode = P_EDIT;
                }else if(cursor.at->ast_type == AST_TOKEN){
                    go_to_or_make_next();
                    presenter->mode = P_EDIT;
                }
            }
            
            
            //~ Node creation tool
            if(has_pressed_key(KEY_D)){
                make_declaration_from_node(cursor.at, &editor->arc_pool);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->first_child, next);
                set_as_ast(next, AST_TYPE_USAGE);
                cursor.at = next;
                presenter->mode = P_EDIT;
            }
            
            if(has_pressed_key(KEY_F)){
                make_function_from_node(cursor.at, &editor->arc_pool);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->first_child, next);
                cursor.at = next;
                presenter->mode = P_EDIT;
            }
            if(string_eq(cursor.at->string, "if")){
                make_if_from_node(cursor.at, &editor->arc_pool);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->first_child, next);
                cursor.at = next;
                presenter->mode = P_EDIT;
            }
            if(string_eq(cursor.at->string, "return")){
                make_return_from_node(cursor.at, &editor->arc_pool);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->first_child, next);
                cursor.at = next;
                presenter->mode = P_EDIT;
            }
            if(has_pressed_key(KEY_S)){
                make_struct_from_node(cursor.at, &editor->arc_pool);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->first_child, next);
                cursor.at = next;
                presenter->mode = P_EDIT;
            }
            if(has_pressed_key(KEY_C)){
                make_call_from_node(cursor.at, &editor->arc_pool);
                find_function(cursor.at);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->first_child, next);
                cursor.at = next;
                presenter->mode = P_EDIT;
            }
        }
        
        last_cursor = cursor;
        set_next_cursor_pos();
        highlight_reference = nullptr;
        if(platform->frame_count == 0){
            ui->active = cursor.text_id;
        }
    }
    platform->refresh_screen();
}

END_C_EXPORT