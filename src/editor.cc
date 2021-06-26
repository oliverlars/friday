
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
    auto selectable_expr = make_selectable_arc_node(pool);
    
    insert_arc_node_as_child(type, selectable_type);
    set_as_ast(selectable_type, AST_TYPE_TOKEN);
    insert_arc_node_as_child(expr, selectable_expr);
    
    return decl;
}

internal void
handle_creation_mode_input() {
    
    auto current = presenter->cursor.at;
    auto pool = &editor->arc_pool;
    if(has_pressed_key(KEY_D)){
        make_editable_declaration(current, pool);
        advance_cursor(&presenter->cursor, CURSOR_RIGHT);
        editor->mode = E_EDIT;
    }
}

internal void
handle_edit_mode_input() {
    auto pool = &editor->arc_pool;
    
    if(has_pressed_key(KEY_ENTER)){
        Arc_Node* list = nullptr;
        if(presenter->cursor.at->string.length){
            if(is_sub_node_of_list(presenter->cursor.at, &list) &&
               arc_has_property(presenter->cursor.at, AP_AST)){
                auto next = make_selectable_arc_node(pool);
                insert_arc_node_as_sibling(presenter->cursor.at, next);
                advance_cursor(&presenter->cursor, CURSOR_RIGHT);
            }else {
                editor->mode = E_CREATE;
            }
        }else {
            mark_node_for_deletion(presenter->cursor.at);
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
