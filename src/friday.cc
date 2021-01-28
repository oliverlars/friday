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
#include "editor.h"
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
    load_theme_dots();
    
    editor->ast_pool = make_pool(sizeof(Ast_Node));
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
        f32 end = platform->get_time();
        time_per_gui_update = end - start;
        
        //NOTE(Oliver): handle input for presenter
        // put this somewhere else
        auto string = &cursor.arc->string;
        
        if(presenter->mode == P_EDIT){
            if(has_pressed_key(KEY_ENTER)){
                auto parent = cursor.arc->parent;
                
                if(parent && parent->ast_type == AST_DECLARATION){
                    
                    arc_set_property(cursor.arc, AP_AST);
                    
                    cursor.arc->ast_type = AST_TYPE_USAGE;
                    
                    auto next = make_arc_node(&editor->arc_pool);
                    insert_arc_node_as_child(cursor.arc->parent->parent, next);
                    cursor.arc = next;
                    
                    presenter->mode = P_EDIT;
                }
                else if(parent && parent->parent->ast_tag == AT_PARAMS){
                    cursor.arc = cursor.arc->parent->next_sibling->first_child;
                }
                else if(parent && parent->ast_tag == AT_RETURN_TYPE){
                    arc_set_property(cursor.arc, AP_AST);
                    
                    cursor.arc->ast_type = AST_TYPE_USAGE;
                    auto scope = cursor.arc->parent->next_sibling->first_child;
                    auto next = make_arc_node(&editor->arc_pool);
                    scope->first_child = next;
                    cursor.arc = next;
                    
                }else if(!string->length){
                    cursor.arc = cursor.arc->parent->next_sibling->first_child;
                }else {
                    presenter->mode = P_CREATE;
                }
            }
        }
        
        if(presenter->mode == P_CREATE){
            if(has_pressed_key(KEY_S)){
                arc_set_property(cursor.arc, AP_AST);
                cursor.arc->ast_type = AST_STRUCT;
                
                cursor.arc->first_child = make_arc_node(&editor->arc_pool);
                cursor.arc->first_child->parent = cursor.arc;
                arc_set_property(cursor.arc->first_child, AP_AST);
                cursor.arc->first_child->ast_type = AST_SCOPE;
                
                
                cursor.arc->first_child->first_child = make_arc_node(&editor->arc_pool);
                cursor.arc->first_child->first_child->parent = cursor.arc->first_child;
                
                cursor.arc = cursor.arc->first_child->first_child;
                presenter->mode = P_EDIT;
            }
            if(has_pressed_key(KEY_D)){
                arc_set_property(cursor.arc, AP_AST);
                cursor.arc->ast_type = AST_DECLARATION;
                auto type = make_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(cursor.arc, type);
                cursor.arc = type;
                presenter->mode = P_EDIT;
            }
            if(has_pressed_key(KEY_F)){
                arc_set_property(cursor.arc, AP_AST);
                
                cursor.arc->ast_type = AST_FUNCTION;
                auto params = make_arc_node(&editor->arc_pool);
                arc_set_property(params, AP_AST_TAG);
                params->ast_tag = AT_PARAMS;
                insert_arc_node_as_child(cursor.arc, params);
                
                auto return_type = make_arc_node(&editor->arc_pool);
                arc_set_property(return_type, AP_AST_TAG);
                return_type->ast_tag = AT_RETURN_TYPE;
                insert_arc_node_as_child(cursor.arc, return_type);
                
                auto body = make_arc_node(&editor->arc_pool);
                arc_set_property(body, AP_AST_TAG);
                body->ast_tag = AT_BODY;
                insert_arc_node_as_child(cursor.arc, body);
                
                auto  first_decl = make_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(params, first_decl);
                
                auto type_expr = make_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(return_type, type_expr);
                
                auto scope = make_arc_node(&editor->arc_pool);
                insert_arc_node_as_child(body, scope);
                
                cursor.arc = first_decl;
                
                presenter->mode = P_EDIT;
            }
            
        }
        
    }
    platform->refresh_screen();
}

END_C_EXPORT