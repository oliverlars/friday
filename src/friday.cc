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
#include "c_backend.h"
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
#include "c_backend.cc"


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
make_builtins(){
    
    char* builtins[] = {"void", "s8", "s16", "s32", "s64", "u8", "u16", "u32", "u64", "f32", "f64", "string"};
    
    auto first_builtin = make_arc_node(&editor->arc_pool);
    set_as_ast(first_builtin, AST_TYPE_TOKEN);
    auto builtin = first_builtin;
    for(int i = 0; i < sizeof(builtins)/sizeof(builtins[0]); i++){
        int length = snprintf(builtin->string.text, 256, "%s", builtins[i]);
        builtin->string.length = length;
        builtin->next_sibling = make_arc_node(&editor->arc_pool);
        set_as_ast(builtin->next_sibling, AST_TYPE_TOKEN);
        builtin = builtin->next_sibling;
    }
    auto print = make_arc_node(&editor->arc_pool);
    set_as_ast(print, AST_FUNCTION);
    int length = snprintf(print->string.text, 256, "print");
    print->string.length = length;
    
    editor->builtins = first_builtin;
    editor->stdlib = print;
    
}

internal void
start_frame(){
    ui->root = nullptr;
    ui->layout_stack = nullptr;
    
    ui->last_widget_table = ui->widget_table;
    ui->widget_table = (Widget**)push_size_zero(&platform->frame_arena, MAX_TABLE_WIDGETS*sizeof(Widget*));
    
    ui->previous_windows = ui->windows;
    ui->previous_window_count = ui->window_count;
    ui->window_count = 0;
    ui->windows = (Widget**)push_size_zero(&platform->frame_arena, 32*sizeof(Widget));
    
    opengl_start_frame();
}

internal void
end_frame(){
    
    opengl_end_frame();
}

#define FRAME defer_loop(start_frame(), end_frame())

BEGIN_C_EXPORT
global int pause = 0;


PERMANENT_LOAD {
    platform = platform_;
    pause = 400;
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
    ui->panel->type = PANEL_HEADER;
    ui->editing_string.text = (char*)push_size_zero(&platform->permanent_arena, 8192); //big boi string
    ui->editing_string.length = 0;
    ui->editing_string.capacity = 8192;
    split_panel(ui->panel, 0.06, PANEL_SPLIT_HORIZONTAL, PANEL_EDITOR, false);
    ui->panel->first->no_pad = true;
    split_panel(ui->panel->second, 0.7, PANEL_SPLIT_VERTICAL, PANEL_PROPERTIES);
    split_panel(ui->panel->second->first, 0.92, PANEL_SPLIT_HORIZONTAL, PANEL_STATUS, false);
    
    editor->string_pool = make_pool(256); //node strings are capped at 256 chars
    
    auto scope = make_scope(&editor->arc_pool);
    set_as_ast(scope, AST_SCOPE);
    arc_set_property(scope, AP_LIST);
    auto first = make_selectable_arc_node(&editor->arc_pool);
    insert_arc_node_as_child(scope, first);
    
    make_builtins();
    
    editor->root = scope;
    
    presenter->cursor.at = first;
    presenter->cursor.string = &first->string;
    
    presenter->indent_level = 20;
    
    editor->views[editor->view_count++] = make_stringf(&editor->string_pool, "default");
    
    editor->file_location = make_stringf(&editor->string_pool, "test.arc");
    
    presenter->select_first = first;
    
    
    ui->logo = make_bitmap("icon.png");
}

HOT_LOAD {
    platform = platform_;
    load_all_opengl_procs();
    initialise_globals();
    load_theme_dots();
}

HOT_UNLOAD {
    
}

internal void
adult_swim_trend(){
    local_persist int ch = 0;
    local_persist int str = 0;
    local_persist f32 indent_offset = 0;
    char* strings[] = {
        "what if", "string", "variables", "could be named", "string", "anything?",
        "what if syntax", "s64", "1", "+", "2",  "didn't matter?", "void",
        "what about tabs v spaces?", "f32", "no more what ifs, its real", "s32",
        "if", "2", ">", "1", "welcome to friday", "s8", "1", "print", "welcom",
    };
    
    char* as = "[as]";
    if(str >= 22){
        indent_offset += 0.05f;
        //presenter->indent_level += 0.5f*sinf(indent_offset);
    }
    if(str == 12){
        present_style = 1;
    }
    if(str == 30){
        present_style = 0;
    }
    if(str == 32){
        indent_offset += 0.02f;
        presenter->indent_level += 0.5f*sinf(indent_offset);
    }
    if(pause > 0){
        pause--;
    }else if((platform->frame_count % 6) == 0){
        switch(str){
            case 0: {
                if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    str++;
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    platform_push_event(platform_key_press(KEY_D, {}));
                    ch = 0;
                }
            }break;
            case 1: {
                if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    str++;
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch = -1;
                }
            }break;
            case 2: {
                if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }
                else if(ch < strlen(strings[str])){
                    if(ch == 0){
                        platform_push_event(platform_key_press(KEY_2, KEY_MOD_CTRL));
                    }
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    str++;
                    ch = -1;
                }
            }break;
            case 3: {
                if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }
                else if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    platform_push_event(platform_key_press(KEY_D, {}));
                    ch = 0;
                    str++;
                }
            }break;
            case 4: {
                if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    str++;
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch = -1;
                }
            }break;
            case 5: {
                if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }else if(ch < strlen(strings[str])){
                    if(ch == 0){
                        platform_push_event(platform_key_press(KEY_2, KEY_MOD_CTRL));
                    }
                    
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    str++;
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch = -1;
                    pause = 100;
                }
                break;
                case 6: {
                    if(ch == -1){
                        platform_push_event(platform_key_press(KEY_ENTER, {}));
                        ch++;
                    }
                    else if(ch < strlen(strings[str])){
                        platform_push_event(platform_character_input(strings[str][ch++]));
                    }else {
                        str++;
                        platform_push_event(platform_key_press(KEY_ENTER, {}));
                        platform_push_event(platform_key_press(KEY_D, {}));
                        ch = 0;
                    }
                }break;
                case 7: {
                    if(ch < strlen(strings[str])){
                        platform_push_event(platform_character_input(strings[str][ch++]));
                    }else {
                        str++;
                        platform_push_event(platform_key_press(KEY_ENTER, {}));
                        ch = -1;
                    }
                }break;
            }break;
            case 8: {
                if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }
                else if(ch < strlen(strings[str])){
                    
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    str++;
                    ch = 0;
                }
            }break;
            
            case 9: {
                
                if(ch < strlen(strings[str])){
                    
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    str++;
                    ch = 0;
                }
            }break;
            
            case 10: {
                if(ch < strlen(strings[str])){
                    
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    str++;
                    ch = -1;
                }
            }break;
            case 11: {
                if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }else if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    str++;
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    platform_push_event(platform_key_press(KEY_D, {}));
                    ch = 0;
                    pause = 100;
                }
            }break;
            
            case 12: {
                if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }else if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch = -2;
                    str++;
                    pause = 40;
                }
            }break;
            case 13: {
                if(ch == -2){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }
                else if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }else if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    str++;
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    platform_push_event(platform_key_press(KEY_D, {}));
                    ch = 0;
                }
            }break;
            
            case 14: {
                if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }else if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch = -2;
                    str++;
                    pause = 30;
                }
            }break;
            case 15: {
                if(ch == -2){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }else if(ch == -1){
                    
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }else if(ch < strlen(strings[str])){
                    if(ch == 17){
                        pause = 50;
                    }
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    pause = 50;
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    platform_push_event(platform_key_press(KEY_F, {}));
                    str++;
                    ch = -1;
                }
            }break;
            case 16: {
                if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }else if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch = -1;
                    str++;
                }
            }break;
            case 17: {
                if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }else if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch = 0;
                    str++;
                }
            }break;
            case 18: {
                if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    str++;
                    ch = 0;
                }
            }break;
            case 19: {
                if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    str++;
                    ch = 0;
                }
            }break;
            case 20: {
                if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    str++;
                    ch = -1;
                }
            }break;
            case 21: {
                if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }else if(ch < strlen(strings[str])){
                    if(ch == 6){
                        pause = 40;
                    }if(ch == 9){
                        pause = 20;
                    }
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    platform_push_event(platform_key_press(KEY_D, {}));
                    str++;
                    ch = 0;
                }
            }break;
            case 22: {
                if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    str++;
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch = -1;
                }
            }break;
            case 23: {
                if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }else if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    str++;
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch = -1;
                }
            }break;
            case 24: {
                if(ch == -1){
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch++;
                }else if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    str++;
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch = 0;
                }
            }break;
            case 25: {
                if(ch < strlen(strings[str])){
                    platform_push_event(platform_character_input(strings[str][ch++]));
                }else {
                    str++;
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                    ch = -1;
                    
                    platform_push_event(platform_key_press(KEY_TAB, {}));
                    platform_push_event(platform_key_press(KEY_ENTER, {}));
                }
            }break;
            case 26: {
                platform_push_event(platform_key_press(KEY_LEFT, {}));
                str++;
            }break;
            case 27:{ 
                platform_push_event(platform_key_press(KEY_UP, {}));
                str++;
            }break;
            case 28:{ 
                platform_push_event(platform_key_press(KEY_LEFT, {}));
                str++;
            }break;
            case 29: {
                platform_push_event(platform_key_press(KEY_LEFT, {}));
                str++;
                ch = strlen("welcome to friday");
                ch = 5;
            }break;
            case 30:{
                if(ch == 5){
                    platform_push_event(platform_key_press(KEY_LEFT, {}));
                    ch--;
                }else if(ch >= 0){
                    pause = 30;
                    platform_push_event(platform_key_press(KEY_BACKSPACE, {KEY_MOD_CTRL}));
                    ch--;
                }else {
                    str++;
                    ch = 0;
                    pause = 40;
                }
            }break;
            case 31: {
                if(ch < strlen(as)){
                    platform_push_event(platform_character_input(as[ch++]));
                }else {
                    str++;
                }
            }break;
        }
    }
}

f32 graph_height = 100.0f;
f32 graph_width = 400.0f;
f32 delta_times[64] = {};

internal void
frame_graph(){
    String8 string = make_string("flame graph");
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_FIRST_TRANSITION);
    widget_set_property(widget, WP_RENDER_BORDER);
    
    auto render_hook = [](Widget* widget ){
        RENDER_CLIP(v4f2(widget->pos - v2f(0, widget->min.height), widget->min)){
            f32 max_delta = 15.0f;
            f32 x = widget->pos.x;
            for(int i = 0; i < 64; i++){
                f32 height = (delta_times[i]/max_delta)*widget->min.height;
                f32 width = (widget->min.width)/64.0f;
                push_rectangle(v4f2(v2f(x, widget->pos.y - widget->min.height), 
                                    v2f(width, height)), 0, ui->theme.text);
                x += width;
            }
        }
        
    };
    auto result = update_widget(widget);
    widget->min.height = 100;
    widget->render_hook = render_hook;
}


UPDATE {
    FRAME
    {
        
        platform->reset_cursor();
        f32 start = platform->get_time();
        presenter->number_of_deletions = 0;
        presenter->number_of_deletions_before_cursor = 0;
        presenter->number_of_deletions_after_cursor = 0;
        presenter->pos = 0;
        presenter->delete_queue = nullptr;
        presenter->delete_queue_size = 0;
        
        
        //adult_swim_trend();
        
        f32 amount = 0;
        if(has_mouse_scrolled(&amount)){
            next_font_scale += amount/1000.0f;
        }
        animate(&font_scale, next_font_scale, 0.1f);
        
        if(has_pressed_key_modified(KEY_LBRACKET, KEY_MOD_CTRL)){
            if(presenter->cursor.at->ast_type == AST_TOKEN){
                
                presenter->cursor.at->token_type = TOKEN_ARRAY;
                auto after = make_selectable_arc_node(&editor->arc_pool);
                set_as_ast(after, AST_TOKEN);
                insert_arc_node_as_sibling(presenter->cursor.at, after);
                auto expr = make_arc_node(&editor->arc_pool);
                set_as_ast(expr, AST_EXPR);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                set_as_ast(next, AST_TOKEN);
                insert_arc_node_as_child(presenter->cursor.at, expr);
                insert_arc_node_as_child(expr, next);
                //advance_cursor(&presenter->cursor, CURSOR_RIGHT);
            }else {
                presenter->cursor.at->token_type = TOKEN_ARRAY;
            }
        }else if(has_pressed_key_modified(KEY_FULLSTOP, KEY_MOD_CTRL)){
            if(presenter->cursor.at->ast_type == AST_TOKEN){
                auto next = make_selectable_arc_node(&editor->arc_pool);
                set_as_ast(next, AST_TOKEN);
                next->token_type = TOKEN_REFERENCE;
                insert_arc_node_as_child(presenter->cursor.at, next);
                advance_cursor(&presenter->cursor, CURSOR_LEFT);
            }
        }else if(has_pressed_key_modified(KEY_2, KEY_MOD_CTRL)){
            
            presenter->cursor.at->token_type = TOKEN_STRING;
            presenter->mode = P_EDIT;
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
        render_popup();
        // NOTE(Oliver): background
        Colour backdrop = ui->theme.background;
        backdrop.r /= 1.2f;
        backdrop.g /= 1.2f;
        backdrop.b /= 1.2f;
        push_rectangle(v4f(0,0, platform->window_size.width,platform->window_size.height), 1, backdrop);
        f32 widget_start = platform->get_time();
        
        
        ForEachWidgetSibling(ui->root){
            layout_widgets(it);
            render_widgets(it);
        }
        
        f32 widget_end = platform->get_time();
        if(widget_end - widget_start > 0.001){
        }
        //log("number of widgets: %d",number_of_widgets);
        //if(number_of_widgets != 28) assert(0);
        number_of_widgets = 0;
        Colour select_colour;
        select_colour.packed = 0xB73E4Aff;
        select_colour.a *= 0.4f;
        push_rectangle(union_rects(presenter->select_first_rect,
                                   presenter->select_second_rect),
                       3, select_colour);
        
        //push_bezier(platform->mouse_position, v2f(50, 50), v2f(200, 200), 1, ui->theme.text);
        delta_times[platform->frame_count%64] = platform->dt;
        
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
        Arc_Node* would_be_reference = nullptr;
        if(presenter->mode == P_CREATE){
            
            //~ Node creation keybinds
            if((presenter->cursor.at->ast_type == AST_TOKEN && 
                presenter->cursor.at->reference) ||
               (presenter->cursor.at->prev_sibling &&
                presenter->cursor.at->prev_sibling->token_type == TOKEN_ARRAY) ||
               ((presenter->cursor.at->token_type == TOKEN_ARRAY ||
                 presenter->cursor.at->token_type == TOKEN_REFERENCE) &&
                presenter->cursor.at->prev_sibling &&
                presenter->cursor.at->prev_sibling->token_type == TOKEN_REFERENCE)){
                
                if(has_pressed_key(KEY_LBRACKET)){
                    if(!presenter->cursor.at->next_sibling){
                        auto next = make_selectable_arc_node(&editor->arc_pool);
                        set_as_ast(next, AST_TOKEN);
                        insert_arc_node_as_sibling(presenter->cursor.at, next);
                    }
                    Arc_Node* after = presenter->cursor.at;
                    if(presenter->cursor.at->string.length){
                        after = make_selectable_arc_node(&editor->arc_pool);
                        set_as_ast(after, AST_TOKEN);
                        insert_arc_node_as_sibling(presenter->cursor.at, after);
                        advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                    }else {
                        set_as_ast(after, AST_TOKEN);
                        presenter->cursor.at->token_type = TOKEN_ARRAY;
                    }
                    auto expr = make_arc_node(&editor->arc_pool);
                    arc_remove_property(expr, AP_DELETABLE);
                    set_as_ast(expr, AST_EXPR);
                    
                    auto next = make_selectable_arc_node(&editor->arc_pool);
                    after->token_type = TOKEN_ARRAY;
                    
                    set_as_ast(next, AST_TOKEN);
                    insert_arc_node_as_child(after, expr);
                    insert_arc_node_as_child(expr, next);
                    presenter->mode = P_EDIT;
                }else if(has_pressed_key(KEY_FULLSTOP)){
                    if(!presenter->cursor.at->next_sibling){
                        auto next = make_selectable_arc_node(&editor->arc_pool);
                        set_as_ast(next, AST_TOKEN);
                        insert_arc_node_as_sibling(presenter->cursor.at, next);
                    }
                    Arc_Node* prev_ref = nullptr;
                    if(find_previous_reference(presenter->cursor.at, &prev_ref)){
                        auto next = make_selectable_arc_node(&editor->arc_pool);
                        set_as_ast(next, AST_TOKEN);
                        insert_arc_node_as_sibling(presenter->cursor.at, next);
                        advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                        presenter->mode = P_EDIT;
                    }
                }else if(has_pressed_key(KEY_ENTER)){
                    presenter->mode = P_EDIT;
                    advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                }
            }else if(has_pressed_key(KEY_D)){
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
                
            }else if(string_eq(presenter->cursor.at->string, "foreign")){
                if(!presenter->cursor.at->next_sibling){
                    append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                }
                arc_set_property(presenter->cursor.at, AP_DELETABLE);
                make_foreign_from_node(presenter->cursor.at, &editor->arc_pool);
            }else if(string_eq(presenter->cursor.at->string, "if")){
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
            }else if(string_eq(presenter->cursor.at->string, "while")){
                if(!presenter->cursor.at->next_sibling){
                    append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                }
                arc_set_property(presenter->cursor.at, AP_DELETABLE);
                make_while_from_node(presenter->cursor.at, &editor->arc_pool);
                
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
                
            }else if(string_eq(presenter->cursor.at->string, "new")){
                
                if(!presenter->cursor.at->next_sibling){
                    append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                }
                
                arc_set_property(presenter->cursor.at, AP_DELETABLE);
                make_new_from_node(presenter->cursor.at, &editor->arc_pool);
                auto next = make_selectable_arc_node(&editor->arc_pool);
                set_as_ast(next, AST_TYPE_TOKEN);
                insert_arc_node_as_child(presenter->cursor.at, next);
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
                if(presenter->cursor.at->parent &&
                   presenter->cursor.at->parent->parent &&
                   presenter->cursor.at->parent->parent->parent &&
                   presenter->cursor.at->parent->parent->parent->ast_type == AST_CALL &&
                   !presenter->cursor.at->next_sibling &&
                   presenter->cursor.at->prev_sibling){
                    // NOTE(Oliver): shitty special casing for function calls :(
                    auto expr = make_arc_node(&editor->arc_pool);
                    set_as_ast(expr, AST_EXPR);
                    auto next = make_selectable_arc_node(&editor->arc_pool);
                    set_as_ast(next, AST_TOKEN);
                    insert_arc_node_as_sibling(presenter->cursor.at->parent, expr);
                    insert_arc_node_as_child(expr, next);
                }
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
            }else if(can_resolve_reference(presenter->cursor.at, &would_be_reference)){
                
                if(would_be_reference->ast_type == AST_FUNCTION){
                    if(!presenter->cursor.at->next_sibling){
                        append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                    }
                    arc_set_property(presenter->cursor.at, AP_DELETABLE);
                    make_call_from_node(presenter->cursor.at, &editor->arc_pool);
                    presenter->cursor.at->reference = would_be_reference;
                    replace_string(&presenter->cursor.at->string, would_be_reference->string);
                    auto next = make_selectable_arc_node(&editor->arc_pool);
                    set_as_ast(next, AST_TOKEN);
                    insert_arc_node_as_child(presenter->cursor.at->first_child->first_child, next);
                    advance_cursor(&presenter->cursor, CURSOR_RIGHT);
                    presenter->mode = P_EDIT;
                    
                }else {
                    if(!arc_has_property(presenter->cursor.at, AP_AST)){
                        
                        if(!presenter->cursor.at->next_sibling){
                            append_empty_arc_node(presenter->cursor.at, &editor->arc_pool);
                        }
                        
                        make_assignment_from_node(presenter->cursor.at, &editor->arc_pool);
                        auto next = make_selectable_arc_node(&editor->arc_pool);
                        next->string = presenter->cursor.at->string;
                        next->reference = presenter->cursor.at->reference;
                        set_as_ast(next, AST_TOKEN);
                        insert_arc_node_as_child(presenter->cursor.at->first_child, next);
                        
                        auto empty = make_selectable_arc_node(&editor->arc_pool);
                        set_as_ast(empty, AST_TOKEN);
                        insert_arc_node_as_child(presenter->cursor.at->last_child, empty);
                        
                    }
                }
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
            //ui->active = presenter->cursor.text_id;
        }
        
        if(editor->should_reload){
            pool_free(&editor->arc_pool);
            editor->arc_pool = make_pool(sizeof(Arc_Node));
            editor->root = deserialise(editor->file_location);
            globals->presenter = push_type_zero(&platform->permanent_arena, Presenter_State);
            presenter = globals->presenter;
            ui->editing_string.length = 0;
            ui->active = -1;
            ui->hot = -1;
            ui->cursor_pos = 0;
            presenter->cursor.at = editor->root->first_child;
            presenter->cursor.string = &editor->root->first_child->string;
            presenter->buffer_index = 0;
            presenter->line_index = 0;
            editor->should_reload = false;
            highlight_reference = nullptr;
            presenter->last_cursor = {};
            make_builtins();
            fix_references(editor->root);
            
        }
        presenter->select_first_rect = v4f(0,0,0,0);
        presenter->select_second_rect = v4f(0,0,0,0);
        //font_scale += 0.0001f;
        
    }
    platform->refresh_screen();
}

END_C_EXPORT