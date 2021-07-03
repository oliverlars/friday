
internal Arc_Node*
make_editable_declaration(Arc_Node* decl, Pool* pool){
    
    arc_set_property(decl, AP_AST);
    decl->ast_type = AST_DECLARATION;
    
    auto type = make_arc_node(pool);
    auto expr = make_arc_node(pool);
    set_as_ast(expr, AST_EXPR);
    arc_set_property(expr, AP_LIST);
    arc_set_property(type, AP_LIST);
    
    insert_arc_node_as_child(decl, type);
    insert_arc_node_as_sibling(type, expr);
    
    
    auto selectable_type = make_selectable_arc_node(pool);
    arc_remove_property(selectable_type, AP_DELETABLE);
    auto selectable_expr = make_selectable_arc_node(pool);
    arc_remove_property(selectable_expr, AP_DELETABLE);
    
    insert_arc_node_as_child(type, selectable_type);
    
    set_as_ast(selectable_type, AST_TYPE_TOKEN);
    insert_arc_node_as_child(expr, selectable_expr);
    
    set_as_ast(selectable_expr, AST_TOKEN);
    
    return decl;
}

internal Arc_Node*
make_editable_if(Arc_Node* _if, Pool* pool){
    
    arc_set_property(_if, AP_AST);
    arc_set_property(_if, AP_CONTAINS_SCOPE);
    _if->ast_type = AST_IF;
    
    auto expr = make_arc_node(pool);
    arc_set_property(expr, AP_AST);
    expr->ast_type = AST_EXPR;
    auto scope = make_arc_node(pool);
    
    arc_set_property(scope, AP_AST);
    arc_set_property(scope, AP_LIST);
    scope->ast_type = AST_SCOPE;
    scope->ast_tag = AST_TAG_BODY;
    
    insert_arc_node_as_child(_if, expr);
    insert_arc_node_as_sibling(expr, scope);
    
    auto selectable_expr = make_selectable_arc_node(pool);
    arc_remove_property(selectable_expr, AP_DELETABLE);
    set_as_ast(selectable_expr, AST_TOKEN);
    
    auto selectable_scope = make_selectable_arc_node(pool);
    arc_remove_property(selectable_expr, AP_DELETABLE);
    
    insert_arc_node_as_child(expr, selectable_expr);
    insert_arc_node_as_child(scope, selectable_scope);
    
    return _if;
}

internal Arc_Node*
make_editable_while(Arc_Node* _while, Pool* pool){
    set_as_ast(_while, AST_WHILE);
    
    auto expr = make_arc_node(pool);
    set_as_ast(expr, AST_EXPR);
    
    auto scope = make_arc_node(pool);
    set_as_ast(scope, AST_SCOPE);
    arc_set_property(scope, AP_LIST);
    scope->ast_tag = AST_TAG_BODY;
    
    insert_arc_node_as_child(_while, expr);
    insert_arc_node_as_sibling(expr, scope);
    
    auto selectable_expr = make_selectable_arc_node(pool);
    arc_remove_property(selectable_expr, AP_DELETABLE);
    set_as_ast(selectable_expr, AST_TOKEN);
    
    auto selectable_scope = make_selectable_arc_node(pool);
    arc_remove_property(selectable_expr, AP_DELETABLE);
    
    insert_arc_node_as_child(expr, selectable_expr);
    insert_arc_node_as_child(scope, selectable_scope);
    
    return _while;
}

internal Arc_Node*
make_editable_return(Arc_Node* ret, Pool* pool){
    arc_set_property(ret, AP_AST);
    ret->ast_type = AST_RETURN;
    
    auto expr = make_arc_node(pool);
    arc_set_property(expr, AP_AST);
    expr->ast_type = AST_EXPR;
    
    insert_arc_node_as_child(ret, expr);
    
    auto selectable_expr = make_selectable_arc_node(pool);
    arc_remove_property(selectable_expr, AP_DELETABLE);
    set_as_ast(selectable_expr, AST_TOKEN);
    
    insert_arc_node_as_child(expr, selectable_expr);
    
    return ret;
}

internal Arc_Node*
make_editable_using(Arc_Node* _using, Pool* pool){
    arc_set_property(_using, AP_AST);
    _using->ast_type = AST_USING;
    
    auto expr = make_arc_node(pool);
    arc_set_property(expr, AP_AST);
    expr->ast_type = AST_EXPR;
    
    insert_arc_node_as_child(_using, expr);
    
    auto selectable_expr = make_selectable_arc_node(pool);
    arc_remove_property(selectable_expr, AP_DELETABLE);
    set_as_ast(selectable_expr, AST_TOKEN);
    
    insert_arc_node_as_child(expr, selectable_expr);
    
    return _using;
}

internal Arc_Node*
make_editable_function(Arc_Node* func, Pool* pool){
    arc_set_property(func, AP_AST);
    arc_set_property(func, AP_CONTAINS_SCOPE);
    func->ast_type = AST_FUNCTION;
    
    auto params = make_arc_node(pool);
    arc_set_property(params, AP_AST);
    arc_set_property(params, AP_LIST);
    params->ast_tag = AST_TAG_PARAMS;
    
    auto ret = make_arc_node(pool);
    arc_set_property(ret, AP_AST);
    ret->ast_tag = AST_TAG_RETURN_TYPE;
    
    auto scope = make_arc_node(pool);
    arc_set_property(scope, AP_AST);
    arc_set_property(scope, AP_LIST);
    scope->ast_type = AST_SCOPE;
    scope->ast_tag = AST_TAG_BODY;
    
    insert_arc_node_as_child(func, params);
    insert_arc_node_as_sibling(params, ret);
    insert_arc_node_as_sibling(ret, scope);
    
    auto selectable_param = make_selectable_arc_node(pool);
    arc_remove_property(selectable_param, AP_DELETABLE);
    
    auto selectable_ret = make_selectable_arc_node(pool);
    arc_remove_property(selectable_ret, AP_DELETABLE);
    set_as_ast(selectable_ret, AST_TYPE_TOKEN);
    
    auto selectable_scope = make_selectable_arc_node(pool);
    arc_remove_property(selectable_scope, AP_DELETABLE);
    
    insert_arc_node_as_child(params, selectable_param);
    insert_arc_node_as_child(ret, selectable_ret);
    insert_arc_node_as_child(scope, selectable_scope);
    
    
    return func;
}

internal Arc_Node*
make_editable_assignment(Arc_Node* assign, Pool* pool){
    arc_set_property(assign, AP_AST);
    assign->ast_type = AST_ASSIGNMENT;
    
    auto lhs = make_arc_node(pool);
    set_as_ast(lhs, AST_EXPR);
    
    auto rhs = make_arc_node(pool);
    set_as_ast(rhs, AST_EXPR);
    
    insert_arc_node_as_child(assign, lhs);
    insert_arc_node_as_sibling(lhs, rhs);
    
    auto selectable_lhs = make_selectable_arc_node(pool);
    arc_remove_property(selectable_lhs, AP_DELETABLE);
    insert_arc_node_as_child(lhs, selectable_lhs);
    selectable_lhs->string = assign->string;
    selectable_lhs->reference = assign->reference;
    set_as_ast(selectable_lhs, AST_TOKEN);
    
    auto selectable_rhs = make_selectable_arc_node(pool);
    arc_remove_property(selectable_rhs, AP_DELETABLE);
    set_as_ast(selectable_rhs, AST_TOKEN);
    insert_arc_node_as_child(rhs, selectable_rhs);
    
    return assign;
}

internal void
handle_creation_mode_input() {
    
    auto current = presenter->cursor.at;
    auto pool = &editor->arc_pool;
    
    
    if(string_eq(current->string, "if")){
        arc_set_property(current, AP_DELETABLE);
        auto next_in_scope = make_selectable_arc_node(pool);
        
        arc_remove_property(next_in_scope, AP_DELETABLE);
        insert_arc_node_as_sibling(current, next_in_scope);
        make_editable_if(current, pool);
        advance_cursor(&presenter->cursor, CURSOR_RIGHT);
        editor->mode = E_EDIT;
    }
    else if(string_eq(current->string, "while")){
        arc_set_property(current, AP_DELETABLE);
        auto next_in_scope = make_selectable_arc_node(pool);
        
        arc_remove_property(next_in_scope, AP_DELETABLE);
        insert_arc_node_as_sibling(current, next_in_scope);
        make_editable_while(current, pool);
        advance_cursor(&presenter->cursor, CURSOR_RIGHT);
        editor->mode = E_EDIT;
    }
    else if(string_eq(current->string, "return")){
        arc_set_property(current, AP_DELETABLE);
        auto next_in_scope = make_selectable_arc_node(pool);
        
        arc_remove_property(next_in_scope, AP_DELETABLE);
        insert_arc_node_as_sibling(current, next_in_scope);
        make_editable_return(current, pool);
        advance_cursor(&presenter->cursor, CURSOR_RIGHT);
        editor->mode = E_EDIT;
    }
    else if(has_pressed_key(KEY_D)){
        arc_set_property(current, AP_DELETABLE);
        auto next_in_scope = make_selectable_arc_node(pool);
        
        arc_remove_property(next_in_scope, AP_DELETABLE);
        insert_arc_node_as_sibling(current, next_in_scope);
        make_editable_declaration(current, pool);
        advance_cursor(&presenter->cursor, CURSOR_RIGHT);
        editor->mode = E_EDIT;
    }
    else if(has_pressed_key(KEY_F)){
        
        arc_set_property(current, AP_DELETABLE);
        auto next_in_scope = make_selectable_arc_node(pool);
        
        arc_remove_property(next_in_scope, AP_DELETABLE);
        insert_arc_node_as_sibling(current, next_in_scope);
        make_editable_function(current, pool);
        advance_cursor(&presenter->cursor, CURSOR_RIGHT);
        editor->mode = E_EDIT;
        
    }
}

internal void
handle_edit_mode_input() {
    
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
    
    auto current = presenter->cursor.at;
    auto pool = &editor->arc_pool;
    if(has_pressed_key_modified(KEY_ENTER, KEY_MOD_CTRL)){
        auto next_in_scope = make_selectable_arc_node(pool);
        set_as_ast(next_in_scope, AST_TOKEN);
        insert_arc_node_as_sibling(current, next_in_scope);
        arc_set_property(current, AP_SLIDER);
        advance_cursor(&presenter->cursor, CURSOR_RIGHT);
    }
    else if(has_pressed_key_modified(KEY_I, KEY_MOD_CTRL)){
        auto next_in_scope = make_selectable_arc_node(pool);
        set_as_ast(next_in_scope, AST_TOKEN);
        insert_arc_node_as_sibling(current, next_in_scope);
        arc_set_property(current, AP_IMAGE);
    }
    else if(has_pressed_key(KEY_ENTER)){
        Arc_Node* list = nullptr;
        if(current->string.length){
            
            Arc_Node* would_be_reference = nullptr;
            if(can_resolve_reference(current, &would_be_reference) &&
               !arc_has_property(current, AP_AST)){
                arc_set_property(current, AP_DELETABLE);
                auto next_in_scope = make_selectable_arc_node(pool);
                
                arc_remove_property(next_in_scope, AP_DELETABLE);
                insert_arc_node_as_sibling(current, next_in_scope);
                set_token_type(current);
                make_editable_assignment(current, pool);
                editor->mode = E_EDIT;
            }
            else if(is_sub_node_of_list(current, &list) &&
                    arc_has_property(current, AP_AST)){
                auto next = make_selectable_arc_node(pool);
                if(current->ast_type == AST_TYPE_TOKEN){
                    set_as_ast(next, AST_TYPE_TOKEN);
                }else if(current->ast_type == AST_TOKEN){
                    set_as_ast(next, AST_TOKEN);
                }
                insert_arc_node_as_sibling(current, next);
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
            }else {
                editor->mode = E_CREATE;
            }
        }else {
            mark_node_for_deletion(current);
            advance_cursor(&presenter->cursor, CURSOR_RIGHT);
        }
        
    }
}

internal void
handle_editor_input(){
    
    if(editor->mode == E_CREATE){
        handle_creation_mode_input();
    }else if(editor->mode == E_EDIT){
        handle_edit_mode_input();
    }
}
