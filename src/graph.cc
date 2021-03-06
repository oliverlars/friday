
internal bool
arc_has_property(Arc_Node* arc, Arc_Property property){
    return !!(arc->properties[property / 64] & (1ll << (property % 64)));
}

internal void
arc_set_property(Arc_Node* arc, Arc_Property property){
    arc->properties[property / 64] |= (1ll << (property % 64));
}

internal void
arc_remove_property(Arc_Node* arc, Arc_Property property){
    arc->properties[property / 64] &= ~(1ll << (property % 64));
}

internal void
arc_clear_all_properties(Arc_Node* arc){
    for(int i = 0; i < NUM_PROPERTY_ARRAYS; i++){
        arc->properties[i] = 0;
    }
}

internal void
set_as_ast(Arc_Node* node, Ast_Type type){
    arc_set_property(node, AP_AST);
    node->ast_type = type;
}

internal Arc_Node*
make_arc_node(Pool* pool){
    auto result = (Arc_Node*)pool_allocate(pool);
    result->string.text = (char*)pool_allocate(&editor->string_pool);
    return result;
}

internal Arc_Node*
make_selectable_arc_node(Pool* pool){
    auto result = (Arc_Node*)pool_allocate(pool);
    result->string.text = (char*)pool_allocate(&editor->string_pool);
    arc_set_property(result, AP_SELECTABLE);
    arc_set_property(result, AP_DELETABLE);
    return result;
}

internal Arc_Node*
make_scope(Pool* pool){
    auto scope = make_arc_node(pool);
    arc_set_property(scope, AP_AST);
    scope->ast_type = AST_SCOPE;
    return scope;
}

internal Arc_Node*
make_new_from_node(Arc_Node* _new, Pool* pool){
    set_as_ast(_new, AST_NEW);
    return _new;
}

internal Arc_Node*
make_variadic_from_node(Arc_Node* variadic, Pool* pool){
    set_as_ast(variadic, AST_VARIADIC);
    return variadic;
}

internal Arc_Node*
make_foreign_from_node(Arc_Node* foreign, Pool* pool){
    set_as_ast(foreign, AST_FOREIGN);
    return foreign;
}

internal Arc_Node*
make_while_from_node(Arc_Node* _while, Pool* pool){
    set_as_ast(_while, AST_WHILE);
    
    auto expr = make_arc_node(pool);
    set_as_ast(expr, AST_EXPR);
    
    auto scope = make_arc_node(pool);
    set_as_ast(scope, AST_SCOPE);
    arc_set_property(scope, AP_LIST);
    scope->ast_tag = AST_TAG_BODY;
    
    insert_arc_node_as_child(_while, expr);
    insert_arc_node_as_sibling(expr, scope);
    
    return _while;
}

internal Arc_Node*
make_if_from_node(Arc_Node* _if, Pool* pool){
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
    
    assert(is_sub_node_of_ast_type(expr, AST_IF));
    assert(is_sub_node_of_ast_type(scope, AST_IF));
    
    return _if;
}

internal Arc_Node*
make_return_from_node(Arc_Node* ret, Pool* pool){
    arc_set_property(ret, AP_AST);
    ret->ast_type = AST_RETURN;
    
    auto expr = make_arc_node(pool);
    arc_set_property(expr, AP_AST);
    expr->ast_type = AST_EXPR;
    
    insert_arc_node_as_child(ret, expr);
    
    return ret;
}

internal Arc_Node*
make_using_from_node(Arc_Node* _using, Pool* pool){
    arc_set_property(_using, AP_AST);
    _using->ast_type = AST_USING;
    return _using;
}

internal Arc_Node*
make_assignment_from_node(Arc_Node* assign, Pool* pool){
    arc_set_property(assign, AP_AST);
    assign->ast_type = AST_ASSIGNMENT;
    
    auto lhs = make_arc_node(pool);
    set_as_ast(lhs, AST_EXPR);
    
    auto rhs = make_arc_node(pool);
    set_as_ast(rhs, AST_EXPR);
    
    insert_arc_node_as_child(assign, lhs);
    insert_arc_node_as_sibling(lhs, rhs);
    
    return assign;
}

internal Arc_Node*
make_function_from_node(Arc_Node* func, Pool* pool){
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
    
    assert(is_sub_node_of_ast_type(params, AST_FUNCTION));
    assert(is_sub_node_of_ast_type(ret, AST_FUNCTION));
    assert(is_sub_node_of_ast_type(scope, AST_FUNCTION));
    
    
    return func;
}

internal b32
is_sub_node_of_ast_type(Arc_Node* node, Ast_Type type, Arc_Node** result){
    if(!node) return false;
    node = node->parent;
    while(node){
        if(arc_has_property(node, AP_AST)){
            if(node->ast_type == type){
                if(result) *result = node;
                return true;
            }
        }
        node = node->parent;
    }
    return false;
}

internal b32
find_sub_node_of_scope(Arc_Node* node, Arc_Node** result){
    if(!node) return false;
    node = node->parent;
    while(node){
        if(node->parent){
            if(arc_has_property(node->parent, AP_AST)){
                if(node->parent->ast_type == AST_SCOPE){
                    if(result) *result = node;
                    return true;
                }
            }
            node = node->parent;
        }
    }
    return false;
}

internal b32
find_next_sub_node_of_scope(Arc_Node* node, Arc_Node** result){
    if(!node) return false;
    while(node){
        if(arc_has_property(node, AP_LIST) && node->ast_type == AST_SCOPE){
            if(result) *result = node;
            return true;
        }
        if(node->first_child){
            find_next_sub_node_of_scope(node->first_child, result);
        }
        node = node->next_sibling;
    }
    return false;
}

internal b32
find_a_nearby_scope(Arc_Node* node, Arc_Node** result){
    if(!node) return false;
    auto did_find = false;
    did_find = find_next_sub_node_of_scope(node, result);
    if(!did_find){
        did_find = find_sub_node_of_scope(node, result);
    }
    assert(did_find);
    return did_find;
}

internal b32
is_direct_sub_node_of_ast_type(Arc_Node* node, Ast_Type type, Arc_Node** result){
    if(!node) return false;
    //node = node->parent;
    while(node){
        if(arc_has_property(node, AP_AST)){
            if(node->ast_type == type){
                if(result) *result = node;
                return true;
            }else if(node->ast_type != AST_SCOPE){
                return false;
            }
        }
        node = node->parent;
    }
    return false;
}

internal b32
is_sub_node_of_ast_tag(Arc_Node* node, Ast_Tag type, Arc_Node** result){
    if(!node) return false;
    node = node->parent;
    while(node){
        if(arc_has_property(node, AP_AST)){
            if(node->ast_tag == type){
                if(result) *result = node;
                return true;
            }
        }
        node = node->parent;
    }
    return false;
}

internal b32
find_sub_node_of_list(Arc_Node* node, Arc_Node** result){
    if(!node) return false;
    while(node){
        if(node->parent){
            if(arc_has_property(node->parent, AP_LIST)){
                if(result) *result = node;
                return true;
            }
        }
        node = node->parent;
        
    }
    return false;
}

internal b32
find_parent_list(Arc_Node* node, Arc_Node** result){
    if(!node) return false;
    
    node = node->parent;
    while(node){
        if(node->parent){
            if(arc_has_property(node->parent, AP_LIST)){
                if(result) *result = node;
                return true;
            }
        }
        node = node->parent;
        
    }
    return false;
}

internal void
find_inner_scope(Arc_Node* node, Arc_Node** result){
    
    if(!node) return;
    if(*result) return;
    node = node->parent;
    while(node){
        if(arc_has_property(node->parent, AP_LIST) && arc_has_property(node->parent, AP_AST) &&
           node->parent->ast_type == AST_SCOPE){
            if(result) *result = node;
            return;
        }
        if(node->first_child){
            find_inner_scope(node->first_child, result);
        }
        node = node->next_sibling;
    }
    return;
}

internal b32
find_inner_sub_node_of_list(Arc_Node* node, Arc_Node** result){
    Arc_Node* sub_node;
    assert(find_sub_node_of_list(node, &sub_node));
    return *result != 0;
}

internal b32
is_sub_node_of_list(Arc_Node* node, Arc_Node** result){
    if(!node) return false;
    node = node->parent;
    while(node){
        if(arc_has_property(node, AP_LIST)){
            if(result) *result = node;
            return true;
        }
        node = node->parent;
    }
    return false;
}

internal b32
is_node_sub_node_of_list(Arc_Node* node, Arc_Node* list){
    node = node->parent;
    while(node){
        if(node == list){
            return true;
        }
        node = node->parent;
    }
    return false;
}

internal b32
is_child_of_node(Arc_Node* node, Arc_Node* parent){
    node = node;
    while(node){
        if(node == parent){
            return true;
        }
        node = node->parent;
    }
    return false;
}

internal Arc_Node*
get_scope_of_node(Arc_Node* node){
    auto child = node->first_child;
    while(child){
        if(arc_has_property(child, AP_AST) && child->ast_type == AST_SCOPE){
            return child;
        }
        child = child->next_sibling;
    }
    assert(0);
    return nullptr;
}

internal void
append_empty_arc_node(Arc_Node* at, Pool* pool){
    auto node = make_selectable_arc_node(pool);
    arc_remove_property(node, AP_DELETABLE);
    insert_arc_node_as_sibling(at, node);
}

internal Arc_Node*
make_declaration_from_node(Arc_Node* decl, Pool* pool){
    
    arc_set_property(decl, AP_AST);
    decl->ast_type = AST_DECLARATION;
    
    auto type = make_arc_node(pool);
    auto expr = make_arc_node(pool);
    set_as_ast(expr, AST_EXPR);
    
    insert_arc_node_as_child(decl, type);
    insert_arc_node_as_sibling(type, expr);
    
    return decl;
}

internal b32
find_previous_reference(Arc_Node* at, Arc_Node** result){
    auto node = at;
    while(node){
        if(node->token_type == TOKEN_REFERENCE){
            *result = node;
            return true;
        }else if(node->token_type != TOKEN_ARRAY){
            return false;
        }
        node = node->prev_sibling;
    }
    *result = nullptr;
    return false;
}

internal Arc_Node*
make_for_from_node(Arc_Node* _for, Pool* pool){
    arc_set_property(_for, AP_AST);
    arc_set_property(_for, AP_CONTAINS_SCOPE);
    _for->ast_type = AST_FOR;
    
    auto init = make_arc_node(pool);
    auto cond = make_arc_node(pool);
    auto stmt = make_arc_node(pool);
    auto body = make_arc_node(pool);
    
    set_as_ast(cond, AST_EXPR);
    
    arc_set_property(body, AP_AST);
    arc_set_property(body, AP_LIST);
    body->ast_type = AST_SCOPE;
    body->ast_tag = AST_TAG_BODY;
    
    insert_arc_node_as_child(_for, init);
    insert_arc_node_as_sibling(init, cond);
    insert_arc_node_as_sibling(cond, stmt);
    insert_arc_node_as_sibling(stmt, body);
    
    return _for;
}

internal Arc_Node*
make_call_from_node(Arc_Node* call, Pool* pool){
    arc_set_property(call, AP_AST);
    call->ast_type = AST_CALL;
    
    auto args = make_arc_node(pool);
    arc_set_property(args, AP_LIST);
    arc_set_property(args, AP_AST);
    
    auto expr = make_arc_node(pool);
    arc_set_property(expr, AP_AST);
    expr->ast_type = AST_EXPR;
    expr->ast_tag = AST_TAG_ARGS;
    
    insert_arc_node_as_child(call, args);
    insert_arc_node_as_child(args, expr);
    
    return call;
}

internal Arc_Node*
make_struct_from_node(Arc_Node* _struct, Pool* pool){
    arc_set_property(_struct, AP_AST);
    _struct->ast_type = AST_STRUCT;
    
    auto members = make_arc_node(pool);
    arc_set_property(members, AP_AST);
    arc_set_property(members, AP_LIST);
    members->ast_type = AST_SCOPE;
    
    insert_arc_node_as_child(_struct, members);
    
    return _struct;
}

internal void
go_to_or_make_next(){
    
    // NOTE(Oliver): string is empty, we now want to exit the current edit and move
    // on
    if(can_advance_cursor(&presenter->cursor, CURSOR_RIGHT)){
        remove_arc_node_at(&presenter->cursor.at->parent->first_child, presenter->cursor.at);
        advance_cursor(&presenter->cursor, CURSOR_RIGHT);
    }else {
        auto next = make_selectable_arc_node(&editor->arc_pool);
        Arc_Node* list;
        if(is_sub_node_of_list(presenter->cursor.at, &list)){
            remove_arc_node_at(&presenter->cursor.at->parent->first_child, presenter->cursor.at);
            insert_arc_node_as_sibling(list->last_child, next);
            presenter->cursor.at = next;
        }
    }
}

internal b32
declaration_type_is_composite(Arc_Node* node){
    if(!node) return false;
    if(node->ast_type != AST_DECLARATION) return false;
    if(node->first_child->first_child->reference){
        return node->first_child->first_child->reference->ast_type == AST_STRUCT;
    }else {
        return false;
    }
}

internal b32
declaration_type_is_array(Arc_Node* node){
    if(!node) return false;
    assert(node->ast_type == AST_DECLARATION);
    return node->first_child->first_child->token_type == TOKEN_ARRAY;
}

internal void
remove_arc_node_at(Arc_Node** head, Arc_Node* at){
    if (!*head || !at) return;
    
    if (*head == at){
        *head = at->next_sibling;
    }
    
    if (at->next_sibling)
        at->next_sibling->prev_sibling = at->prev_sibling;
    
    if (at->prev_sibling)
        at->prev_sibling->next_sibling = at->next_sibling;
    
    memset(at, 0, sizeof(Arc_Node));
    pool_clear(&editor->arc_pool, at);
    return;
}

internal void
remove_sub_tree_at_helper(Arc_Node* at){
    if(!at) return;
    while(at) {
        //pool_clear(&editor->arc_pool, at);
        if(at->first_child){
            remove_sub_tree_at_helper(at->first_child);
        }
        at = at->next_sibling;
    }
}

internal void
remove_sub_tree_at(Arc_Node** head, Arc_Node* at){
    if(!head && !*head) return;
    if (!at) return;
    arc_set_property(at, AP_MARK_DELETE);
}



internal void
insert_arc_node_as_sibling(Arc_Node* at, Arc_Node* node){
    assert(at);
    assert(node);
    
    node->next_sibling = at->next_sibling;
    
    at->next_sibling = node;
    
    node->prev_sibling = at;
    
    if(node->next_sibling){
        node->next_sibling->prev_sibling = node;
    }else {
        if(at->parent){
            at->parent->last_child = node;
        }
    }
    
    node->parent = at->parent;
    
}

internal void
insert_arc_node_as_child(Arc_Node* at, Arc_Node* node){
    assert(at);
    assert(node);
    if(at->last_child){
        insert_arc_node_as_sibling(at->last_child, node);
    }else {
        at->first_child = node;
        at->last_child = node;
        node->parent = at;
    }
    
}

internal void
serialise(String8 filename, Arc_Node* root){
    if(!root) return;
    while(root){
        Arc_Node node = *root;
        node.string = {};
        node.prev_sibling = 0;
        node.next_sibling = 0;
        node.first_child = 0;
        node.last_child = 0;
        node.parent = 0;
        node.reference = 0;
        Serial_Node serial_node = {0, node};
        snprintf(serial_node.string, 256, "%.*s", 
                 root->string.length, root->string.text);
        platform->append_to_file(filename, &serial_node, sizeof(Serial_Node));
        serialise(filename, root->first_child);
        serial_node = { 1, {}};
        platform->append_to_file(filename, &serial_node, sizeof(Serial_Node));
        root = root->next_sibling;
    }
}

internal Arc_Node*
deserialise(String8 filename){
    void* file = nullptr;
    u64 length_in_bytes = 0;
    platform->load_file(&platform->frame_arena, filename, &file, &length_in_bytes);
    assert(length_in_bytes);
    Serial_Node* nodes = (Serial_Node*)file;
    assert(nodes);
    
    s64 index = 0;
    // NOTE(Oliver): another upper limit... maybe sort that out but it's probably not an issue
    // even for large files
    // also this does NOT need to be on the frame arena, switch to using a temp allocator
    
    Arc_Node** stack = (Arc_Node**)push_size(&platform->frame_arena, length_in_bytes);
    
    if(!index){
        Arc_Node* node = (Arc_Node*)pool_allocate(&editor->arc_pool);
        *node = nodes[0].node;
        node->string.text = (char*)pool_allocate(&editor->string_pool);
        int length = snprintf(node->string.text, 256, "%s", nodes[0].string);
        node->string.length = clampi(length, 0, 256);
        stack[index++] = node;
    }
    
    for(int i = 1; i < length_in_bytes/sizeof(Serial_Node); i++){
        if(nodes[i].marker){
            index--;
        }else {
            Arc_Node* node = (Arc_Node*)pool_allocate(&editor->arc_pool);
            *node = nodes[i].node;
            node->string.text = (char*)pool_allocate(&editor->string_pool);
            int length = snprintf(node->string.text, 256, "%s", nodes[i].string);
            node->string.length = clampi(length, 0, 256);
            Arc_Node* current = stack[index-1];
            insert_arc_node_as_child(current, node);
            stack[index++] = node;
        }
    }
    return stack[0];
}

internal void
fix_references(Arc_Node* node){
    
    while(node){
        if(arc_has_property(node, AP_AST) && (node->ast_type == AST_TOKEN || node->ast_type == AST_TYPE_TOKEN ||
                                              node->ast_type == AST_CALL)){
            if(node->ast_type == AST_CALL){
                set_token_type(node);
            }
            else if(node->token_type == TOKEN_REFERENCE){
                if(node->ast_type == AST_TOKEN){
                    set_token_type(node);
                }else {
                    set_type_token_type(node);
                }
            }
        }
        if(node->first_child){
            fix_references(node->first_child);
        }
        node = node->next_sibling;
    }
}
