
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

internal void
handle_creation_mode_input() {
    
    auto current = presenter->cursor.at;
    auto pool = &editor->arc_pool;
    
    if(has_pressed_key(KEY_D)){
        arc_set_property(current, AP_DELETABLE);
        auto next_in_scope = make_selectable_arc_node(pool);
        arc_remove_property(next_in_scope, AP_DELETABLE);
        insert_arc_node_as_sibling(current, next_in_scope);
        make_editable_declaration(current, pool);
        advance_cursor(&presenter->cursor, CURSOR_RIGHT);
        editor->mode = E_EDIT;
    }
}

internal void
handle_edit_mode_input() {
    
    auto current = presenter->cursor.at;
    auto pool = &editor->arc_pool;
    
    if(has_pressed_key(KEY_ENTER)){
        Arc_Node* list = nullptr;
        if(current->string.length){
            if(is_sub_node_of_list(current, &list) &&
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
