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
        
        f32 start = platform->get_time();
        
        //update_panel_split(ui->panel, platform->mouse_position.x/platform->window_size.width);
        render_panels(ui->panel, v4f(0,platform->window_size.height, 
                                     platform->window_size.width, platform->window_size.height));
        
        ForEachWidgetSibling(ui->root){
            layout_widgets(it);
            render_widgets(it);
        }
        //present_debug_arc(v2f(100, 400), editor->root);
        
        if(cursor.at->ast_type == AST_TOKEN){
            set_token_type(cursor.at);
        }
        if(has_pressed_key_modified(KEY_J, KEY_MOD_CTRL)){
            jump_to_declaration();
        }
        
        //~ Check
        if(presenter->mode == P_EDIT){
            
            if(has_pressed_key(KEY_ENTER)){
                if(cursor.at->ast_type == AST_INVALID){
                    presenter->mode = P_CREATE;
                }else if(cursor.at->ast_type == AST_TOKEN){
                    if(cursor.at->string.length){
                        auto token = make_selectable_arc_node(&editor->arc_pool);
                        set_as_ast(token, AST_TOKEN);
                        arc_set_property(token, AP_DELETABLE);
                        insert_arc_node_as_sibling(cursor.at, token);
                        advance_cursor(CURSOR_RIGHT);
                    }else {
                        auto next_in_scope = make_selectable_arc_node(&editor->arc_pool);
                        arc_set_property(next_in_scope, AP_DELETABLE);
                        Arc_Node* member;
                        assert(find_sub_node_of_list(cursor.at, &member));
                        insert_arc_node_as_sibling(member, next_in_scope);
                        if(cursor.at->prev_sibling){
                            mark_node_for_deletion(cursor.at);
                        }
                        advance_cursor(CURSOR_RIGHT);
                    }
                }else {
                    advance_cursor(CURSOR_RIGHT);
                }
            }
        }
        
        if(presenter->mode == P_CREATE){
            
            //~ Node creation keybinds
            if(has_pressed_key(KEY_D)){
                arc_set_property(cursor.at, AP_DELETABLE);
                make_declaration_from_node(cursor.at, &editor->arc_pool);
                auto type = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->first_child, type);
                set_as_ast(type, AST_TYPE_USAGE);
                auto expr = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->last_child, expr);
                set_as_ast(expr, AST_TOKEN);
                advance_cursor(CURSOR_RIGHT);
                presenter->mode = P_EDIT;
            }else if(has_pressed_key(KEY_F)){
                arc_set_property(cursor.at, AP_DELETABLE);
                make_function_from_node(cursor.at, &editor->arc_pool);
                
                auto param = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->first_child, param);
                
                auto type = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->first_child->next_sibling, type);
                set_as_ast(type, AST_TYPE_USAGE);
                
                auto empty = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->last_child, empty);
                
                advance_cursor(CURSOR_RIGHT);
                presenter->mode = P_EDIT;
            } else if(string_eq(cursor.at->string, "if")){
                arc_set_property(cursor.at, AP_DELETABLE);
                make_if_from_node(cursor.at, &editor->arc_pool);
                
                auto expr = make_selectable_arc_node(&editor->arc_pool);
                set_as_ast(expr, AST_TOKEN);
                insert_arc_node_as_child(cursor.at->first_child, expr);
                
                auto empty = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->last_child, empty);
                
                advance_cursor(CURSOR_RIGHT);
                presenter->mode = P_EDIT;
            } else if(string_eq(cursor.at->string, "return")){
                make_return_from_node(cursor.at, &editor->arc_pool);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                set_as_ast(next, AST_TOKEN);
                insert_arc_node_as_child(cursor.at->first_child, next);
                cursor.at = next;
                presenter->mode = P_EDIT;
            }else if(has_pressed_key(KEY_S)){
                arc_set_property(cursor.at, AP_DELETABLE);
                make_struct_from_node(cursor.at, &editor->arc_pool);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->first_child, next);
                cursor.at = next;
                presenter->mode = P_EDIT;
            } else if(has_pressed_key(KEY_C)){
                arc_set_property(cursor.at, AP_DELETABLE);
                make_call_from_node(cursor.at, &editor->arc_pool);
                find_function(cursor.at);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.at->first_child->first_child, next);
                cursor.at = next;
                presenter->mode = P_EDIT;
            }else if(cursor.at->string.length == 0){
                advance_cursor(CURSOR_RIGHT);
                presenter->mode = P_EDIT;
            }
        }
        
        //present_debug_arc(v2f(100, platform->window_size.height - 500), editor->root);
        f32 end = platform->get_time();
        time_per_gui_update = end - start;
        
        //NOTE(Oliver): handle input for presenter
        // put this somewhere else
        delete_nodes_marked_for_deletion(editor->root);
        build_navigation_buffer(editor->root);
        set_next_cursor_pos();
        presenter->direction = CURSOR_NONE;
        presenter->direction_count = 0;
        
        
        last_cursor = cursor;
        highlight_reference = nullptr;
        if(platform->frame_count == 0){
            ui->active = cursor.text_id;
        }
    }
    platform->refresh_screen();
}

END_C_EXPORT