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
    
    split_panel(ui->panel, 0.7, PANEL_SPLIT_VERTICAL, PANEL_PROPERTIES);
    split_panel(ui->panel->first, 0.92, PANEL_SPLIT_HORIZONTAL, PANEL_STATUS);
    
    editor->string_pool = make_pool(256); //node strings are capped at 256 chars
    
    char* builtins[] = {"s8", "s16", "s32", "s64", "u8", "u16", "u32", "u64", "f32", "f64"};
    
    auto start = make_arc_node(&editor->arc_pool);
    auto builtin = start;
    for(int i = 0; i < 10; i++){
        builtin->string = make_string(builtins[i]);
        builtin->next_sibling = make_arc_node(&editor->arc_pool);
        builtin = builtin->next_sibling;
    }
    
    editor->builtins = start;
    
    auto scope = make_scope(&editor->arc_pool);
    set_as_ast(scope, AST_SCOPE);
    arc_set_property(scope, AP_LIST);
    auto first = make_selectable_arc_node(&editor->arc_pool);
    insert_arc_node_as_child(scope, first);
    
    editor->root = scope;
    presenter->cursor.at = first;
    presenter->cursor.string = &first->string;
    
    editor->views[editor->view_count++] = make_stringf(&editor->string_pool, "default");
    
    presenter->select_first = first;
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
        presenter->number_of_deletions = 0;
        presenter->number_of_deletions_before_cursor = 0;
        presenter->number_of_deletions_after_cursor = 0;
        presenter->pos = 0;
        presenter->delete_queue = nullptr;
        presenter->delete_queue_size = 0;
        
        if(has_pressed_key_modified(KEY_A, KEY_MOD_CTRL) &&
           presenter->cursor.at->ast_type == AST_TYPE_TOKEN){
            presenter->cursor.at->token_type = TOKEN_ARRAY;
        }
        
        if(has_pressed_key_modified(KEY_RIGHT, KEY_MOD_SHIFT)){
            if(!presenter->select_start.at && !presenter->select_end.at){
                presenter->select_start = presenter->cursor;
                presenter->select_end = presenter->cursor;
            }
            
            
            if(presenter->select_start.at == presenter->cursor.at &&
               presenter->select_end.at == presenter->cursor.at){
                
                advance_cursor(&presenter->select_end, CURSOR_RIGHT);
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                
            }else if(presenter->select_end.at == presenter->cursor.at){
                
                advance_cursor(&presenter->select_end, CURSOR_RIGHT);
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                
            }else if(presenter->select_start.at == presenter->cursor.at){
                
                advance_cursor(&presenter->select_start, CURSOR_RIGHT);
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                
            }else {
                assert(0);
            }
            
        }
        
        if(has_pressed_key_modified(KEY_LEFT, KEY_MOD_SHIFT)){
            if(!presenter->select_start.at && !presenter->select_end.at){
                presenter->select_start = presenter->cursor;
                presenter->select_end = presenter->cursor;
                
            }
            
            if(presenter->select_start.at == presenter->cursor.at &&
               presenter->select_end.at == presenter->cursor.at){
                
                advance_cursor(&presenter->select_start, CURSOR_LEFT);
                advance_cursor(&presenter->cursor, CURSOR_LEFT);
                
            }else if(presenter->select_end.at == presenter->cursor.at){
                
                advance_cursor(&presenter->select_end, CURSOR_LEFT);
                advance_cursor(&presenter->cursor, CURSOR_LEFT);
                
            }else if(presenter->select_start.at == presenter->cursor.at){
                
                advance_cursor(&presenter->select_start, CURSOR_LEFT);
                advance_cursor(&presenter->cursor, CURSOR_LEFT);
                
            }else {
                
                assert(0);
            }
            
        }
        
        if(has_pressed_key_modified(KEY_D, KEY_MOD_CTRL)){
            // NOTE(Oliver): this is an upper limit, the real size is N - non_deletable_nodes
            presenter->delete_queue_size = (presenter->end_pos - presenter->start_pos)+1;
            presenter->delete_queue = (Arc_Node**)push_size(&platform->frame_arena, 
                                                            presenter->delete_queue_size*sizeof(Arc_Node*));
            int index = 0;
            for(int i = presenter->start_pos; i <= presenter->end_pos; i++){
                //mark_node_for_deletion(presenter->buffer[i].node);
                if(arc_has_property(presenter->buffer[i].node, AP_DELETABLE)){
                    presenter->delete_queue[index++] = presenter->buffer[i].node;
                }
            }
            presenter->delete_queue_size = index-1;
            advance_cursor(&presenter->cursor, CURSOR_LEFT, 1);
            
            presenter->select_start = {};
            presenter->select_end = {};
            presenter->start_pos = 0;
            presenter->end_pos = 0;
            
        }
        
        if(has_pressed_key(KEY_ESC)){
            
            presenter->select_start = {};
            presenter->select_end = {};
            presenter->start_pos = 0;
            presenter->end_pos = 0;
            
        }
        
        render_panels(ui->panel, v4f(0,platform->window_size.height, 
                                     platform->window_size.width, platform->window_size.height));
        
        ForEachWidgetSibling(ui->root){
            layout_widgets(it);
            render_widgets(it);
        }
        
        if(presenter->last_cursor.at && presenter->last_cursor.at != presenter->cursor.at && 
           (presenter->last_cursor.at->ast_type == AST_TOKEN ||
            presenter->last_cursor.at->ast_type == AST_TYPE_TOKEN) &&
           presenter->last_cursor.at->string.length == 0){
            mark_node_for_deletion(presenter->last_cursor.at);
        }
        if(presenter->cursor.at->ast_type == AST_TOKEN){
            set_token_type(presenter->cursor.at);
        }
        
        if(presenter->cursor.at->ast_type == AST_TYPE_TOKEN){
            set_type_token_type(presenter->cursor.at);
        }
        if(has_pressed_key_modified(KEY_J, KEY_MOD_CTRL)){
            jump_to_declaration();
        }
        
        if(arc_has_property(presenter->cursor.at, AP_AST) &&
           presenter->cursor.at->ast_type == AST_TYPE_TOKEN &&
           presenter->cursor.at->reference){
            if(has_pressed_key_modified(KEY_P, KEY_MOD_CTRL)){
                presenter->cursor.at->number_of_pointers++;
            }else if(has_pressed_key_modified(KEY_P, KEY_MOD_SHIFT | KEY_MOD_CTRL)){
                if(presenter->cursor.at->number_of_pointers <= 0){
                    presenter->cursor.at->number_of_pointers = 0;
                }else {
                    presenter->cursor.at->number_of_pointers--;
                }
            }
        }
        //~ Check
        if(presenter->mode == P_EDIT){
            if(has_pressed_key(KEY_ENTER)){
                presenter->mode = P_CREATE;
            }
        }
        
        if(presenter->mode == P_CREATE){
            
            //~ Node creation keybinds
            if(has_pressed_key(KEY_D)){
                if(!presenter->cursor.at->next_sibling){
                    append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                }
                arc_set_property(presenter->cursor.at, AP_DELETABLE);
                make_declaration_from_node(presenter->cursor.at, &editor->arc_pool);
                auto type = make_selectable_arc_node(&editor->arc_pool);
                arc_remove_property(type, AP_DELETABLE);
                insert_arc_node_as_child(presenter->cursor.at->first_child, type);
                set_as_ast(type, AST_TYPE_TOKEN);
                auto expr = make_selectable_arc_node(&editor->arc_pool);
                arc_remove_property(expr, AP_DELETABLE);
                insert_arc_node_as_child(presenter->cursor.at->last_child, expr);
                set_as_ast(expr, AST_TOKEN);
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                presenter->mode = P_EDIT;
                
            }else if(has_pressed_key(KEY_F)){
                if(!presenter->cursor.at->next_sibling){
                    append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                }
                arc_set_property(presenter->cursor.at, AP_DELETABLE);
                make_function_from_node(presenter->cursor.at, &editor->arc_pool);
                
                auto param = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(presenter->cursor.at->first_child, param);
                
                auto type = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(presenter->cursor.at->first_child->next_sibling, type);
                set_as_ast(type, AST_TYPE_TOKEN);
                
                auto empty = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(presenter->cursor.at->last_child, empty);
                
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                presenter->mode = P_EDIT;
                
            } else if(string_eq(presenter->cursor.at->string, "if")){
                if(!presenter->cursor.at->next_sibling){
                    append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                }
                arc_set_property(presenter->cursor.at, AP_DELETABLE);
                make_if_from_node(presenter->cursor.at, &editor->arc_pool);
                
                auto expr = make_selectable_arc_node(&editor->arc_pool);
                arc_remove_property(expr, AP_DELETABLE);
                set_as_ast(expr, AST_TOKEN);
                insert_arc_node_as_child(presenter->cursor.at->first_child, expr);
                
                auto empty = make_selectable_arc_node(&editor->arc_pool);
                arc_remove_property(expr, AP_DELETABLE);
                insert_arc_node_as_child(presenter->cursor.at->last_child, empty);
                
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                presenter->mode = P_EDIT;
            }else if(string_eq(presenter->cursor.at->string, "using")){
                if(!presenter->cursor.at->next_sibling){
                    append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                }
                arc_set_property(presenter->cursor.at, AP_DELETABLE);
                make_using_from_node(presenter->cursor.at, &editor->arc_pool);
                
                auto empty = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(presenter->cursor.at, empty);
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                
            }else if(string_eq(presenter->cursor.at->string, "for")){
                if(!presenter->cursor.at->next_sibling){
                    append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                }
                arc_set_property(presenter->cursor.at, AP_DELETABLE);
                make_for_from_node(presenter->cursor.at, &editor->arc_pool);
                
                auto init = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(presenter->cursor.at->first_child, init);
                
                auto cond = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(presenter->cursor.at->first_child->next_sibling, cond);
                set_as_ast(cond, AST_TOKEN);
                
                auto stmt = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(presenter->cursor.at->first_child->next_sibling->next_sibling, stmt);
                
                auto body = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(presenter->cursor.at->last_child, body);
                
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                presenter->mode = P_EDIT;
                
            }else if(string_eq(presenter->cursor.at->string, "return")){
                if(!presenter->cursor.at->next_sibling){
                    append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                }
                arc_set_property(presenter->cursor.at, AP_DELETABLE);
                make_return_from_node(presenter->cursor.at, &editor->arc_pool);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                set_as_ast(next, AST_TOKEN);
                insert_arc_node_as_child(presenter->cursor.at->first_child, next);
                presenter->mode = P_EDIT;
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                
            }else if(has_pressed_key(KEY_S)){
                if(!presenter->cursor.at->next_sibling){
                    append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                }
                arc_set_property(presenter->cursor.at, AP_DELETABLE);
                make_struct_from_node(presenter->cursor.at, &editor->arc_pool);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(presenter->cursor.at->first_child, next);
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                presenter->mode = P_EDIT;
                
            }else if(has_pressed_key(KEY_C)){
                if(!presenter->cursor.at->next_sibling){
                    append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                }
                arc_set_property(presenter->cursor.at, AP_DELETABLE);
                
                make_call_from_node(presenter->cursor.at, &editor->arc_pool);
                find_function(presenter->cursor.at);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(presenter->cursor.at->first_child->first_child, next);
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                presenter->mode = P_EDIT;
                
            }else if(presenter->cursor.at->string.length == 0){
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                presenter->mode = P_EDIT;
            }else if(has_pressed_key(KEY_ENTER)){
                auto next = make_selectable_arc_node(&editor->arc_pool);
                insert_arc_node_as_sibling(presenter->cursor.at, next);
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                presenter->mode = P_EDIT;
            }else if(presenter->cursor.at->ast_type == AST_TOKEN ||
                     presenter->cursor.at->ast_type == AST_TYPE_TOKEN){
                
                auto next = make_selectable_arc_node(&editor->arc_pool);
                set_as_ast(next, presenter->cursor.at->ast_type);
                insert_arc_node_as_sibling(presenter->cursor.at, next);
                if(presenter->cursor.at == presenter->cursor.at->parent->last_child){
                    arc_remove_property(next, AP_DELETABLE);
                }
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                presenter->mode = P_EDIT;
            }else if(presenter->cursor.at->ast_type != AST_INVALID && !presenter->cursor.at->next_sibling){
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                presenter->mode = P_EDIT;
            }
        }
        
        //present_debug_arc(v2f(100, platform->window_size.height - 500), editor->root);
        f32 end = platform->get_time();
        time_per_gui_update = end - start;
        
        //NOTE(Oliver): handle input for presenter
        // put this somewhere else
        for(int i = presenter->delete_queue_size-1; i >= 0; i--){
            remove_arc_node_at(&presenter->delete_queue[i]->parent->first_child, 
                               presenter->delete_queue[i]);
        }
        delete_nodes_marked_for_deletion(editor->root);
        build_navigation_buffer(editor->root);
        set_next_cursor_pos(&presenter->cursor);
        set_next_cursor_pos(&presenter->select_start);
        set_next_cursor_pos(&presenter->select_end);
        presenter->last_cursor = presenter->cursor;
        
        for(int i = 1; i < 4; i++){
            presenter->cursors[i].direction = CURSOR_NONE;
            presenter->cursors[i].direction_count = 0;
        }
        
        highlight_reference = nullptr;
        if(platform->frame_count == 0){
            ui->active = presenter->cursor.text_id;
        }
    }
    platform->refresh_screen();
}

END_C_EXPORT