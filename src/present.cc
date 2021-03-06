
internal void
mark_node_for_deletion(Arc_Node* at){
    if(arc_has_property(at, AP_DELETABLE)){
        arc_set_property(at, AP_MARK_DELETE);
    }
}

internal void
advance_cursor(Cursor* cursor, Cursor_Direction dir, int count = 1){
    if(!cursor->at) return;
    cursor->direction = dir;
    cursor->direction_count = count;
}

internal void
set_cursor_as_node(Cursor* cursor, Arc_Node* node){
    ui->cursor_pos = 0;
    int index = 0;
    for(; index < presenter->buffer_pos; index++){
        if(presenter->buffer[index].node == node){
            break;
        }
    }
    // NOTE(Oliver): use navigation system to find where to click to
    // as it needs to update presenter->buffer_index and presenter->line_index
    int pos_diff = index - cursor->buffer_index;
    if(pos_diff < 0){
        advance_cursor(cursor, CURSOR_LEFT, abs(pos_diff));
    }else if(pos_diff > 0){
        advance_cursor(cursor, CURSOR_RIGHT, pos_diff);
    }
    
}

internal b32
is_after_cursor_of_ast_type(Ast_Type type){
    int current_index = presenter->buffer_index;
    if(presenter->buffer[current_index+1].node){
        if(arc_has_property(presenter->buffer[current_index+1].node, AP_AST),
           presenter->buffer[current_index+1].node->ast_type == type){
            return true;
        }
    }
    return false;
}

internal int
is_node_before_or_after_cursor(Arc_Node* node){
    if(!node) return 0;
    if(node == presenter->cursor.at) return 0;
    for(int i = 0; i < presenter->buffer_index; i++){
        if(presenter->buffer[i].node == node){
            return -1;
        }
    }
    for(int i = presenter->buffer_index +1; i < presenter->buffer_pos; i++){
        if(presenter->buffer[i].node == node){
            return 1;
        }
    }
    return 0;
}

internal void
delete_sub_tree_marked_for_deletion(Arc_Node* node){
    
    while(node){
        if(node->first_child){
            delete_sub_tree_marked_for_deletion(node->first_child);
        }
        
        int before_or_after_cursor = is_node_before_or_after_cursor(node);
        if(before_or_after_cursor > 0){
            presenter->number_of_deletions_before_cursor++;
        }else {
            presenter->number_of_deletions_after_cursor++;
        }
        
        presenter->number_of_deletions++;
        pool_clear(&editor->arc_pool, node);
        node = node->next_sibling;
    }
}

internal void
delete_nodes_marked_for_deletion(Arc_Node* node){
    while(node){
        if(arc_has_property(node, AP_MARK_DELETE)){
            if(node->prev_sibling){
                remove_arc_node_at(&node->parent->first_child, node);
                
                int before_or_after_cursor = is_node_before_or_after_cursor(node);
                if(before_or_after_cursor >= 0){
                    presenter->number_of_deletions_before_cursor++;
                }else if(before_or_after_cursor < 0) {
                    presenter->number_of_deletions_after_cursor++;
                }
                
            }else{
                arc_clear_all_properties(node);
                arc_set_property(node, AP_SELECTABLE);
                node->ast_type = AST_INVALID;
            }
            if(node->first_child){
                delete_sub_tree_marked_for_deletion(node->first_child);
                node->first_child = nullptr;
                node->last_child = nullptr;
            }
        }else {
            if(node->first_child){
                delete_nodes_marked_for_deletion(node->first_child);
            }
        }
        node = node->next_sibling;
    }
}

internal b32
can_advance_cursor(Cursor* cursor,Cursor_Direction dir){
    int pos_index = cursor->buffer_index;
    int line_index = cursor->line_index;
    int count = cursor->direction_count;
    
    switch(dir){
        case CURSOR_RIGHT:{
            return (pos_index+count) < presenter->buffer_pos;
        }break;
        case CURSOR_LEFT:{
            return pos_index - count >= 0;
        }break;
        case CURSOR_UP:{
            return line_index - count >= 0;
        }break;
        case CURSOR_DOWN:{
            return line_index+count < presenter->line_pos;
        }break;
    }
    return false;
}

internal void
jump_to_declaration(){
    // TODO(Oliver): use new cursor api to find the reference location
    if(presenter->cursor.at->reference){
        presenter->cursor.at = presenter->cursor.at->reference;
    }
}

internal void
set_matching_reference_in_composite(Arc_Node* node, b32* found){
    if(!found) return;
    if(*found) return;
    if(!node) return;
    auto parent = node->parent;
    Arc_Node* member = nullptr;
    if(declaration_type_is_composite(node)){
        auto decl = node;
        auto type = decl->first_child->first_child;
        auto _struct = type->reference;
        member = _struct->first_child->first_child;
    }
    while(member && !*found){
        if(arc_has_property(member, AP_AST) && member->ast_type == AST_USING){
            set_matching_reference_in_composite(member->first_child, found);
            if(*found) return;
        }else if(string_eq(presenter->cursor.at->string, member->string)){
            presenter->cursor.at->token_type = TOKEN_REFERENCE;
            presenter->cursor.at->reference = member;
            replace_string(&presenter->cursor.at->string, member->string);
            *found = true;
            return;
        }
        member = member->next_sibling;
    }
    return;
}

internal void
set_matching_reference_in_composite(Arc_Node* at, Arc_Node* node, b32* found){
    if(!found) return;
    if(*found) return;
    if(!node) return;
    auto parent = node->parent;
    Arc_Node* member = nullptr;
    if(declaration_type_is_composite(node)){
        auto decl = node;
        auto type = decl->first_child->first_child;
        auto _struct = type->reference;
        member = _struct->first_child->first_child;
    }
    while(member && !*found){
        if(arc_has_property(member, AP_AST) && member->ast_type == AST_USING){
            set_matching_reference_in_composite(member->first_child, found);
            if(*found) return;
        }else if(string_eq(at->string, member->string)){
            at->token_type = TOKEN_REFERENCE;
            at->reference = member;
            replace_string(&at->string, member->string);
            *found = true;
            return;
        }
        member = member->next_sibling;
    }
    return;
}

internal void
set_token_type(Arc_Node* node){
    Arc_Node* result;
    if(node->token_type == TOKEN_ARRAY ||
       node->token_type == TOKEN_STRING) return;
    
    Arc_Node* prev_ref = nullptr;
    b32 found = false;
    if(find_previous_reference(node->prev_sibling, &prev_ref)){
        set_matching_reference_in_composite(node, prev_ref, &found);
        if(found) return;
    }
    
    auto function = editor->stdlib;
    while(function){
        if(string_eq(node->string, function->string)){
            node->token_type = TOKEN_REFERENCE;
            node->reference = function;
            replace_string(&node->string, node->reference->string);
            return;
        }
        function = function->next_sibling;
    }
    
    auto scope = node;
    if(node->parent){
        // NOTE(Oliver): must be a dot operator
        auto parent = node->parent;
        Arc_Node* member = nullptr;
        if(parent->reference){
            b32 found = false;
            set_matching_reference_in_composite(parent->reference, &found);
            if(found) return;
        }
    }
    
    if(node->prev_sibling){
        Arc_Node* member = nullptr;
        if(node->prev_sibling->reference){
            b32 found = false;
            set_matching_reference_in_composite(node->prev_sibling->reference, &found);
            if(found) return;
        }
    }
    
    Arc_Node* _for;
    if(is_sub_node_of_ast_type(node, AST_FOR, &_for)){
        auto init = _for->first_child->first_child;
        if(!is_child_of_node(node, init)){
            if(string_eq(node->string, init->string)){
                node->token_type = TOKEN_REFERENCE;
                node->reference = init;
                replace_string(&node->string, init->string);
                return;
            }
        }
    }
    
    while(scope){
        Arc_Node* function;
        if(is_sub_node_of_ast_type(scope, AST_FUNCTION, &function)){
            auto param = function->first_child->first_child;
            while(param){
                if(string_eq(param->string, node->string)){
                    node->token_type = TOKEN_REFERENCE;
                    node->reference = param;
                    replace_string(&node->string, node->reference->string);
                    return;
                }
                param = param->next_sibling;
            }
        }
        if(scope->parent && arc_has_property(scope->parent, AP_AST)){
            if(scope->parent->ast_type == AST_SCOPE){
                result = scope;
                auto member = result->prev_sibling;
                while(member){
                    if(member->ast_type != AST_ASSIGNMENT && string_eq(member->string, node->string)){
                        node->token_type = TOKEN_REFERENCE;
                        node->reference = member;
                        replace_string(&node->string, node->reference->string);
                        return;
                    }
                    member = member->prev_sibling;
                }
                if(string_eq(node->string, "+") ||
                   string_eq(node->string, "-") ||
                   string_eq(node->string, "/") ||
                   string_eq(node->string, "*") ||
                   string_eq(node->string, "(") ||
                   string_eq(node->string, ")") ||
                   string_eq(node->string, "<") ||
                   string_eq(node->string, ">") ||
                   string_eq(node->string, ">=") ||
                   string_eq(node->string, "<=") ||
                   string_eq(node->string, "!=") ||
                   string_eq(node->string, "<<") ||
                   string_eq(node->string, ">>")){
                    node->token_type = TOKEN_MISC;
                }else{
                    char* str = string_to_cstr(&platform->frame_arena, node->string);
                    int f = atof(str);
                    int i = atoi(str);
                    if(i || f){
                        node->token_type = TOKEN_LITERAL;
                    }else {
                        node->token_type = TOKEN_UNASSIGNED;
                    }
                }
            }
        }
        
        scope = scope->parent;
    }
    
}

internal b32
can_resolve_reference(Arc_Node* node, Arc_Node**  would_be_reference = nullptr){
    Arc_Node* result;
    auto scope = node;
    
    auto func = editor->stdlib;
    while(func){
        if(string_eq(node->string, func->string)){
            if(would_be_reference) *would_be_reference = func;
            return true;
        }
        func = func->next_sibling;
    }
    
    if(node->parent){
        // NOTE(Oliver): must be a dot operator
        auto parent = node->parent;
        Arc_Node* member = nullptr;
        if(parent->reference){
            if(declaration_type_is_composite(parent->reference)){
                auto decl = parent->reference;
                auto type = decl->first_child->first_child;
                auto _struct = type->reference;
                member = _struct->first_child->first_child;
            }
            while(member){
                if(string_eq(node->string, member->string)){
                    if(would_be_reference) *would_be_reference = member;
                    return true;
                }
                member = member->next_sibling;
            }
        }
    }
    
    Arc_Node* function;
    if(is_sub_node_of_ast_type(node, AST_FOR, &function)){
        auto init = function->first_child->first_child;
        if(!is_child_of_node(node, init)){
            if(string_eq(node->string, init->string)){
                if(would_be_reference) *would_be_reference = init;
                return true;
            }
        }
    }
    
    while(scope){
        Arc_Node* function;
        if(is_sub_node_of_ast_type(scope, AST_FUNCTION, &function)){
            auto param = function->first_child->first_child;
            if(!is_child_of_node(node, function->first_child)){
                while(param){
                    if(string_eq(param->string, node->string)){
                        if(would_be_reference) *would_be_reference = param;
                        return true;
                    }
                    param = param->next_sibling;
                }
            }
        }
        if(scope->parent && arc_has_property(scope->parent, AP_AST)){
            if(scope->parent->ast_type == AST_SCOPE){
                result = scope;
                auto member = result->prev_sibling;
                while(member){
                    if(string_eq(member->string, node->string)){
                        if(would_be_reference) *would_be_reference = member;
                        return true;
                    }
                    member = member->prev_sibling;
                }
            }
        }
        scope = scope->parent;
    }
    return false;
}

internal void
set_type_token_type(Arc_Node* node){
    Arc_Node* result;
    if(node->token_type == TOKEN_ARRAY) return;
    if(node->token_type == TOKEN_REFERENCE &&
       node->string.length == 0){
        node->token_type = TOKEN_UNASSIGNED;
    }
    auto builtin = editor->builtins;
    while(builtin){
        if(builtin->ast_type == AST_TYPE_TOKEN && string_eq(node->string, builtin->string)){
            node->token_type = TOKEN_REFERENCE;
            node->reference = builtin;
            replace_string(&node->string, node->reference->string);
            return;
        }
        builtin = builtin->next_sibling;
    }
    
    Arc_Node* _struct;
    if(is_sub_node_of_ast_type(node, AST_STRUCT, &_struct)){
        if(string_eq(node->string, _struct->string)){
            node->token_type = TOKEN_REFERENCE;
            node->reference = _struct;
            replace_string(&node->string, node->reference->string);
            return;
        }
    }
    
    auto scope = node;
    
    while(scope){
        Arc_Node* function;
        
        if(scope->parent && arc_has_property(scope->parent, AP_AST)){
            if(scope->parent->ast_type == AST_SCOPE){
                result = scope;
                auto member = result->prev_sibling;
                while(member){
                    if(string_eq(member->string, node->string)){
                        node->token_type = TOKEN_REFERENCE;
                        node->reference = member;
                        replace_string(&node->string, node->reference->string);
                        return;
                    }
                    member = member->prev_sibling;
                }
                if(string_eq(node->string, "*")){
                    node->token_type = TOKEN_MISC;
                }else{
                    node->token_type = TOKEN_LITERAL;
                }
            }
        }
        scope = scope->parent;
    }
    
}

internal void
find_function(Arc_Node* node){
    Arc_Node* result;
    auto scope = node;
    while(scope){
        if(scope->parent && arc_has_property(scope->parent, AP_AST)){
            if(scope->parent->ast_type == AST_SCOPE){
                result = scope;
                auto member = result->prev_sibling;
                while(member){
                    if(string_eq(member->string, node->string)){
                        node->reference = member;
                        replace_string(&node->string, node->reference->string);
                        return;
                    }
                    member = member->prev_sibling;
                }
            }
        }
        scope = scope->parent;
    }
    
}

internal String8
find_matching_reference_in_composite(Arc_Node* node, b32* found){
    if(!found) return {};
    if(*found) return {};
    if(!node) return {};
    auto parent = node->parent;
    Arc_Node* member = nullptr;
    if(declaration_type_is_composite(node)){
        auto decl = node;
        auto type = decl->first_child->first_child;
        auto _struct = type->reference;
        member = _struct->first_child->first_child;
    }
    while(member && !*found){
        if(arc_has_property(member, AP_AST) && member->ast_type == AST_USING){
            auto result = find_matching_reference_in_composite(member->first_child, found);
            if(*found) return result;
        }else if(is_strict_substring(presenter->cursor.at->string, member->string)){
            
            if(has_pressed_key(KEY_TAB)){
                presenter->cursor.at->token_type = TOKEN_REFERENCE;
                presenter->cursor.at->reference = member;
                replace_string(&presenter->cursor.at->string, presenter->cursor.at->reference->string);
                ui->cursor_pos = presenter->cursor.at->string.length;
                *found = true;
                return {};
            }else{
                *found = true;
                return make_stringf(&platform->frame_arena, "%.*s", member->string.length,
                                    member->string.text+presenter->cursor.at->string.length);
            }
            
        }
        member = member->next_sibling;
    }
    return {};
}

internal String8
tab_completer(Arc_Node* node){
    Arc_Node* result = nullptr;
    if(node->reference) return {};
    b32 found = false;
    if(node->parent->ast_type == AST_TOKEN){
        auto string = find_matching_reference_in_composite(node->parent->reference, &found);
        if(found) return string;
    }
    
    auto func = editor->stdlib;
    while(func){
        if(is_strict_substring(node->string, func->string)){
            if(has_pressed_key(KEY_TAB)){
                node->token_type = TOKEN_REFERENCE;
                node->reference = func;
                replace_string(&node->string, node->reference->string);
                ui->cursor_pos = presenter->cursor.at->string.length;
                return {};
            }else{
                return make_stringf(&platform->frame_arena, "%.*s", func->string.length,
                                    func->string.text+node->string.length);
            }
        }
        func = func->next_sibling;
    }
    
    Arc_Node* prev_ref = nullptr;
    if(find_previous_reference(node->prev_sibling, &prev_ref)){
        auto string = find_matching_reference_in_composite(prev_ref->reference, &found);
        if(found) return string;
    }
    
    Arc_Node* function;
    if(is_sub_node_of_ast_type(node, AST_FOR, &function)){
        auto init = function->first_child->first_child;
        assert(init);
        if(is_strict_substring(node->string, init->string)){
            if(has_pressed_key(KEY_TAB)){
                node->token_type = TOKEN_REFERENCE;
                node->reference = init;
                replace_string(&node->string, node->reference->string);
                ui->cursor_pos = presenter->cursor.at->string.length;
                return {};
            }else{
                return make_stringf(&platform->frame_arena, "%.*s", init->string.length,
                                    init->string.text+node->string.length);
            }
        }
    }
    
    auto scope = node;
    while(scope){
        Arc_Node* function;
        if(is_sub_node_of_ast_type(scope, AST_FUNCTION, &function)){
            auto param = function->first_child->first_child;
            while(param){
                if(is_strict_substring(node->string, param->string)){
                    if(has_pressed_key(KEY_TAB)){
                        node->token_type = TOKEN_REFERENCE;
                        node->reference = param;
                        replace_string(&node->string, node->reference->string);
                        ui->cursor_pos = presenter->cursor.at->string.length;
                        return {};
                    }else{
                        return make_stringf(&platform->frame_arena, "%.*s", param->string.length-node->string.length,
                                            param->string.text+node->string.length);
                    }
                    
                }
                param = param->next_sibling;
            }
        }
        if(scope->parent && arc_has_property(scope->parent, AP_AST)){
            if(scope->parent->ast_type == AST_SCOPE){
                result = scope;
                auto member = result->prev_sibling;
                while(member){
                    if(is_strict_substring(node->string, member->string)){
                        if(has_pressed_key(KEY_TAB)){
                            node->token_type = TOKEN_REFERENCE;
                            node->reference = member;
                            replace_string(&node->string, node->reference->string);
                            ui->cursor_pos = presenter->cursor.at->string.length;
                            return {};
                        }else{
                            return make_stringf(&platform->frame_arena, "%.*s", member->string.length-node->string.length,
                                                member->string.text+node->string.length);
                        }
                    }
                    member = member->prev_sibling;
                }
                
            }
        }
        
        scope = scope->parent;
        
    }
    return {};
}

internal String8
tab_completer_type(Arc_Node* node){
    Arc_Node* result;
    
    auto builtin = editor->builtins;
    while(builtin){
        
        if(is_strict_substring(node->string, builtin->string)){
            if(has_pressed_key(KEY_TAB)){
                node->token_type = TOKEN_REFERENCE;
                node->reference = builtin;
                replace_string(&node->string, node->reference->string);
                ui->cursor_pos = presenter->cursor.at->string.length;
                return {};
            }else{
                return make_stringf(&platform->frame_arena, "%.*s", builtin->string.length-node->string.length,
                                    builtin->string.text+node->string.length);
            }
        }
        
        builtin = builtin->next_sibling;
    }
    
    Arc_Node* _struct;
    if(is_sub_node_of_ast_type(node, AST_STRUCT, &_struct)){
        if(is_strict_substring(node->string, _struct->string)){
            if(has_pressed_key(KEY_TAB)){
                node->token_type = TOKEN_REFERENCE;
                node->reference = _struct;
                replace_string(&node->string, node->reference->string);
                ui->cursor_pos = presenter->cursor.at->string.length;
                return {};
            }else{
                return make_stringf(&platform->frame_arena, "%.*s", _struct->string.length-node->string.length,
                                    _struct->string.text+node->string.length);
            }
        }
        
    }
    
    auto scope = node;
    while(scope){
        if(scope->parent && arc_has_property(scope->parent, AP_AST)){
            if(scope->parent->ast_type == AST_SCOPE){
                result = scope;
                auto member = result->prev_sibling;
                while(member){
                    if(is_strict_substring(node->string, member->string) &&
                       arc_has_property(member, AP_AST) && member->ast_type == AST_STRUCT){
                        if(has_pressed_key(KEY_TAB)){
                            node->token_type = TOKEN_REFERENCE;
                            node->reference = member;
                            replace_string(&node->string, node->reference->string);
                            ui->cursor_pos = presenter->cursor.at->string.length;
                            return {};
                        }else{
                            return make_stringf(&platform->frame_arena, "%.*s", member->string.length-node->string.length,
                                                member->string.text+node->string.length);
                        }
                    }
                    member = member->prev_sibling;
                }
                
            }
        }
        
        scope = scope->parent;
        
    }
    return {};
}

internal void
set_next_cursor_pos(Cursor* cursor){
    if(!cursor->at) return;
    if(cursor->direction != CURSOR_NONE){
        presenter->find_next_text_id = true;
    }
    auto dir = cursor->direction;
    auto count = cursor->direction_count;
    auto offset = presenter->number_of_deletions_before_cursor;
    if(dir == CURSOR_LEFT){
        if(offset) offset--;
    }
    auto pos = cursor->buffer_index - offset;
    auto line_index = cursor->line_index;
    int next_pos = pos;
    cursor->at = presenter->buffer[next_pos].node;
    switch(dir){
        case CURSOR_LEFT:{
            next_pos = clampi(next_pos-count, 0, presenter->buffer_pos-1);
            cursor->at = presenter->buffer[next_pos].node;
            ui->cursor_pos = cursor->at->string.length;
            while(next_pos < presenter->lines[line_index].start){
                line_index = clampi(line_index-1, 0, presenter->line_pos-1);
            }
        }break;
        case CURSOR_RIGHT:{
            next_pos = clampi(next_pos+count, 0, presenter->buffer_pos-1);
            cursor->at = presenter->buffer[next_pos].node;
            ui->cursor_pos = 0;
            
            while(next_pos > presenter->lines[line_index].end){
                line_index = clampi(line_index+1, 0, presenter->line_pos-1);
            }
        }break;
        case CURSOR_UP:{
            int distance_from_start = next_pos - presenter->lines[line_index].start;
            line_index = clampi(line_index-1, 0, presenter->line_pos-1);
            auto line = presenter->lines[line_index];
            next_pos = clampi(line.start+distance_from_start, line.start, line.end);
            cursor->at = presenter->buffer[next_pos].node;
        }break;
        case CURSOR_DOWN:{
            
            int distance_from_start = next_pos - presenter->lines[line_index].start;
            line_index = clampi(line_index+1, 0, presenter->line_pos-1);
            auto line = presenter->lines[line_index];
            next_pos = clampi(line.start+distance_from_start, line.start, line.end);
            if(presenter->buffer[next_pos].node){
                cursor->at = presenter->buffer[next_pos].node;
            }else {
                next_pos = pos;
                line_index--;
            }
        }break;
    }
    //assert(cursor->at);
    cursor->buffer_index = next_pos;
    cursor->line_index = line_index;
}

internal inline void
render_cursor(Cursor* cursor, v2f size, Colour colour){
    push_bezier(cursor->v0, cursor->v1, cursor->v2, size.width, colour);
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
    
    
    Widget_Style style = {};
    
    style.text_colour = v4f_from_colour(colour),
    style.border_colour  = v4f_from_colour(ui->theme.text),
    style.font_scale = font_scale;
    widget->style = style;
    
    v2f size = get_text_size(widget->string, widget->style.font_scale);
    widget->min = size;
    
    
}

internal void
present_space() {
    f32 space = get_text_width(" ", font_scale);
    xspacer(space*0.5f);
}

internal void
present_indent() {
    xspacer(presenter->indent_level);
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
    Line_Info line_info = {};
    if(presenter->line_pos == 0){
        line_info.end = presenter->buffer_pos-1;
        line_info.start = 0;
    }else{
        line_info.start = presenter->lines[presenter->line_pos-1].end+1;
        line_info.end = presenter->buffer_pos-1;
        if(line_info.end <= line_info.start){
            line_info.end = line_info.start;
        }
    }
    presenter->lines[presenter->line_pos++] = line_info;
    
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
    if(editor->mode == E_CREATE) return;
    clampi(&ui->cursor_pos, 0, node->string.length);
    auto string = &node->string;
    
    
    if(has_pressed_key(KEY_UP)){
        advance_cursor(&presenter->cursor,CURSOR_UP);
    }
    if(has_pressed_key(KEY_DOWN)){
        advance_cursor(&presenter->cursor,CURSOR_DOWN);
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
            advance_cursor(&presenter->cursor,CURSOR_LEFT);
        }else {
            ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
        }
    }
    if(has_pressed_key(KEY_RIGHT)){
        if(ui->cursor_pos == string->length){
            advance_cursor(&presenter->cursor,CURSOR_RIGHT);
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
            Arc_Node* result;
            if(arc_has_property(presenter->cursor.at, AP_DELETABLE)){
                mark_node_for_deletion(presenter->cursor.at);
                if(is_sub_node_of_ast_type(presenter->cursor.at, AST_ASSIGNMENT, &result)){
                    // NOTE(Oliver): assignment nodes are weird, they need special casing
                    // to delete because the root node isn't the first editable node
                    if(result->first_child == presenter->cursor.at->parent){
                        mark_node_for_deletion(result);
                    }
                }
            }
            advance_cursor(&presenter->cursor, CURSOR_LEFT);
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
    
    auto widget_string = make_stringf(&platform->frame_arena, "editable_ref%d", (int)node);
    auto widget = push_widget(widget_string);
    
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_ALT_STRING);
    widget_set_property(widget, WP_FIRST_TRANSITION);
    widget->alt_string = string;
    widget->style.text_colour = v4f_from_colour(colour);
    
    widget->arc = node;
    
    if(presenter->select_start.at == node){
        presenter->start_pos = presenter->pos;
    }
    if(presenter->select_end.at == node){
        presenter->end_pos = presenter->pos;
    }
    
    widget->present_pos = presenter->pos;
    
    if(ui->hot == widget->id){
        highlight_reference = node->reference;
    }
    
    auto render_hook = [](Widget* widget ){
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        v4f bbox = v4f2(pos, widget->min);
        
        if(!widget->alt_string.length && ui->text_edit != widget->id){
            push_rectangle(v4f2(pos - v2f(0, 5), v2f(10, 3)), 1, colour_from_v4f(v4f(1,0,0,0)));
        }
        
        if(presenter->start_pos != presenter->end_pos &&
           presenter->start_pos == widget->present_pos){
            presenter->select_first_rect = bbox;
        }
        if(presenter->start_pos != presenter->end_pos &&
           presenter->end_pos == widget->present_pos){
            presenter->select_second_rect = bbox;
        }
        
        push_string(pos, widget->alt_string, colour_from_v4f(widget->style.text_colour), widget->style.font_scale);
        
        v4f underline = v4f(bbox.x, bbox.y, bbox.width, 3);
        if(highlight_reference && widget->arc->token_type == TOKEN_REFERENCE &&
           (widget->arc->reference == highlight_reference 
            || widget->arc == highlight_reference)){
            push_rectangle(underline, 1, colour_from_v4f(v4f(1,0,0,1)));
        }
        
        if(ui->text_edit == widget->id){
            v2f next = {};
            next.x = pos.x + get_text_width_n(widget->alt_string, ui->cursor_pos, widget->style.font_scale);
            next.y = bbox.y;
            animate(&presenter->cursor.pos.x, next.x, 0.4f);
            animate(&presenter->cursor.pos.y, next.y, 0.4f);
            
            if(editor->mode == E_CREATE){
                push_rectangle(v4f2(presenter->cursor.pos, v2f(2, widget->min.height)), 1, colour_from_v4f(v4f(1,0,0,1)));
            }else {
                push_rectangle(v4f2(presenter->cursor.pos, v2f(2, widget->min.height)), 1, ui->theme.cursor);
            }
            
        }
        
    };
    
    widget->render_hook = render_hook;
    
    // NOTE(Oliver): custom text edit
    {
        if(ui->text_edit == widget->id){
            //ui->text_edit = widget->id;
            if(node->reference) highlight_reference = node->reference;
            edit_text(presenter->cursor.at);
        }
    }
    
    if(presenter->find_next_text_id){
        if(presenter->cursor.at == node){
            ui->text_edit = widget->id;
            presenter->find_next_text_id = false;
        }
    }
    
    auto result = update_widget(widget);
    
    Widget_Style style = {};
    
    style.text_colour = v4f_from_colour(colour),
    style.border_colour  = v4f_from_colour(ui->theme.text),
    style.font_scale = font_scale;
    widget->style = style;
    
    widget->style = style;
    
    if(ui->text_edit == widget->id){
        v2f size = get_text_size(widget->alt_string, widget->style.font_scale);
        widget->min = size;
    }else {
        v2f size = get_text_size(widget->alt_string, widget->style.font_scale);
        widget->min = size;
    }
    
    if(result.clicked){
        set_cursor_as_node(&presenter->cursor, widget->arc);
        
        ui->text_edit = widget->id;
        
    }
    
    if(ui->active == widget->id){
        ui->text_edit = widget->id;
    }
    
    if(result.hovered && node->reference){
        highlight_reference = node->reference;
    }
    presenter->pos++;
}

internal void
present_editable_string(Colour colour, Arc_Node* node){
    
    auto string = &node->string;
    auto widget_string = make_stringf(&platform->frame_arena, "editable_string%d", (int)node);
    auto widget = push_widget(widget_string);
    
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_ALT_STRING);
    
    Widget_Style style = {};
    
    style.text_colour = v4f_from_colour(colour),
    style.border_colour  = v4f_from_colour(ui->theme.text),
    style.font_scale = font_scale;
    style.background_colour = v4f_from_colour(ui->theme.background);
    style.rounded_corner_amount = 5.0f;
    
    push_style(style);
    
    widget->alt_string = node->string;
    widget->arc = node;
    
    if(presenter->select_start.at == node){
        presenter->start_pos = presenter->pos;
    }
    if(presenter->select_end.at == node){
        presenter->end_pos = presenter->pos;
    }
    
    if(ui->hot == widget->id){
        
        highlight_reference = node->reference;
    }
    
    widget->present_pos = presenter->pos;
    
    auto render_hook = [](Widget* widget ){
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        v4f bbox = v4f2(pos, widget->min);
        
        Colour colour = colour_from_v4f(widget->style.text_colour);
        v4f underline = v4f(bbox.x, bbox.y, bbox.width, 3);
        if(highlight_reference && (widget->arc->reference == highlight_reference || widget->arc == highlight_reference)){
            push_rectangle(underline, 1, colour_from_v4f(v4f(1,0,0,1)));
        }
        
        if(presenter->start_pos != presenter->end_pos &&
           presenter->start_pos == widget->present_pos){
            presenter->select_first_rect = bbox;
        }
        if(presenter->start_pos != presenter->end_pos &&
           presenter->end_pos == widget->present_pos){
            presenter->select_second_rect = bbox;
        }
        
        push_string(pos, widget->alt_string, colour, widget->style.font_scale);
        
        if(ui->text_edit == widget->id){
            
            v2f next = {};
            next.x = pos.x + get_text_width_n(widget->alt_string, ui->cursor_pos, widget->style.font_scale);
            next.y = bbox.y;
            
            v2f cursor_size = v2f(1.5, widget->min.height*0.9f);
            animate(&presenter->cursor.pos.x, next.x, 0.1f);
            animate(&presenter->cursor.pos.y, next.y, 0.2f);
            
            animate(&presenter->cursor.v0.x, next.x, 0.1f);
            animate(&presenter->cursor.v0.y, next.y, 0.1f);
            
            animate(&presenter->cursor.v1.x, next.x, 0.2f);
            animate(&presenter->cursor.v1.y, next.y+cursor_size.height/2.0, 0.3f);
            
            animate(&presenter->cursor.v2.x, next.x, 0.1f);
            animate(&presenter->cursor.v2.y, next.y+cursor_size.height, 0.3f);
            
            if(editor->mode == E_CREATE){
                render_cursor(&presenter->cursor, cursor_size,colour_from_v4f(v4f(1,0,0,1)));
            }else {
                render_cursor(&presenter->cursor, cursor_size, ui->theme.cursor);
            }
            
        }
        
    };
    
    widget->render_hook = render_hook;
    
    // NOTE(Oliver): custom text edit
    {
        if(ui->text_edit == widget->id){
            //ui->text_edit = widget->id;
            if(node->reference) highlight_reference = node->reference;
            edit_text(presenter->cursor.at);
        }
    }
    
    if(presenter->find_next_text_id){
        if(presenter->cursor.at == node){
            ui->text_edit = widget->id;
            presenter->find_next_text_id = false;
        }
    }
    
    auto result = update_widget(widget);
    
    
    if(widget->alt_string.length){
        v2f size = get_text_size(widget->alt_string, widget->style.font_scale);
        widget->min = size;
    }else {
        widget->min = get_text_size(" ", widget->style.font_scale);
    }
    
    if(result.clicked){
        set_cursor_as_node(&presenter->cursor, widget->arc);
        ui->text_edit = widget->id;
    } 
    
    if(result.hovered && node->reference){
        highlight_reference = node->reference;
    }
    presenter->pos++;
}

internal void present_arc(Arc_Node* node);

//~  DEFAULT SYNTAX 

internal void
present_declaration(Arc_Node* node){
    ID("declaration%d", (int)node){
        UI_ROW {
            present_editable_string(ui->theme.text, node);
            present_space();
            present_string(ui->theme.text_misc, make_string(":"));
            present_space();
            present_arc(node->first_child->first_child);
            if(node->last_child->first_child){
                if(node->last_child->first_child->next_sibling ||
                   node->last_child->first_child->string.length ||
                   presenter->cursor.at == node->last_child->first_child){
                    present_space();
                    present_string(ui->theme.text_misc, make_string("="));
                    present_space();
                    present_arc(node->last_child->first_child);
                }
            }
            
        }
    }
}

internal void
present_assignment(Arc_Node* node){
    ID("assignment%d", (int)node){
        UI_ROW {
            present_arc(node->first_child);
            present_space();
            present_string(ui->theme.text_misc, make_string("="));
            present_space();
            present_arc(node->last_child);
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
                            present_indent();
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
                            if(decl->next_sibling->string.length || presenter->cursor.at == decl->next_sibling){
                                present_string(ui->theme.text_misc, make_string(","));
                                present_space();
                            }
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
present_new(Arc_Node* node){
    UI_ROW {
        present_editable_string(ui->theme.text_type, node);
        present_space();
        present_arc(node->first_child);
    }
}

internal void
present_if(Arc_Node* node){
    ID("if%d", (int)node){
        
        UI_COLUMN{
            UI_ROW{
                ID("expr%d", (int)node->first_child){
                    present_editable_string(ui->theme.text_type, node);
                    present_space();
                    present_arc(node->first_child->first_child);
                    present_space();
                    present_string(ui->theme.text_misc, make_string("{"));
                }
            }
            UI_ROW {
                ID("scope%d", (int)node->last_child){
                    present_indent();
                    present_arc(node->last_child);
                }
            }
            UI_ROW {
                ID("%d", (int)node){
                    
                    present_string(ui->theme.text_misc, make_string("}"));
                }
            }
        }
    }
}

internal void
present_while(Arc_Node* node){
    ID("while%d", (int)node){
        
        UI_COLUMN{
            UI_ROW{
                ID("expr%d", (int)node->first_child){
                    present_editable_string(ui->theme.text_type, node);
                    present_space();
                    present_arc(node->first_child->first_child);
                    present_space();
                    present_string(ui->theme.text_misc, make_string("{"));
                }
            }
            UI_ROW {
                ID("scope%d", (int)node->last_child){
                    present_indent();
                    present_arc(node->last_child);
                }
            }
            UI_ROW {
                ID("%d", (int)node){
                    present_string(ui->theme.text_misc, make_string("}"));
                }
            }
        }
    }
}

internal void
present_for(Arc_Node* node){
    ID("for%d", (int)node){
        UI_COLUMN{
            UI_ROW {
                present_editable_string(ui->theme.text_type, node);
                ID("init%d"){
                    present_space();
                    present_arc(node->first_child->first_child);
                    present_string(ui->theme.text_misc, make_string(";"));
                    present_space();
                }
                
                ID("cond%d", (int)node){
                    present_arc(node->first_child->next_sibling->first_child);
                    present_string(ui->theme.text_misc, make_string(";"));
                    present_space();
                }
                
                ID("stmt%d", (int)node){
                    present_arc(node->first_child->next_sibling->next_sibling->first_child);
                    present_space();
                    present_string(ui->theme.text_misc, make_string("{"));
                }
            }
            UI_ROW {
                ID("body%d", (int)node){
                    present_space();
                    present_space();
                    present_arc(node->last_child);
                }
            }
            UI_ROW {
                present_string(ui->theme.text_misc, make_string("}"));
            }
        }
    }
}

internal void
present_return(Arc_Node* node){
    ID("return%d", (int)node){
        UI_ROW{
            present_editable_string(ui->theme.text_type, node);
            present_space();
            present_arc(node->first_child->first_child);
            present_space();
        }
    }
}

internal void
present_using(Arc_Node* node){
    ID("using%d", (int)node){
        UI_ROW{
            present_editable_string(ui->theme.text_type, node);
            present_space();
            present_arc(node->first_child);
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
                present_space();
                present_string(ui->theme.text_misc, make_string("{"));
            }
            UI_ROW {
                present_space();
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
present_call(Arc_Node* node){
    UI_ROW{
        ID("call%d", (int)node){
            ID("name%d", (int)node){
                if(presenter->cursor.at == node){
                    present_editable_string(ui->theme.text_function, node);
                }else{
                    replace_string(&node->string, node->reference->string);
                    present_editable_reference(ui->theme.text_function, node);
                }
            }
            present_string(ui->theme.text_misc, make_string("("));
            
            auto arg = node->first_child->first_child;
            assert(node->reference);
            auto param = node->reference->first_child;
            if(param){
                param = param->first_child;
            }else {
                param = nullptr;
            }
            
            for(auto expr = arg; expr; expr = expr->next_sibling){
                ID("args%d", (int)expr){
                    if(param){
                        present_string(ui->theme.text_misc, param->string);
                        present_string(ui->theme.text_misc, make_string(": "));
                    }
                    present_arc(expr->first_child);
                    if(expr->next_sibling && expr->next_sibling->first_child &&
                       (expr->next_sibling->first_child->string.length ||
                        presenter->cursor.at == expr->next_sibling->first_child)){
                        present_string(ui->theme.text_misc, make_string(","));
                        present_space();
                    }
                }
                if(param){
                    param = param->next_sibling;
                }
            }
            present_string(ui->theme.text_misc, make_string(")"));
        }
    }
}

//~ C SYNTAX

internal void
present_c_declaration(Arc_Node* node){
    ID("declaration%d", (int)node){
        UI_ROW {
            present_arc(node->first_child->first_child);
            present_space();
            present_editable_string(ui->theme.text, node);
            if(node->last_child->first_child){
                if(node->last_child->first_child->string.length ||
                   presenter->cursor.at == node->last_child->first_child){
                    present_space();
                    present_string(ui->theme.text_misc, make_string("="));
                    present_space();
                    present_arc(node->last_child->first_child);
                }
            }
            if(node->parent && node->parent->ast_type == AST_SCOPE){
                present_string(ui->theme.text_misc, make_string(";"));
            }
        }
    }
}

internal void
present_c_function(Arc_Node* node){
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
                            present_c_function(return_type);
                            present_space();
                            present_editable_string(ui->theme.text_function, node);
                            present_space();
                            present_string(ui->theme.text_misc, make_string("("));
                            present_c_function(params);
                            present_string(ui->theme.text_misc, make_string(")"));
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
                            if(decl->next_sibling->string.length || presenter->cursor.at == decl->next_sibling){
                                present_string(ui->theme.text_misc, make_string(","));
                                present_space();
                            }
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
present_c_type_usage(Arc_Node* node){
    present_editable_string(ui->theme.text_type, node);
}

internal void
present_c_if(Arc_Node* node){
    ID("if%d", (int)node){
        
        UI_COLUMN{
            UI_ROW{
                present_editable_string(ui->theme.text, node);
                present_space();
                present_string(ui->theme.text_misc, make_string("("));
                present_arc(node->first_child->first_child);
                present_string(ui->theme.text_misc, make_string(")"));
                present_space();
                present_string(ui->theme.text_misc, make_string("{"));
            }
            UI_ROW {
                present_space();
                present_space();
                present_arc(node->last_child);
            }
            UI_ROW {
                present_string(ui->theme.text_misc, make_string("}"));
            }
        }
    }
}

internal void
present_c_return(Arc_Node* node){
    ID("return%d", (int)node){
        UI_ROW{
            present_editable_string(ui->theme.text_type, node);
            present_space();
            present_arc(node->first_child->first_child);
            present_space();
        }
    }
}

internal void
present_c_struct(Arc_Node* node){
    ID("struct%d", (int)node){
        
        UI_COLUMN{
            UI_ROW{
                present_string(ui->theme.text, make_string("struct"));
                present_space();
                present_editable_string(ui->theme.text_type, node);
                present_space();
                present_string(ui->theme.text_misc, make_string("{"));
            }
            UI_ROW {
                present_space();
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
present_c_call(Arc_Node* node){
    ID("call%d", (int)node){
        UI_ROW{
            if(presenter->cursor.at == node){
                present_editable_string(ui->theme.text_function, node);
            }else{
                replace_string(&node->string, node->reference->string);
                present_editable_reference(ui->theme.text_function, node);
            }
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
                    if(expr->next_sibling && expr->next_sibling->first_child){
                        present_string(ui->theme.text_misc, make_string(","));
                        present_space();
                    }
                }
                if(param){
                    param = param->next_sibling;
                }
            }
            present_string(ui->theme.text_misc, make_string(")"));
        }
    }
}

//~ PASCAL SYNAX

internal void
present_pascal_declaration(Arc_Node* node){
    ID("declaration%d", (int)node){
        UI_ROW {
            present_arc(node->first_child->first_child);
            present_space();
            present_editable_string(ui->theme.text, node);
            if(node->last_child->first_child){
                if(node->last_child->first_child->string.length ||
                   presenter->cursor.at == node->last_child->first_child){
                    present_space();
                    present_string(ui->theme.text_misc, make_string("="));
                    present_space();
                    present_arc(node->last_child->first_child);
                }
            }
            present_string(ui->theme.text_misc, make_string(";"));
            
        }
    }
}

internal void
present_pascal_function(Arc_Node* node){
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
                            if(return_type->string.length){
                                present_string(ui->theme.text_misc, make_string("function"));
                            }else{
                                present_string(ui->theme.text_misc, make_string("procedure"));
                            }
                            present_space();
                            present_editable_string(ui->theme.text_function, node);
                            present_space();
                            present_string(ui->theme.text_misc, make_string("("));
                            present_pascal_function(params);
                            present_string(ui->theme.text_misc, make_string(")"));
                            present_string(ui->theme.text_misc, make_string(":"));
                            present_space();
                            present_pascal_function(return_type);
                            
                        }
                        UI_ROW{
                            present_string(ui->theme.text_misc, make_string("begin"));
                        }
                        UI_ROW {
                            present_space();
                            present_space();
                            present_arc(scope);
                        }
                        UI_ROW {
                            present_string(ui->theme.text_misc, make_string("end;"));
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
                            if(decl->next_sibling->string.length || presenter->cursor.at == decl->next_sibling){
                                present_string(ui->theme.text_misc, make_string(","));
                                present_space();
                            }
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
present_pascal_type_usage(Arc_Node* node){
    present_editable_string(ui->theme.text_type, node);
}

internal void
present_pascal_if(Arc_Node* node){
    ID("if%d", (int)node){
        
        UI_COLUMN{
            UI_ROW{
                present_editable_string(ui->theme.text, node);
                present_space();
                present_string(ui->theme.text_misc, make_string("("));
                present_arc(node->first_child->first_child);
                present_string(ui->theme.text_misc, make_string(")"));
                present_space();
                present_string(ui->theme.text_misc, make_string("{"));
            }
            UI_ROW {
                present_space();
                present_space();
                present_arc(node->last_child);
            }
            UI_ROW {
                present_string(ui->theme.text_misc, make_string("}"));
            }
        }
    }
}

internal void
present_pascal_return(Arc_Node* node){
    ID("return%d", (int)node){
        UI_ROW{
            present_editable_string(ui->theme.text_type, node);
            present_space();
            present_arc(node->first_child->first_child);
            present_space();
        }
    }
}

internal void
present_pascal_struct(Arc_Node* node){
    ID("struct%d", (int)node){
        
        UI_COLUMN{
            UI_ROW{
                present_string(ui->theme.text, make_string("struct"));
                present_space();
                present_editable_string(ui->theme.text_type, node);
                present_space();
                present_string(ui->theme.text_misc, make_string("{"));
            }
            UI_ROW {
                present_space();
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
present_pascal_call(Arc_Node* node){
    ID("call%d", (int)node){
        UI_ROW{
            if(presenter->cursor.at == node){
                present_editable_string(ui->theme.text_function, node);
            }else{
                replace_string(&node->string, node->reference->string);
                present_editable_reference(ui->theme.text_function, node);
            }
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
                    if(expr->next_sibling && expr->next_sibling->first_child){
                        present_string(ui->theme.text_misc, make_string(","));
                        present_space();
                    }
                }
                if(param){
                    param = param->next_sibling;
                }
            }
            present_string(ui->theme.text_misc, make_string(")"));
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
        case AST_ASSIGNMENT:{
            present_assignment(node);
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
        case AST_FOR: {
            present_for(node);
        }break;
        case AST_IF: {
            present_if(node);
        }break;
        case AST_NEW: {
            present_new(node);
        }break;
        case AST_WHILE: {
            present_while(node);
        }break;
        case AST_SCOPE: {
            auto member = node->first_child;
            UI_COLUMN{
                while(member){
                    if(member->next_sibling || 
                       member->string.length || 
                       member == presenter->cursor.at){
                        present_arc(member);
                    }
                    member = member->next_sibling;
                }
            }
        }break;
        case AST_EXPR:{
            present_ast(node->first_child);
        }break;
        case AST_TOKEN: {
            ID("token%d", (int)node){
                
                if(node->token_type == TOKEN_MISC){
                    present_editable_string(ui->theme.text_misc, node);
                }else if(node->token_type == TOKEN_REFERENCE){
                    if(presenter->cursor.at->reference &&
                       presenter->cursor.at->reference->ast_type == AST_FUNCTION){
                    }else if(presenter->cursor.at == node){
                        present_editable_string(ui->theme.text_function, node);
                    }else{
                        if(node->reference){
                            replace_string(&node->string, node->reference->string);
                            present_editable_reference(ui->theme.text_function, node);
                        }
                    }
                    if(node->first_child){
                        present_arc(node->first_child);
                    }
                }else if(node->token_type == TOKEN_LITERAL){
                    present_editable_string(ui->theme.text_literal, node);
                }else if(node->token_type == TOKEN_ARRAY){
                    present_string(ui->theme.text_misc, make_string("["));
                    present_arc(node->first_child);
                    present_string(ui->theme.text_misc, make_string("]"));
                }else if(node->token_type == TOKEN_STRING){
                    ID("left"){
                        present_string(ui->theme.text_misc, make_string("\""));
                    }
                    present_editable_string(ui->theme.text_literal, node);
                    ID("right"){
                        present_string(ui->theme.text_misc, make_string("\""));
                    }
                }else {
                    present_editable_string(ui->theme.text, node);
                }
                
                ID("tab_completer%d", (int)node){
                    auto preview = tab_completer(node);
                    if(preview.length){
                        present_string(ui->theme.text_misc, preview);
                    }
                }
                if(node->token_type == TOKEN_REFERENCE &&
                   node->next_sibling && node->next_sibling->token_type == TOKEN_REFERENCE){
                    present_string(ui->theme.text_misc, make_string("."));
                    present_arc(node->next_sibling);
                }
                else if(node->next_sibling && 
                        (node->next_sibling->next_sibling || 
                         node->next_sibling->string.length || 
                         node->next_sibling == presenter->cursor.at ||
                         node->next_sibling->token_type == TOKEN_ARRAY)){
                    
                    if(node->next_sibling->token_type != TOKEN_ARRAY){
                        present_space();
                    }
                    present_arc(node->next_sibling);
                }
            }
        }break;
        case AST_TYPE_TOKEN: {
            
            if(node->token_type == TOKEN_MISC){
                present_editable_string(ui->theme.text_misc, node);
            }else if(node->token_type == TOKEN_REFERENCE){
                
                for(int i = 0; i < node->number_of_pointers; i++){
                    ID("pointers%d", i){
                        present_string(ui->theme.text_misc, make_string("^"));
                    }
                }
                if(presenter->cursor.at == node){
                    present_editable_string(ui->theme.text_type, node);
                    
                }else{
                    replace_string(&node->string, node->reference->string);
                    present_editable_reference(ui->theme.text_type, node);
                }
                
            }else if(node->token_type == TOKEN_ARRAY){
                
                present_string(ui->theme.text_misc, make_string("["));
                present_editable_string(ui->theme.text_literal, node);
                present_string(ui->theme.text_misc, make_string("]"));
                
            }else if(node->token_type == TOKEN_LITERAL){
                present_editable_string(ui->theme.text_literal, node);
            }else {
                present_editable_string(ui->theme.text, node);
            }
            ID("tab_completer%d", (int)node){
                auto preview = tab_completer_type(node);
                if(preview.length){
                    present_string(ui->theme.text_misc, preview);
                }
            }
            if(node->next_sibling && node->token_type != TOKEN_ARRAY &&
               (node->next_sibling->string.length || 
                node->next_sibling == presenter->cursor.at)){
                present_space();
            }
            
            present_arc(node->next_sibling);
        }break;
        case AST_CALL:{
            present_call(node);
        }break;
        case AST_USING: {
            present_using(node);
        }break;
        case AST_RETURN:{
            present_return(node);
        }break;
    }
}

internal void
present_c_ast(Arc_Node* node){
    if(!node) return;
    switch(node->ast_type){
        case AST_DECLARATION: {
            present_c_declaration(node);
        }break;
        case AST_STRUCT: {
            present_c_struct(node);
        }break;
        case AST_FUNCTION: {
            present_c_function(node);
        }break;
        case AST_TYPE_USAGE: {
            present_c_type_usage(node);
        }break;
        case AST_IF: {
            present_c_if(node);
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
            present_c_ast(node->first_child);
        }break;
        case AST_TOKEN: {
            
            if(node->token_type == TOKEN_MISC){
                present_editable_string(ui->theme.text_misc, node);
            }else if(node->token_type == TOKEN_REFERENCE){
                if(presenter->cursor.at->reference &&
                   presenter->cursor.at->reference->ast_type == AST_FUNCTION){
                }else if(presenter->cursor.at == node){
                    present_editable_string(ui->theme.text_function, node);
                }else{
                    if(node->reference){
                        replace_string(&node->string, node->reference->string);
                        present_editable_reference(ui->theme.text_function, node);
                    }
                }
                if(node->first_child){
                    present_arc(node->first_child);
                }
            }else if(node->token_type == TOKEN_LITERAL){
                present_editable_string(ui->theme.text_literal, node);
            }else if(node->token_type == TOKEN_ARRAY){
                present_string(ui->theme.text_misc, make_string("["));
                present_arc(node->first_child);
                present_string(ui->theme.text_misc, make_string("]"));
            }else if(node->token_type == TOKEN_STRING){
                ID("left"){
                    present_string(ui->theme.text_misc, make_string("\""));
                }
                present_editable_string(ui->theme.text_literal, node);
                ID("right"){
                    present_string(ui->theme.text_misc, make_string("\""));
                }
            }else {
                present_editable_string(ui->theme.text, node);
            }
            
            ID("tab_completer%d", (int)node){
                auto preview = tab_completer(node);
                if(preview.length){
                    present_string(ui->theme.text_misc, preview);
                }
            }
            if(node->token_type == TOKEN_REFERENCE &&
               node->next_sibling && node->next_sibling->token_type == TOKEN_REFERENCE){
                present_string(ui->theme.text_misc, make_string("."));
                present_arc(node->next_sibling);
            }
            else if(node->next_sibling && (node->next_sibling->string.length || 
                                           node->next_sibling == presenter->cursor.at ||
                                           node->next_sibling->token_type == TOKEN_ARRAY)){
                if(node->next_sibling->token_type != TOKEN_ARRAY){
                    present_space();
                }
                present_arc(node->next_sibling);
            }
        }break;
        case AST_TYPE_TOKEN: {
            if(node->token_type == TOKEN_MISC){
                present_editable_string(ui->theme.text_misc, node);
            }else if(node->token_type == TOKEN_REFERENCE){
                if(presenter->cursor.at == node){
                    present_editable_string(ui->theme.text_type, node);
                }else{
                    replace_string(&node->string, node->reference->string);
                    present_editable_reference(ui->theme.text_type, node);
                }
                
            }else if(node->token_type == TOKEN_LITERAL){
                present_editable_string(ui->theme.text_literal, node);
            }else {
                present_editable_string(ui->theme.text, node);
            }
            ID("tab_completer%d", (int)node){
                auto preview = tab_completer_type(node);
                if(preview.length){
                    present_string(ui->theme.text_misc, preview);
                }
            }
            if(node->next_sibling){
                present_space();
            }
            present_arc(node->next_sibling);
        }break;
        case AST_CALL:{
            present_call(node);
        }break;
        case AST_RETURN:{
            present_c_return(node);
        }break;
    }
}

internal void
present_pascal_ast(Arc_Node* node){
    if(!node) return;
    switch(node->ast_type){
        case AST_DECLARATION: {
            present_pascal_declaration(node);
        }break;
        case AST_STRUCT: {
            present_pascal_struct(node);
        }break;
        case AST_FUNCTION: {
            present_pascal_function(node);
        }break;
        case AST_TYPE_USAGE: {
            present_pascal_type_usage(node);
        }break;
        case AST_IF: {
            present_pascal_if(node);
        }break;
        case AST_SCOPE: {
            auto member = node->first_child;
            UI_COLUMN{
                while(member){
                    present_arc(member);
                    if(member->ast_type == AST_DECLARATION){
                        present_string(ui->theme.text_misc, make_string(";"));
                    }
                    member = member->next_sibling;
                    
                }
            }
        }break;
        case AST_EXPR:{
            present_pascal_ast(node->first_child);
        }break;
        case AST_TOKEN: {
            ID("token%d", (int)node){
                if(node->token_type == TOKEN_MISC){
                    present_editable_string(ui->theme.text_misc, node);
                }else if(node->token_type == TOKEN_REFERENCE){
                    if(presenter->cursor.at == node){
                        present_editable_string(ui->theme.text_function, node);
                    }else{
                        replace_string(&node->string, node->reference->string);
                        present_editable_reference(ui->theme.text_function, node);
                    }
                    if(node->first_child){
                        present_string(ui->theme.text_misc, make_string("."));
                        present_arc(node->first_child);
                    }
                }else if(node->token_type == TOKEN_LITERAL){
                    present_editable_string(ui->theme.text_literal, node);
                }else {
                    present_editable_string(ui->theme.text, node);
                }
                ID("tab_completer%d", (int)node){
                    auto preview = tab_completer(node);
                    if(preview.length){
                        present_string(ui->theme.text_misc, preview);
                    }
                }
                if(node->next_sibling){
                    present_space();
                }
                present_arc(node->next_sibling);
            }
        }break;
        case AST_TYPE_TOKEN: {
            if(node->token_type == TOKEN_MISC){
                present_editable_string(ui->theme.text_misc, node);
            }else if(node->token_type == TOKEN_REFERENCE){
                if(presenter->cursor.at == node){
                    present_editable_string(ui->theme.text_type, node);
                }else{
                    replace_string(&node->string, node->reference->string);
                    present_editable_reference(ui->theme.text_type, node);
                }
                
            }else if(node->token_type == TOKEN_LITERAL){
                present_editable_string(ui->theme.text_literal, node);
            }else{
                present_editable_string(ui->theme.text, node);
            }
            ID("tab_completer%d", (int)node){
                auto preview = tab_completer_type(node);
                if(preview.length){
                    present_string(ui->theme.text_misc, preview);
                }
            }
            if(node->next_sibling){
                present_space();
            }
            present_arc(node->next_sibling);
        }break;
        case AST_CALL:{
            present_pascal_call(node);
        }break;
        case AST_RETURN:{
            present_pascal_return(node);
        }break;
    }
}

internal void
present_arc(Arc_Node* node){
    if(!node) return;
    if(arc_has_property(node, AP_SLIDER)){
        local_persist f32 ui_mix_test = 0;
        fslider(0, 10, &ui_mix_test, "slider");
        present_space();
        present_arc(node->next_sibling);
        present_arc(node->first_child);
    }
    else if(arc_has_property(node, AP_IMAGE)){
        local_persist f32 image_scale = 0.2f;
        UI_ROW {
            UI_COLUMN{
                UI_ROW{
                    present_editable_string(ui->theme.text, node);
                    editor->image_location = node->string;
                    present_space();
                    if(button("load")){
                        editor->image = make_bitmap(string_to_cstr(&platform->frame_arena, editor->image_location));
                        
                    }
                    present_space();
                }
                fslider(0, 1, &image_scale, "image scale");
                image(editor->image, image_scale, "image");
            }
        }
    }
    else if(arc_has_property(node, AP_AST)){
        if(present_style == 0){
            present_ast(node);
        }else if(present_style == 1){
            present_c_ast(node);
        }else if(present_style == 2){
            present_pascal_ast(node);
        }
    }else {
        UI_ROW{
            ID("emptynode%d", (int)node){
                present_editable_string(ui->theme.text, node);
                ID("tab_completer%d", (int)node){
                    auto preview = tab_completer(node);
                    if(preview.length){
                        present_string(ui->theme.text_misc, preview);
                    }
                }
            }
        }
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
        push_string(pos + v2f(25, 0), node->string, ui->theme.text, 0.5f);
        offset = get_text_width(node->string, .5f) + 20;
        if(node == presenter->cursor.at){
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

//~ Build navigation buffer from ARC

internal void
build_buffer_from_arc(Arc_Node* node){
    if(!node) return;
    
    switch(node->ast_type){
        case AST_DECLARATION: {
            push_arc(node);
            build_buffer_from_arc(node->first_child->first_child);
            build_buffer_from_arc(node->last_child->first_child);
        }break;
        case AST_ASSIGNMENT:{
            build_buffer_from_arc(node->first_child);
            build_buffer_from_arc(node->last_child);
        }break;
        case AST_STRUCT: {
            push_arc(node);
            push_newline();
            build_buffer_from_arc(node->first_child);
        }break;
        case AST_FUNCTION: {
            push_arc(node);
            
            auto params = node->first_child->first_child;
            for(auto decl = params; decl; decl = decl->next_sibling){
                build_buffer_from_arc(decl);
            }
            build_buffer_from_arc(node->first_child->next_sibling->first_child);
            build_buffer_from_arc(node->last_child);
        }break;
        case AST_TYPE_USAGE: {
            push_arc(node);
        }break;
        case AST_IF: {
            push_arc(node);
            build_buffer_from_arc(node->first_child->first_child);
            push_newline();
            build_buffer_from_arc(node->last_child);
        }break;
        case AST_WHILE: {
            push_arc(node);
            build_buffer_from_arc(node->first_child->first_child);
            push_newline();
            build_buffer_from_arc(node->last_child);
        }break;
        case AST_NEW: {
            push_arc(node);
            build_buffer_from_arc(node->first_child);
        }break;
        case AST_FOR: {
            push_arc(node);
            build_buffer_from_arc(node->first_child->first_child);
            build_buffer_from_arc(node->first_child->next_sibling->first_child);
            build_buffer_from_arc(node->first_child->next_sibling->next_sibling->first_child);
            push_newline();
            build_buffer_from_arc(node->last_child);
        }break;
        case AST_SCOPE: {
            auto member = node->first_child;
            while(member){
                build_buffer_from_arc(member);
                push_newline();
                member = member->next_sibling;
            }
            push_newline();
        }break;
        case AST_EXPR:{
            build_buffer_from_arc(node->first_child);
        }break;
        case AST_TOKEN: {
            if(node->token_type != TOKEN_ARRAY){
                push_arc(node);
            }
            if(node->first_child){
                build_buffer_from_arc(node->first_child);
            }
            build_buffer_from_arc(node->next_sibling);
        }break;
        case AST_CALL:{
            push_arc(node);
            auto arg = node->first_child->first_child;
            for(auto expr = arg; expr; expr = expr->next_sibling){
                build_buffer_from_arc(expr);
            }
        }break;
        case AST_RETURN:{
            push_arc(node);
            build_buffer_from_arc(node->first_child->first_child);
        }break;
        default:{
            if(arc_has_property(node, AP_SELECTABLE)){
                push_arc(node);
            }
            build_buffer_from_arc(node->next_sibling);
            build_buffer_from_arc(node->first_child);
        }break;
    }
}

internal void
build_navigation_buffer(Arc_Node* node){
    presenter->buffer_pos = 0;
    presenter->buffer = (Present_Node*)push_size_zero(&platform->frame_arena, sizeof(Present_Node)*8192);
    
    presenter->line_pos = 0;
    presenter->lines = (Line_Info*)push_size_zero(&platform->frame_arena, sizeof(Line_Info)*8192);
    
    build_buffer_from_arc(node);
}

