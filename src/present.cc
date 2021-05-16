
internal Arc_Node*
find_next_selectable(Arc_Node* node){
    if(!node) return nullptr;
    
    while(node){
        if(arc_has_property(node, AP_SELECTABLE)) return node;
        if(node->first_child){
            find_next_selectable(node->first_child);
        }
        node = node->next_sibling;
    }
}


internal void
set_prev_selectable(Arc_Node* node){
    if(!node) return;
    
    while(node){
        if(arc_has_property(node, AP_SELECTABLE)) {
            cursor.at = node;
            return;
        }
        if(node->last_child){
            set_prev_selectable(node->last_child);
        }
        if(!node->prev_sibling){
            node = node->parent;
        }else{
            node = node->prev_sibling;
        }
    }
}

internal void
advance_cursor(Cursor_Direction dir){
    if(!cursor.at) return;
    int pos = 0;
    switch(dir){
        case CURSOR_UP:{
            
        }break;
        case CURSOR_DOWN:{
            
        }break;
        case CURSOR_LEFT:{
            auto current = cursor.at;
            if(cursor.at->prev_sibling){
                set_prev_selectable(cursor.at->prev_sibling);
            }else{
                set_prev_selectable(cursor.at->parent);
            }
            pos = cursor.at->string.length;
        }break;
        case CURSOR_RIGHT:{
            if(cursor.at->next_sibling){
                cursor.at = cursor.at->next_sibling;
            }else {
                if(cursor.at->parent){
                    cursor.at = find_next_selectable(cursor.at->parent->next_sibling);
                }
            }
        }break;
    }
    
    ui->cursor_pos = pos;
    
}

internal void
present_string(Colour colour, String8 string){
    
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_FIRST_TRANSITION);
    widget->style.text_colour = v4f_from_colour(colour);
    
    auto render_hook = [](Widget* widget ){
        
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        v4f bbox = v4f2(pos, widget->min);
        
        push_string(pos, widget->string, colour_from_v4f(widget->style.text_colour), widget->style.font_scale);
        
    };
    
    
    widget->render_hook = render_hook;
    auto result = update_widget(widget);
    
    
    Widget_Style style = {
        v4f_from_colour(colour),
        v4f_from_colour(ui->theme.text),
        font_scale,
    };
    widget->style = style;
    
    v2f size = get_text_size(widget->string, widget->style.font_scale);
    widget->min = size;
    
    
}

internal void
present_space() {
    f32 space = get_text_width(" ", font_scale);
    xspacer(space);
}

internal void
edit_text(Widget* widget){
    if(presenter->mode == P_CREATE) return;
    clampi(&ui->cursor_pos, 0, ui->editing_string.length);
    auto last_widget = get_last_widget(widget->id, widget->string);
    
    if(has_pressed_key(KEY_ENTER)){
        presenter->mode = P_CREATE;
    }
    
    if(has_pressed_key(KEY_UP)){
        advance_cursor(CURSOR_UP);
    }
    if(has_pressed_key(KEY_DOWN)){
        advance_cursor(CURSOR_DOWN);
    }
    
    if(has_pressed_key_modified(KEY_LEFT, KEY_MOD_CTRL)){
        if(ui->editing_string.text[ui->cursor_pos-1] == ' '){
            while(ui->editing_string.text[ui->cursor_pos-1] == ' '){
                ui->cursor_pos--;
            }
        }else {
            while(ui->editing_string.text[ui->cursor_pos-1] != ' ' &&
                  ui->cursor_pos >= 0){
                ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
            }
        }
    }
    
    if(has_pressed_key_modified(KEY_RIGHT, KEY_MOD_CTRL)){
        if(ui->editing_string.text[ui->cursor_pos] == ' '){
            while(ui->editing_string.text[ui->cursor_pos] == ' '){
                ui->cursor_pos++;
            }
        }else{
            while(ui->editing_string.text[ui->cursor_pos] != ' ' &&
                  ui->cursor_pos <= ui->editing_string.length){
                ui->cursor_pos++;
            }
        }
    }
    
    if(has_pressed_key(KEY_LEFT)){
        if(ui->cursor_pos == 0){
            
            advance_cursor(CURSOR_LEFT);
        }else {
            ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
        }
    }
    if(has_pressed_key(KEY_RIGHT)){
        if(ui->cursor_pos == ui->editing_string.length){
            advance_cursor(CURSOR_RIGHT);
        }else{
            ui->cursor_pos = ui->cursor_pos < ui->editing_string.length ? ui->cursor_pos +1: ui->editing_string.length;
        }
    }
    if(has_pressed_key_modified(KEY_BACKSPACE, KEY_MOD_CTRL)){
        if(ui->editing_string.text[ui->cursor_pos-1] == ' '){
            pop_from_string(&ui->editing_string.string, ui->cursor_pos);
            ui->cursor_pos--;
        }else {
            while(ui->editing_string.text[ui->cursor_pos-1] != ' ' &&
                  ui->cursor_pos >= 0){
                pop_from_string(&ui->editing_string.string, ui->cursor_pos);
                ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
            }
        }
    }
    
    if(has_pressed_key(KEY_BACKSPACE)){
        if(ui->cursor_pos){
            pop_from_string(&ui->editing_string.string, ui->cursor_pos);
            ui->cursor_pos--;
        }else {
            remove_arc_node_at(&cursor.at->parent->first_child, cursor.at);
            advance_cursor(CURSOR_LEFT);
        }
    }
    
    if(has_pressed_key(KEY_END)){
        ui->cursor_pos = ui->editing_string.length;
    }
    
    if(has_pressed_key(KEY_HOME)){
        ui->cursor_pos = 0;
    }
    
    Platform_Event* event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_CHARACTER_INPUT){
            char c = event->character;
            insert_in_string(&ui->editing_string.string,
                             &c,
                             ui->cursor_pos++);
            platform_consume_event(event);
        }
    }
}

internal b32
check_dropdown(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    auto widget = push_widget(string);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_LERP_POSITION);
    
    auto render_hook = [](Widget* widget){
        
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        v4f bbox = v4f2(pos, widget->min);
        
        if(widget->checked){
            push_string(pos, make_string("+"), colour_from_v4f(widget->style.text_colour), font_scale);
        }else{
            push_string(pos, make_string("-"), colour_from_v4f(widget->style.text_colour), font_scale);
        }
        
        
    };
    
    auto result = update_widget(widget);
    widget->min = get_text_size("+", font_scale);
    if(result.clicked){
        widget->checked = !widget->checked;
    }
    widget->render_hook = render_hook;
    
    return widget->checked;
}

//~ Presenter 2.0

internal bool
arc_has_property(Arc_Node* arc, Arc_Property property);
internal void
arc_set_property(Arc_Node* arc, Arc_Property property);
internal void
arc_remove_property(Arc_Node* arc, Arc_Property property);


internal void
edit_text(Arc_Node* node){
    if(presenter->mode == P_CREATE) return;
    clampi(&ui->cursor_pos, 0, node->string.length);
    auto string = &node->string;
    
    
    if(has_pressed_key(KEY_UP)){
        advance_cursor(CURSOR_UP);
    }
    if(has_pressed_key(KEY_DOWN)){
        advance_cursor(CURSOR_DOWN);
    }
    
    if(has_pressed_key_modified(KEY_LEFT, KEY_MOD_CTRL)){
        if(string->text[ui->cursor_pos-1] == ' '){
            while(string->text[ui->cursor_pos-1] == ' '){
                ui->cursor_pos--;
            }
        }else {
            while(string->text[ui->cursor_pos-1] != ' ' &&
                  ui->cursor_pos >= 0){
                ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
            }
        }
    }
    
    if(has_pressed_key_modified(KEY_RIGHT, KEY_MOD_CTRL)){
        if(string->text[ui->cursor_pos] == ' '){
            while(string->text[ui->cursor_pos] == ' '){
                ui->cursor_pos++;
            }
        }else{
            while(string->text[ui->cursor_pos] != ' ' &&
                  ui->cursor_pos <= string->length){
                ui->cursor_pos++;
            }
        }
    }
    
    if(has_pressed_key(KEY_LEFT)){
        if(ui->cursor_pos == 0){
            advance_cursor(CURSOR_LEFT);
        }else {
            ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
        }
    }
    if(has_pressed_key(KEY_RIGHT)){
        if(ui->cursor_pos == string->length){
            advance_cursor(CURSOR_RIGHT);
        }else{
            ui->cursor_pos = ui->cursor_pos < string->length ? ui->cursor_pos +1: string->length;
        }
    }
    if(has_pressed_key_modified(KEY_BACKSPACE, KEY_MOD_CTRL)){
        if(string->text[ui->cursor_pos-1] == ' '){
            pop_from_string(string, ui->cursor_pos);
            ui->cursor_pos--;
        }else {
            while(string->text[ui->cursor_pos-1] != ' ' &&
                  ui->cursor_pos >= 0){
                pop_from_string(string, ui->cursor_pos);
                ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
            }
        }
    }
    
    if(has_pressed_key(KEY_BACKSPACE)){
        if(ui->cursor_pos){
            pop_from_string(string, ui->cursor_pos);
            ui->cursor_pos--;
        }else {
            remove_arc_node_at(&cursor.at->parent->first_child, cursor.at);
            advance_cursor(CURSOR_LEFT);
        }
    }
    
    if(has_pressed_key(KEY_END)){
        ui->cursor_pos = string->length;
    }
    
    if(has_pressed_key(KEY_HOME)){
        ui->cursor_pos = 0;
    }
    
    Platform_Event* event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_CHARACTER_INPUT){
            insert_in_string(string,
                             event->character,
                             ui->cursor_pos++);
            //platform_consume_event(event);
        }
    }
}

internal void present_editable_string(Colour colour, Arc_Node* node);

internal void set_token_type(Arc_Node* node);

internal void
present_editable_reference(Colour colour, Arc_Node* node){
    node->string = make_stringf(&platform->frame_arena, "%.*s", node->reference->string.length,
                                node->reference->string.text);
    present_editable_string(colour, node);
    set_token_type(node);
}

internal void
present_editable_string(Colour colour, Arc_Node* node){
    
    auto string = &node->string;
    auto widget_string = make_stringf(&platform->frame_arena, "edit_string%d", (int)node);
    auto widget = push_widget(widget_string);
    
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_ALT_STRING);
    widget_set_property(widget, WP_FIRST_TRANSITION);
    widget->alt_string = node->string;
    widget->style.text_colour = v4f_from_colour(colour);
    widget->style.text_colour.a = 0;
    
    widget->arc = node;
    
    if(ui->hot == widget->id){
        highlight_reference = node->reference;
    }
    
    auto render_hook = [](Widget* widget ){
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        v4f bbox = v4f2(pos, widget->min);
        
        push_string(pos, widget->alt_string, colour_from_v4f(widget->style.text_colour), widget->style.font_scale);
        
        v4f underline = v4f(bbox.x, bbox.y, bbox.width, 3);
        if(highlight_reference && (widget->arc->reference == highlight_reference || widget->arc == highlight_reference)){
            push_rectangle(underline, 1, colour_from_v4f(v4f(1,0,0,1)));
        }
        
        if(ui->active == widget->id){
            v2f next = {};
            next.x = pos.x + get_text_width_n(widget->alt_string, ui->cursor_pos, widget->style.font_scale);
            next.y = bbox.y;
            lerp(&cursor.pos.x, next.x, 0.4f);
            lerp(&cursor.pos.y, next.y, 0.4f);
            
            if(presenter->mode == P_CREATE){
                push_rectangle(v4f2(cursor.pos, v2f(2, widget->min.height)), 1, colour_from_v4f(v4f(1,0,0,1)));
            }else {
                push_rectangle(v4f2(cursor.pos, v2f(2, widget->min.height)), 1, ui->theme.cursor);
            }
        }
        
    };
    
    widget->render_hook = render_hook;
    
    // NOTE(Oliver): custom text edit
    {
        if(cursor.at == node){
            ui->active = widget->id;
            if(node->reference) highlight_reference = node->reference;
            edit_text(cursor.at);
        }
    }
    
    auto result = update_widget(widget);
    
    Widget_Style style = {
        v4f_from_colour(colour),
        v4f_from_colour(ui->theme.text),
        font_scale,
    };
    widget->style = style;
    
    if(ui->active == widget->id){
        v2f size = get_text_size(widget->alt_string, widget->style.font_scale);
        widget->min = size;
    }else {
        v2f size = get_text_size(widget->alt_string, widget->style.font_scale);
        widget->min = size;
    }
    
    if(result.clicked){
        ui->cursor_pos = string->length;
    }
    
    if(result.hovered && node->reference){
        highlight_reference = node->reference;
    }
    
}


internal void present_arc(Arc_Node* node);



internal void
present_struct(Arc_Node* node){
    ID("struct%d", (int)node){
        UI_COLUMN {
            UI_ROW {
                present_editable_string(ui->theme.text, node);
                present_space();
                present_string(ui->theme.text_misc, make_string("::"));
                present_space();
                present_string(ui->theme.text_type, make_string("struct"));
                present_space();
                present_string(ui->theme.text_misc, make_string("{"));
            }
            UI_ROW {
                present_space();
                present_arc(node->first_child);
            }
            UI_ROW {
                present_string(ui->theme.text_misc, make_string("}"));
            }
        }
    }
}

internal void
present_declaration(Arc_Node* node){
    ID("declaration%d", (int)node){
        UI_ROW {
            present_editable_string(ui->theme.text, node);
            present_space();
            present_string(ui->theme.text_misc, make_string(":"));
            present_space();
            present_arc(node->first_child);
            if(node->first_child && node->first_child->next_sibling){
                present_space();
                present_string(ui->theme.text_misc, make_string("="));
                present_space();
                present_arc(node->first_child->next_sibling);
            }
        }
    }
}

internal void
present_function(Arc_Node* node){
    if(!node) return;
    if(arc_has_property(node, AP_AST)){
        
        switch(node->ast_type){
            case AST_FUNCTION: {
                
                ID("function%d", (int)node){
                    UI_COLUMN {
                        
                        auto params = node->first_child->first_child;
                        auto return_type = node->first_child->next_sibling->first_child;
                        auto scope = node->first_child->next_sibling->next_sibling->first_child;
                        UI_ROW{
                            present_editable_string(ui->theme.text_function, node);
                            present_space();
                            present_string(ui->theme.text_misc, make_string("::"));
                            present_space();
                            present_string(ui->theme.text_misc, make_string("("));
                            present_function(params);
                            present_string(ui->theme.text_misc, make_string(")"));
                            present_space();
                            present_string(ui->theme.text_misc, make_string("->"));
                            present_space();
                            present_function(return_type);
                            present_space();
                            present_string(ui->theme.text_misc, make_string("{"));
                        }
                        UI_ROW {
                            present_space();
                            present_space();
                            present_arc(scope);
                        }
                        UI_ROW {
                            present_string(ui->theme.text_misc, make_string("}"));
                        }
                    }
                }
            }break;
            case AST_DECLARATION: {
                auto args = node;
                for(auto decl = args; decl; decl = decl->next_sibling){
                    ID("param%d", (int)decl){
                        present_arc(decl);
                        if(decl->next_sibling){
                            present_string(ui->theme.text_misc, make_string(","));
                            present_space();
                        }
                    }
                }
            }break;
            default: {
                present_arc(node);
            }break;
            
        }
    }else {
        present_arc(node);
    }
    
}

internal void
present_type_usage(Arc_Node* node){
    present_editable_string(ui->theme.text_type, node);
}

internal void
present_ast(Arc_Node* node){
    if(!node) return;
    switch(node->ast_type){
        case AST_STRUCT: {
            present_struct(node);
        }break;
        case AST_DECLARATION: {
            present_declaration(node);
        }break;
        case AST_FUNCTION: {
            present_function(node);
        }break;
        case AST_TYPE_USAGE: {
            present_type_usage(node);
        }break;
        case AST_SCOPE: {
            auto member = node->first_child;
            UI_COLUMN{
                while(member){
                    present_arc(member);
                    member = member->next_sibling;
                }
            }
        }break;
        case AST_TOKEN: {
            if(node->token_type == TOKEN_MISC){
                present_editable_string(ui->theme.text_misc, node);
            }else if(node->token_type == TOKEN_REFERENCE){
                present_editable_reference(ui->theme.text_function, node);
            }else{
                present_editable_string(ui->theme.text_literal, node);
            }
            present_space();
            present_arc(node->next_sibling);
        }break;
    }
}

internal void
present_arc(Arc_Node* node){
    if(!node) return;
    if(arc_has_property(node, AP_AST)){
        present_ast(node);
    }else {
        present_editable_string(ui->theme.text, node);
        present_arc(node->next_sibling);
        present_arc(node->first_child);
    }
}

internal void
present_debug_arc(Arc_Node* node){
    
}

