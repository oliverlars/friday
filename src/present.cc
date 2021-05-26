
internal void
advance_cursor(Cursor_Direction dir){
    if(!cursor.at) return;
    presenter->direction = dir;
}

internal b32
can_advance_cursor(Cursor_Direction dir){
    for(int i = presenter->buffer_index+1; i < presenter->buffer_pos; i++){
        if(!presenter->buffer[i].newline){
            return true;
        }
    }
    return false;
}

internal void
set_next_cursor_pos(){
    int pos;
    for(pos = 0; pos < presenter->buffer_pos; pos++){
        if(presenter->buffer[pos].node &&
           presenter->buffer[pos].node == cursor.at){
            break;
        }
    }
    int next_pos = pos;
    switch(presenter->direction){
        case CURSOR_LEFT:{
            next_pos = clampi(next_pos-1, 0, presenter->buffer_pos);
            if(presenter->buffer[next_pos].newline){
                next_pos = clampi(next_pos-1, 0, presenter->buffer_pos);
            }
            cursor.at = presenter->buffer[next_pos].node;
            ui->cursor_pos = cursor.at->string.length;
        }break;
        case CURSOR_RIGHT:{
            next_pos = clampi(next_pos+1, 0, presenter->buffer_pos-1);
            if(presenter->buffer[next_pos].newline){
                if(presenter->buffer_pos-1 == next_pos){
                    next_pos = pos;
                }else{
                    next_pos = clampi(next_pos+1, 0, presenter->buffer_pos-1);
                }
            }
            cursor.at = presenter->buffer[next_pos].node;
            ui->cursor_pos = 0;
        }break;
        case CURSOR_UP:{
            int distance_from_newline = pos;
            while(!presenter->buffer[distance_from_newline++].newline);
            distance_from_newline -= pos;
            while(!presenter->buffer[next_pos--].newline);
            cursor.at = presenter->buffer[next_pos].node;
        }break;
        case CURSOR_DOWN:{
            while(!presenter->buffer[next_pos++].newline);
            cursor.at = presenter->buffer[pos].node;
        }break;
    }
    presenter->buffer_index = next_pos;
}

internal void
present_string(Colour colour, String8 string){
    if(string.length == 0){
        string = make_string(" ");
    }
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
push_arc(Arc_Node* node) {
    Present_Node present_node = {};
    present_node.node = node;
    presenter->buffer[presenter->buffer_pos++] = present_node;
    return;
}

internal void
push_newline(){
    Present_Node present_node = {};
    present_node.newline = true;
    presenter->buffer[presenter->buffer_pos++] = present_node;
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
    String8 string = make_stringf(&platform->frame_arena, "%.*s", node->reference->string.length,
                                  node->reference->string.text);
    
    auto widget_string = make_stringf(&platform->frame_arena, "edit_string%d", (int)node);
    auto widget = push_widget(widget_string);
    
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_ALT_STRING);
    widget_set_property(widget, WP_FIRST_TRANSITION);
    widget->alt_string = string;
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
        
        if(!widget->alt_string.length && cursor.text_id != widget->id){
            push_rectangle(v4f2(pos - v2f(0, 5), v2f(10, 3)), 1, colour_from_v4f(v4f(1,0,0,0)));
        }
        
        push_string(pos, widget->alt_string, colour_from_v4f(widget->style.text_colour), widget->style.font_scale);
        
        v4f underline = v4f(bbox.x, bbox.y, bbox.width, 3);
        if(highlight_reference && (widget->arc->reference == highlight_reference || widget->arc == highlight_reference)){
            push_rectangle(underline, 1, colour_from_v4f(v4f(1,0,0,1)));
        }
        
        if(cursor.text_id == widget->id){
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
            cursor.text_id = widget->id;
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
    
    if(cursor.text_id == widget->id){
        v2f size = get_text_size(widget->alt_string, widget->style.font_scale);
        widget->min = size;
    }else {
        v2f size = get_text_size(widget->alt_string, widget->style.font_scale);
        widget->min = size;
    }
    
    if(result.clicked){
        ui->cursor_pos = string.length;
        cursor.at = widget->arc;
        cursor.text_id = widget->id;
    }
    
    if(result.hovered && node->reference){
        highlight_reference = node->reference;
    }
    
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
        
        if(!widget->alt_string.length && cursor.text_id != widget->id){
            //push_rectangle(v4f2(pos - v2f(0, 5), v2f(10, 3)), 1, colour_from_v4f(v4f(1,0,0,0)));
            push_circle(pos + v2f(0, 5), 3, ui->theme.border);
            //push_string(pos, make_string("->"), ui->theme.border, widget->style.font_scale);
        }
        
        push_string(pos, widget->alt_string, colour_from_v4f(widget->style.text_colour), widget->style.font_scale);
        
        v4f underline = v4f(bbox.x, bbox.y, bbox.width, 3);
        if(highlight_reference && (widget->arc->reference == highlight_reference || widget->arc == highlight_reference)){
            push_rectangle(underline, 1, colour_from_v4f(v4f(1,0,0,1)));
        }
        
        if(cursor.text_id == widget->id){
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
            cursor.text_id = widget->id;
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
    
    if(cursor.text_id == widget->id){
        v2f size = get_text_size(widget->alt_string, widget->style.font_scale);
        widget->min = size;
    }else {
        v2f size = get_text_size(widget->alt_string, widget->style.font_scale);
        widget->min = size;
    }
    
    if(result.clicked){
        ui->cursor_pos = string->length;
        cursor.at = widget->arc;
        cursor.text_id = widget->id;
    }
    
    if(result.hovered && node->reference){
        highlight_reference = node->reference;
    }
    
}


internal void present_arc(Arc_Node* node);

internal void
present_declaration(Arc_Node* node){
    ID("declaration%d", (int)node){
        UI_ROW {
            push_arc(node);
            present_editable_string(ui->theme.text, node);
            present_space();
            present_string(ui->theme.text_misc, make_string(":"));
            present_space();
            present_arc(node->first_child->first_child);
            //push_arc(node->first_child->first_child);
            if(node->last_child->first_child){
                present_space();
                present_string(ui->theme.text_misc, make_string("="));
                present_space();
                //push_arc(node->last_child->first_child);
                present_arc(node->last_child->first_child);
                push_newline();
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
                        auto scope = node->first_child->next_sibling->next_sibling;
                        UI_ROW{
                            push_arc(node);
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
                auto params = node;
                for(auto decl = params; decl; decl = decl->next_sibling){
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
    push_arc(node);
    present_editable_string(ui->theme.text_type, node);
}

internal void
present_if(Arc_Node* node){
    ID("if%d", (int)node){
        
        UI_COLUMN{
            UI_ROW{
                present_editable_string(ui->theme.text, node);
                push_arc(node);
                present_space();
                present_arc(node->first_child->first_child);
                push_arc(node->first_child->first_child);
                present_space();
                present_string(ui->theme.text_misc, make_string("{"));
                push_newline();
            }
            UI_ROW {
                present_space();
                present_space();
                present_arc(node->last_child);
                push_newline();
            }
            UI_ROW {
                present_string(ui->theme.text_misc, make_string("}"));
            }
        }
    }
}

internal void
present_struct(Arc_Node* node){
    ID("struct%d", (int)node){
        
        UI_COLUMN{
            UI_ROW{
                present_editable_string(ui->theme.text_type, node);
                present_space();
                present_string(ui->theme.text, make_string("::"));
                present_space();
                present_string(ui->theme.text, make_string("struct"));
                push_arc(node);
                present_space();
                present_string(ui->theme.text_misc, make_string("{"));
                push_newline();
            }
            UI_ROW {
                present_space();
                present_space();
                present_arc(node->first_child);
                push_newline();
            }
            UI_ROW {
                present_string(ui->theme.text_misc, make_string("}"));
            }
        }
    }
}

internal void
present_call(Arc_Node* node){
    ID("call%d", (int)node){
        UI_ROW{
            present_editable_string(ui->theme.text_type, node);
            push_arc(node);
            present_string(ui->theme.text_misc, make_string("("));
            
            auto arg = node->first_child;
            assert(node->reference);
            auto param = node->reference->first_child->first_child;
            
            for(auto expr = arg; expr; expr = expr->next_sibling){
                ID("args%d", (int)expr){
                    if(param){
                        present_string(ui->theme.text_misc, param->string);
                        present_string(ui->theme.text_misc, make_string(": "));
                    }
                    present_arc(expr->first_child);
                    if(expr->next_sibling){
                        present_string(ui->theme.text_misc, make_string(","));
                        present_space();
                    }
                }
                if(param){
                    param = param->next_sibling;
                }
            }
            present_string(ui->theme.text_misc, make_string(")"));
            push_newline();
        }
    }
}

internal void
present_ast(Arc_Node* node){
    if(!node) return;
    switch(node->ast_type){
        case AST_DECLARATION: {
            present_declaration(node);
        }break;
        case AST_STRUCT: {
            present_struct(node);
        }break;
        case AST_FUNCTION: {
            present_function(node);
        }break;
        case AST_TYPE_USAGE: {
            present_type_usage(node);
        }break;
        case AST_IF: {
            present_if(node);
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
        case AST_EXPR:{
            present_ast(node->first_child);
        }break;
        case AST_TOKEN: {
            if(node->token_type == TOKEN_MISC){
                present_editable_string(ui->theme.text_misc, node);
            }else if(node->token_type == TOKEN_REFERENCE){
                present_editable_reference(ui->theme.text_function, node);
            }else if(node->token_type == TOKEN_LITERAL){
                present_editable_string(ui->theme.text_literal, node);
            }else {
                present_editable_string(ui->theme.text, node);
            }
            if(node->next_sibling){
                present_space();
            }
            push_arc(node);
            present_arc(node->next_sibling);
        }break;
        case AST_CALL:{
            present_call(node);
        }break;
    }
}

internal void
present_arc(Arc_Node* node){
    if(!node) return;
    if(arc_has_property(node, AP_AST)){
        present_ast(node);
    }else {
        push_arc(node);
        present_editable_string(ui->theme.text, node);
        present_arc(node->next_sibling);
        present_arc(node->first_child);
    }
}

internal f32
present_debug_arc(v2f pos, Arc_Node* node){
    if(!node) return 0;
    f32 start_x = pos.x;
    while(node) {
        f32 offset = 0;
        if(node->string.length){
            push_string(pos + v2f(25, 0), node->string, ui->theme.text, 0.5f);
            offset = get_text_width(node->string, .5f) + 20;
        }
        if(node == cursor.at){
            push_circle(pos, 20, ui->theme.cursor);
        }else {
            push_circle(pos, 20, ui->theme.text);
        }
        if(node->first_child)
            pos.x += present_debug_arc(pos + v2f(-10, -30), node->first_child);
        node = node->next_sibling;
        pos.x +=  offset;
    }
    return pos.x - start_x;
}


