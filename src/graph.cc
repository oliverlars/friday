
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
make_assignment_from_node(Arc_Node* assign, Pool* pool){
    arc_set_property(assign, AP_AST);
    assign->ast_type = AST_ASSIGNMENT;
    
    auto expr = make_arc_node(pool);
    arc_set_property(expr, AP_AST);
    expr->ast_type = AST_EXPR;
    
    insert_arc_node_as_child(assign, expr);
    
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

internal Arc_Node*
make_declaration_from_node(Arc_Node* decl, Pool* pool){
    arc_set_property(decl, AP_AST);
    decl->ast_type = AST_DECLARATION;
    
    auto type = make_arc_node(pool);
    auto expr = make_arc_node(pool);
    arc_set_property(expr, AP_AST);
    expr->ast_type = AST_EXPR;
    
    insert_arc_node_as_child(decl, type);
    insert_arc_node_as_sibling(type, expr);
    
    return decl;
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
set_as_ast(Arc_Node* node, Ast_Type type){
    arc_set_property(node, AP_AST);
    node->ast_type = type;
}

internal void
go_to_or_make_next(){
    
    // NOTE(Oliver): string is empty, we now want to exit the current edit and move
    // on
    if(can_advance_cursor(CURSOR_RIGHT)){
        remove_arc_node_at(&cursor.at->parent->first_child, cursor.at);
        advance_cursor(CURSOR_RIGHT);
    }else {
        auto next = make_selectable_arc_node(&editor->arc_pool);
        Arc_Node* list;
        if(is_sub_node_of_list(cursor.at, &list)){
            remove_arc_node_at(&cursor.at->parent->first_child, cursor.at);
            insert_arc_node_as_sibling(list->last_child, next);
            cursor.at = next;
        }
    }
}

internal b32
declaration_type_is_composite(Arc_Node* node){
    if(!node) return false;
    assert(node->ast_type == AST_DECLARATION);
    return node->first_child->first_child->reference->ast_type == AST_STRUCT;
}

internal void
remove_arc_node_at(Arc_Node** head, Arc_Node* at){
    if (!*head || !at) return;
    
    if (*head == at){
        *head = at->next_sibling;
    }
    
    if(at->parent){
        if(at->parent->first_child == at){
            at->parent->first_child = at->next_sibling;
        }
        if(at->parent->last_child == at){
            at->parent->last_child = at->prev_sibling;
        }
    }
    
    if (at->next_sibling)
        at->next_sibling->prev_sibling = at->prev_sibling;
    
    if (at->prev_sibling)
        at->prev_sibling->next_sibling = at->next_sibling;
    
    
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
