
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
make_declaration(Pool* pool){
    auto decl = make_arc_node(pool);
    arc_set_property(decl, AP_AST);
    decl->ast_type = AST_DECLARATION;
    
    auto type = make_arc_node(pool);
    auto expr = make_arc_node(pool);
    
    insert_arc_node_as_child(decl, type);
    insert_arc_node_as_sibling(type, expr);
    
    return decl;
}

internal Arc_Node*
make_declaration_from_node(Arc_Node* decl, Pool* pool){
    arc_set_property(decl, AP_AST);
    decl->ast_type = AST_DECLARATION;
    
    auto type = make_arc_node(pool);
    auto expr = make_arc_node(pool);
    
    insert_arc_node_as_child(decl, type);
    insert_arc_node_as_sibling(type, expr);
    
    return decl;
}

internal void
remove_arc_node_at(Arc_Node** head, Arc_Node* at){
    if (!*head || !at) return;
    
    if (*head == at)
        *head = at->next_sibling;
    
    if (at->next_sibling)
        at->next_sibling->prev_sibling = at->prev_sibling;
    
    if (at->prev_sibling)
        at->prev_sibling->next_sibling = at->next_sibling;
    
    pool_clear(&editor->arc_pool, at);
    return;
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
    }
    
}

internal Arc_Node*
make_arc_node_child(Arc_Node* at, Arc_Node* block){
    if(at->last_child){
        at->last_child->next_sibling = block;
        block->prev_sibling = at->last_child;
        block->parent = at;
        at->last_child = block;
    }else {
        at->last_child = block;
        at->first_child = block;
        block->parent = at;
    }
    return block;
}
internal Arc_Node*
make_sibling_arc_node_after(Arc_Node* at, Arc_Node* block){
    
    block->next_sibling = at->next_sibling;
    
    at->next_sibling = block;
    
    block->prev_sibling = at;
    
    if (block->next_sibling){
        block->next_sibling->prev_sibling = block;
    }
    
    block->parent = at->parent;
    return block;
}


internal void
insert_arc_node_as_child(Arc_Node* at, Arc_Node* node){
    assert(at);
    assert(node);
    if(at->last_child){
        auto last_child = at->last_child;
        at->last_child = node;
        node->parent = at;
        node->prev_sibling = last_child;
        last_child->next_sibling = node;
    }else {
        at->first_child = node;
        at->last_child = node;
        node->parent = at;
    }
}
