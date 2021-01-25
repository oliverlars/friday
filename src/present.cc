
internal void
find_next_selectable(){
    cursor.arc = cursor.arc->parent;
    
    while(!cursor.arc->next_sibling){
        cursor.arc = cursor.arc->parent;
    }
    cursor.arc = cursor.arc->next_sibling;
    while(cursor.arc && cursor.arc->first_child){
        cursor.arc = cursor.arc->first_child;
    }
}

internal void
find_prev_selectable(){
    cursor.arc = cursor.arc->parent;
    
    while(!cursor.arc->next_sibling){
        cursor.arc = cursor.arc->parent;
    }
    cursor.arc = cursor.arc->next_sibling;
    while(cursor.arc && cursor.arc->first_child){
        cursor.arc = cursor.arc->first_child;
    }
}

internal void
advance_cursor(Cursor_Direction dir){
    if(!cursor.arc) return;
    int pos = 0;
    switch(dir){
        case CURSOR_UP:{
            
        }break;
        case CURSOR_DOWN:{
            
        }break;
        case CURSOR_LEFT:{
            if(cursor.arc->parent){
                cursor.arc = cursor.arc->parent;
            }else if(cursor.arc->prev_sibling){
                cursor.arc = cursor.arc->prev_sibling;
            }else {
                //find_prev_selectable();
            }
            pos = cursor.arc->string.length;
        }break;
        case CURSOR_RIGHT:{
            if(cursor.arc->first_child){
                cursor.arc = cursor.arc->first_child;
            }else if(cursor.arc->next_sibling){
                cursor.arc = cursor.arc->next_sibling;
            }else {
                find_next_selectable();
            }
        }break;
    }
    
    ui->cursor_pos = pos;
    
}

internal Present_Node*
get_present_node(String8 string, UI_ID id){
    
    auto hash = id & (MAX_TABLE_WIDGETS - 1);
    if(!presenter->last_table) return nullptr;
    auto node = presenter->last_table[hash];
    
    if(!node){
        return nullptr;
    }else {
        do {
            if(string_eq(string, node->string)){
                return node;
            }
            node = node->next_hash;
        }while(node);
    }
    
    return nullptr;
}

internal void
place_node_in_table(Present_Node* new_node){
    
    auto hash = new_node->id & (MAX_TABLE_WIDGETS - 1);
    auto node = presenter->table[hash];
    
    if(!node){
        presenter->table[hash] = new_node;
    }else {
        do {
            if(string_eq(new_node->string, node->string)){
                assert("this shouldn't happen...");
            }
            if(!node->next_hash){
                node->next_hash = new_node;
            }
            node = node->next_hash;
        }while(node);
    }
}

internal Present_Node*
push_present_node(String8 string, UI_ID id, Ast_Node* node){
    assert(presenter->line);
    auto next = push_type_zero(&platform->frame_arena, Present_Node);
    next->string = string;
    next->id = id;
    next->node = node;
    place_node_in_table(next);
    
    auto sibling = presenter->line;
    while(sibling->next_sibling){
        sibling = sibling->next_sibling;
    }
    
    sibling->next_sibling = next;
    next->prev_sibling = sibling;
    
    return next;
}

internal Present_Node*
push_present_line(){
    auto next = push_type_zero(&platform->frame_arena, Present_Node);
    if(!presenter->lines){
        presenter->lines = next;
        presenter->line = next;
    }else {
        auto node = presenter->lines;
        while(node->child){
            node = node->child;
        }
        node->child = next;
        next->parent = node;
        presenter->line = next;
    }
    return next;
}


internal void
present_cursor(){
    
    auto widget = push_widget(make_string("cursor"));
    u64 custom_hash = 0;
    hash32(&custom_hash, widget->string);
    widget->id = custom_hash;
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_LERP_POSITION);
    
    auto render_hook = [](Widget* widget){
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        push_rectangle(v4f2(pos, widget->min), 1, ui->theme.cursor);
    };
    
    widget->render_hook = render_hook;
    v2f size = get_text_size(widget->string);
    update_widget(widget);
    widget->min = v2f(2, size.height);
    
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
    if(presenter->mode == PRESENT_CREATE) return;
    clampi(&ui->cursor_pos, 0, ui->editing_string.length);
    auto last_widget = get_last_widget(widget->id, widget->string);
    
    if(has_pressed_key(KEY_ENTER)){
        presenter->mode = PRESENT_CREATE;
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
            remove_node_at(cursor.at->node);
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

internal void
present_editable_string(Colour colour, Ast_Node* node){
    
    auto string = &node->name;
    auto widget = push_widget(*string);
    push_present_node(*string, widget->id, node);
    
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_CLICKABLE);
    //widget_set_property(widget, WP_TEXT_EDIT);
    widget->style.text_colour = v4f_from_colour(colour);
    
    
    auto render_hook = [](Widget* widget ){
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        v4f bbox = v4f2(pos, widget->min);
        
        if(ui->active == widget->id){
            String8 s = make_stringf(&platform->frame_arena, "%.*s", ui->editing_string.length, ui->editing_string.text);
            
            push_string(bbox.pos, s, colour_from_v4f(widget->style.text_colour), widget->style.font_scale);
            v2f cursor = {};
            cursor.x = pos.x + get_text_width_n(s, ui->cursor_pos, widget->style.font_scale);
            cursor.y = bbox.y;
            push_rectangle(v4f2(cursor, v2f(2, widget->min.height)), 1, ui->theme.cursor);
        }else {
            v2f pos = widget->pos;
            pos.y -= widget->min.height;
            v4f bbox = v4f2(pos, widget->min);
            
            push_string(pos, widget->string, colour_from_v4f(widget->style.text_colour), widget->style.font_scale);
        }
    };
    
    widget->render_hook = render_hook;
    
    // NOTE(Oliver): custom text edit
    {
        auto prev_active = ui->active;
        if(widget->id == ui->active){
            
            cursor.at = get_present_node(widget->string, widget->id);
            cursor.string = string;
            
            edit_text(widget);
            
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
        v2f size = get_text_size(ui->editing_string.string, widget->style.font_scale);
        widget->min = size;
    }else {
        v2f size = get_text_size(widget->string, widget->style.font_scale);
        widget->min = size;
    }
    
    if(result.clicked){
        memcpy(ui->editing_string.text, string->text, string->length);
        ui->editing_string.length = string->length;
        ui->cursor_pos = string->length;
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

internal void
present_scope(Ast_Node* node, int present_style){
    auto statement = node->scope.statements;
    UI_COLUMN {
        statement = statement->next;
        for(; statement; statement = statement->next){
            UI_ROW{
                push_present_line();
                present_space();
                present_graph(statement, present_style);
            }
        }
    }
}

internal void
present_function(Ast_Node* node, int present_style){
    auto function = &node->function;
    auto parameters = function->parameters;
    auto name = node->name;
    b32 render_body = true;
    if(check_dropdown("%.*s#dropdown", name.length, name.text)){
        render_body = false;
    }
    UI_COLUMN {
        switch(present_style){
            case 0: {
                ID("%d", (int)node) {
                    
                    UI_ROW  {
                        present_editable_string(ui->theme.text_function, node);
                        present_string(ui->theme.text_misc, make_string("("));
                        parameters = parameters->next;
                        for(;parameters; parameters = parameters->next){
                            present_graph(parameters, present_style);
                            if(parameters->next){
                                ID("%d", (int)parameters) present_string(ui->theme.text_misc, make_string(","));
                            }
                        }
                        present_string(ui->theme.text_misc, make_string(")"));
                        present_space();
                        if(render_body)
                            present_string(ui->theme.text_misc, make_string("{"));
                        else {
                            present_string(ui->theme.text_misc, make_string("{"));
                            present_string(ui->theme.text_misc, make_string("..."));
                            present_string(ui->theme.text_misc, make_string("}"));
                        }
                    }
                    
                    if(render_body){
                        present_graph(function->scope, present_style);
                        present_string(ui->theme.text_misc, make_string("}"));
                    }
                    
                }
                
            }break;
            case 1: {
                
                ID("%d", (int)node) {
                    UI_ROW  {
                        present_editable_string(ui->theme.text_function, node);
                        present_space();
                        present_string(ui->theme.text_misc, make_string("::"));
                        present_space();
                        present_string(ui->theme.text_misc, make_string("("));
                        parameters = parameters->next;
                        for(;parameters; parameters = parameters->next){
                            present_graph(parameters, present_style);
                            if(parameters->next){
                                ID("%d", (int)parameters) present_string(ui->theme.text_misc, make_string(","));
                            }
                        }
                        present_string(ui->theme.text_misc, make_string(")"));
                        present_space();
                        present_string(ui->theme.text_misc, make_string("->"));
                        present_space();
                        present_graph(function->return_type, present_style);
                        if(render_body){
                            present_string(ui->theme.text_misc, make_string("{"));
                        }else {
                            present_string(ui->theme.text_misc, make_string("{"));
                            present_string(ui->theme.text_misc, make_string("..."));
                            present_string(ui->theme.text_misc, make_string("}"));
                        }
                    }
                    
                    if(render_body){
                        
                        present_graph(function->scope, present_style);
                        present_string(ui->theme.text_misc, make_string("}"));
                    }
                    
                }
                
            }break;
            case 2: {
                
                ID("%d", (int)node) {
                    UI_ROW  {
                        present_string(ui->theme.text_type, make_string("def"));
                        present_space();
                        present_editable_string(ui->theme.text_function, node);
                        present_space();
                        present_string(ui->theme.text_misc, make_string("("));
                        parameters = parameters->next;
                        for(;parameters; parameters = parameters->next){
                            present_graph(parameters, present_style);
                            if(parameters->next){
                                ID("%d", (int)parameters) {
                                    present_string(ui->theme.text_misc, make_string(","));
                                    present_space();
                                }
                            }
                        }
                        present_string(ui->theme.text_misc, make_string(")"));
                        present_string(ui->theme.text_misc, make_string(":"));
                        present_graph(function->return_type, present_style);
                    }
                    if(render_body){
                        present_graph(function->scope, present_style);
                    }
                    
                }
                
            }break;
            case 3: {
                
                ID("%d", (int)node) {
                    UI_ROW  {
                        present_string(ui->theme.text_type, make_string("procedure"));
                        present_space();
                        present_editable_string(ui->theme.text_function, node);
                        present_space();
                        present_string(ui->theme.text_misc, make_string("("));
                        for(;parameters; parameters = parameters->next){
                            present_graph(parameters, present_style);
                            if(parameters->next){
                                ID("%d", (int)parameters) present_string(ui->theme.text_misc, make_string(","));
                            }
                        }
                        present_string(ui->theme.text_misc, make_string(")"));
                        present_string(ui->theme.text_misc, make_string(":"));
                        present_graph(function->return_type, present_style);
                    }
                    present_string(ui->theme.text_type, make_string("begin"));
                    
                    if(render_body){
                        present_graph(function->scope, present_style);
                    }
                    present_string(ui->theme.text_type, make_string("end"));
                }
                
            }break;
        }
    }
}

internal void
present_declaration(Ast_Node* node, int present_style){
    auto decl = &node->declaration;
    switch(present_style){
        case 0:{
            present_graph(decl->type_usage, present_style);
            present_editable_string(ui->theme.text, node);
            if(decl->is_initialised){
                present_space();
                present_string(ui->theme.text_misc, make_string("="));
                present_graph(decl->expression, present_style);
            }
            
        }break;
        case 1:{
            
            present_editable_string(ui->theme.text, node);
            present_string(ui->theme.text_misc, make_string(":"));
            present_space();
            present_graph(decl->type_usage, present_style);
            if(decl->is_initialised){
                present_space();
                present_string(ui->theme.text_misc, make_string("="));
                present_graph(decl->expression, present_style);
            }
            
        }break;
        case 2:{
            
            present_editable_string(ui->theme.text, node);
            present_string(ui->theme.text_misc, make_string(":"));
            present_space();
            present_graph(decl->type_usage, present_style);
            if(decl->is_initialised){
                present_space();
                present_string(ui->theme.text_misc, make_string("="));
                present_graph(decl->expression, present_style);
            }
            
        }break;
        case 3:{
            present_editable_string(ui->theme.text, node);
            present_string(ui->theme.text_misc, make_string(":"));
            present_space();
            present_graph(decl->type_usage, present_style);
            if(decl->is_initialised){
                present_space();
                present_string(ui->theme.text_misc, make_string("="));
                present_graph(decl->expression, present_style);
            }
            
        }break;
    }
}

internal void
present_type_usage(Ast_Node* node){
    auto tu = &node->type_usage;
    UI_ROW{
        present_editable_string(ui->theme.text_type, tu->type_reference);
    }
}

internal void
present_graph(Ast_Node* node, int present_style){
    
    if(!node) return;
    
    switch(node->type){
        case AST_INVALID:{
        }break;
        case AST_BINARY:{
        }break;
        case AST_UNARY:{
        }break;
        case AST_LITERAL:{
        }break;
        case AST_STRUCT:{
        }break;
        case AST_ENUM:{
        }break;
        case AST_UNION:{
        }break;
        case AST_SCOPE:{
            present_scope(node, present_style);
        }break;
        case AST_TYPE_USAGE:{
            present_type_usage(node);
        }break;
        case AST_DECLARATION:{
            ID("%d", (int)node) UI_ROW present_declaration(node, present_style);
        }break;
        case AST_IDENTIFIER:{
        }break;
        case AST_FUNCTION:{
            present_function(node, present_style);
        }break;
        case AST_CONDITIONAL:{
        }break;
        case AST_LOOP:{
        }break;
        case AST_CALL:{
        }break;
        case AST_TOKEN:{
        }break;
    }
    
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
    if(presenter->mode == PRESENT_CREATE) return;
    clampi(&ui->cursor_pos, 0, node->string.length);
    auto string = &node->string;
    
    if(presenter->mode != PRESENT_EDIT_TYPE && has_pressed_key(KEY_ENTER)){
        presenter->mode = PRESENT_CREATE;
    }
    
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
            arc_remove_property(cursor.arc, AP_AST);
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
            platform_consume_event(event);
        }
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
    
    
    auto render_hook = [](Widget* widget ){
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        v4f bbox = v4f2(pos, widget->min);
        
        push_string(pos, widget->alt_string, colour_from_v4f(widget->style.text_colour), widget->style.font_scale);
        
        if(ui->active == widget->id){
            v2f next = {};
            next.x = pos.x + get_text_width_n(widget->alt_string, ui->cursor_pos, widget->style.font_scale);
            next.y = bbox.y;
            lerp(&cursor.pos.x, next.x, 0.4f);
            lerp(&cursor.pos.y, next.y, 0.4f);
            push_rectangle(v4f2(cursor.pos, v2f(2, widget->min.height)), 1, ui->theme.cursor);
        }
    };
    
    widget->render_hook = render_hook;
    
    // NOTE(Oliver): custom text edit
    {
        if(cursor.arc == node){
            ui->active = widget->id;
            edit_text(cursor.arc);
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
                    UI_ROW {
                        auto params = node->first_child->first_child;
                        auto return_type = node->first_child->next_sibling->first_child;
                        auto scope = node->first_child->next_sibling->next_sibling;
                        present_editable_string(ui->theme.text, node);
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
                        present_arc(scope);
                    }
                    present_string(ui->theme.text_misc, make_string("}"));
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

