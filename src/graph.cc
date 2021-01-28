
global Ast_Node* _u8;
global Ast_Node* _u16;
global Ast_Node* _u32;
global Ast_Node* _u64;

global Ast_Node* _s8;
global Ast_Node* _s16;
global Ast_Node* _s32;
global Ast_Node* _s64;

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

internal Ast_Node*
make_node(Pool* pool, Ast_Type type){
    Ast_Node* result = (Ast_Node*)pool_allocate(pool);
    result->type = type;
    return result;
}

internal Ast_Node*
make_dummy_node(Pool* pool){
    return make_node(pool, AST_DUMMY);
}

internal void
remove_graph_name(String8 string){
    pool_clear(&editor->string_pool, string.text);
}

internal Ast_Node*
make_node(Pool* pool, Ast_Type type, char* name){
    Ast_Node* result = (Ast_Node*)pool_allocate(pool);
    result->name = make_stringf(&editor->string_pool, "%s", name);
    result->type = type;
    result->next = nullptr;
    result->prev = nullptr;
    return result;
}

internal Ast_Node*
make_scope_node(Pool* pool){
    Ast_Node* result = make_node(pool, AST_SCOPE);
    result->scope.statements = make_dummy_node(pool);
    return result;
}


internal Ast_Node*
make_type_usage_node(Pool* pool){
    Ast_Node* result = make_node(pool, AST_TYPE_USAGE);
    result->type_usage.type_reference = _s64;
    result->type_usage.number_of_pointers = 0;
    return result;
}

internal Ast_Node*
make_function_node(Pool* pool, char* name){
    Ast_Node* result = make_node(pool, AST_FUNCTION, name);
    result->function.scope = make_scope_node(pool);
    result->function.parameters = make_dummy_node(pool);
    result->function.return_type = make_dummy_node(pool);
    return result;
}

internal Ast_Node*
make_token_node(Pool* pool, char* name){
    Ast_Node* result = make_node(pool, AST_TOKEN, name);
    result->token.reference = nullptr;
    return result;
}

internal Ast_Node*
make_token_misc_node(Pool* pool, char* name){
    Ast_Node* result = make_token_node(pool, name);
    result->token.token_type = TOKEN_MISC;
    return result;
}

internal Ast_Node*
make_token_literal_node(Pool* pool, char* name){
    Ast_Node* result = make_token_node(pool, name);
    result->token.token_type = TOKEN_LITERAL;
    return result;
}

internal Ast_Node*
make_token_reference_node(Pool* pool, char* name){
    Ast_Node* result = make_token_node(pool, name);
    result->token.token_type = TOKEN_REFERENCE;
    return result;
}

internal Ast_Node*
make_struct_node(Pool* pool, char* name){
    Ast_Node* result = make_node(pool, AST_STRUCT, name);
    return result;
}

internal Ast_Node*
make_declaration_node(Pool* pool, char* name){
    Ast_Node* result = make_node(pool, AST_DECLARATION, name);
    result->declaration.type_usage = make_type_usage_node(pool);
    return result;
}

internal Ast_Node*
make_literal_node(Pool* pool, int value){
    Ast_Node* result = make_node(pool, AST_LITERAL);
    result->literal.lit_type = LIT_INTEGER;
    result->literal._int = value;
    return result;
}

internal Ast_Node*
make_loop_node(Pool* pool, char* name){
    Ast_Node* result = make_node(pool, AST_LOOP, name);
    result->loop.min = make_literal_node(pool, 0);
    result->loop.max = make_literal_node(pool, 10);
    result->loop.scope = make_scope_node(pool);
    return result;
}

internal Ast_Node*
make_binary_node(Pool* pool){
    Ast_Node* result = make_node(pool, AST_BINARY);
    
    return result;
}

internal Ast_Node*
make_conditional_node(Pool* pool){
    Ast_Node* result = make_node(pool, AST_CONDITIONAL);
    result->conditional.condition = make_binary_node(pool);
    auto bin = result->conditional.condition;
    bin->binary.left = make_literal_node(pool, 20);
    bin->binary.right = make_literal_node(pool, 9);
    bin->binary.op_type = OP_GTE;
    
    result->conditional.scope = make_scope_node(pool);
    result->conditional._else_if = nullptr;
    result->conditional._else = nullptr;
    
    return result;
}

internal Arc_Node*
make_arc_node(Pool* pool){
    auto result = (Arc_Node*)pool_allocate(pool);
    result->string.text = (char*)pool_allocate(&editor->string_pool);
    arc_set_property(result, AP_SELECTABLE);
    return result;
}

internal void
insert_node_at(Ast_Node* node, Ast_Node* at){
    if(!at) return;
    if(at->next){
        node->next = at->next;
        at->next = node;
        node->prev = at;
        
        if(node->next){
            node->next->prev = node;
        }
    }else {
        at->next = node;
        node->prev = at;
        node->next = nullptr;
    }
    
}

internal void
remove_node_at(Ast_Node* at){
    if(!at) return;
    
    if(!at->prev && at->next){
        at->next->prev = nullptr;
    }
    
    if(at->next){
        at->next->prev = at->prev;
    }
    
    if(at->prev){
        at->prev->next = at->next;
    }else {
        
    }
    
}

internal void
serialise_to_disk_helper(FILE* file, Ast_Node* node){
    
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
        }break;
        case AST_TYPE_USAGE:{
        }break;
        case AST_DECLARATION:{
            
        }break;
        case AST_IDENTIFIER:{
        }break;
        case AST_FUNCTION:{
        }break;
        case AST_CONDITIONAL:{
        }break;
        case AST_LOOP:{
        }break;
        case AST_CALL:{
        }break;
        case AST_TOKEN:{
        }break;
        case AST_DUMMY:{
            serialise_to_disk_helper(file, node->next);
        }break;
    }
    
}

internal void
serialise_to_disk(Ast_Node* node){
    FILE* file = fopen("main.arc", "wb");
    u64 version = 1;
    fwrite(&version, sizeof(u64), 1, file);
    serialise_to_disk_helper(file, node);
    fclose(file);
}

internal void
insert_arc_node_as_sibling(Arc_Node* at, Arc_Node* node){
    assert(at);
    assert(node);
    assert(at->next_sibling);
    assert(at->prev_sibling);
    auto next_sibling = at->next_sibling;
    at->next_sibling = node;
    node->prev_sibling = at;
    node->next_sibling = next_sibling;
    node->parent = at->parent;
    next_sibling->prev_sibling = node;
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
