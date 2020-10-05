
enum Node_Type {
    NODE_INVALID=-1,
    NODE_BINARY,
    NODE_UNARY,
    NODE_LITERAL,
    NODE_STRUCT,
    NODE_ENUM,
    NODE_UNION,
    NODE_SCOPE,
    NODE_TYPE_USAGE,
    NODE_DECLARATION,
    NODE_IDENTIFIER,
    NODE_FUNCTION,
    NODE_CONDITIONAL,
    NODE_LOOP,
    NODE_CALL,
    NODE_TOKEN,
    NODE_DUMMY,
};

enum Operator_Type {
    OP_PLUS,
    OP_MINUS,
    OP_DIVIDE,
    OP_MULTIPLY,
    OP_LT,
    OP_EQ,
    OP_GT,
    OP_LTE,
    OP_NEQ,
    OP_GTE,
};

enum Literal_Type {
    LIT_FLOAT,
    LIT_INTEGER,
    LIT_STRING,
};

enum Token_Type {
    TOKEN_MISC,
    TOKEN_REFERENCE,
    TOKEN_LITERAL,
};

struct Node {
    Node_Type type;
    
    // TODO(Oliver): strings should go next to a node!!!!
    // same lifetime!!!!!!!!!!!!!!!
    String8 name;
    
    Node* next = nullptr;
    Node* prev = nullptr;
    
    union {
        struct {
            Operator_Type op_type;
            Node* left;
            Node* right;
        }binary;
        
        struct {
            Node* operand;
        }unary;
        
        struct {
            Literal_Type lit_type;
            union {
                f32 _float;
                s32 _int;
            };
        } literal;
        
        struct {
            Node* members;
        }_struct;
        
        struct {
            Node* members;
        }_union;
        
        struct {
            Node* members;
        }_enum;
        
        struct {
            Node* return_type;
            Node* parameters;
            Node* scope;
        } function;
        
        struct {
            Node* expression;
            Node* type_usage;
            bool is_initialised = false;
        } declaration;
        
        struct {
            s64 number_of_pointers;
            Node* type_reference;
        } type_usage;
        
        struct {
            Node* statements;
            Node* outer;
        } scope;
        
        struct {
            Node* condition;
            Node* scope;
            Node* _else_if;
            Node* _else;
        } conditional;
        
        union {
            struct {
                Node* min;
                Node* max;
                Node* scope;
            };
        }loop;
        
        struct {
            Node* who_called_me;
            Node* arguments;
        } call;
        
        struct {
            Node* reference;
            Token_Type token_type;
        } token;
    };
};

global Node* _u8;
global Node* _u16;
global Node* _u32;
global Node* _u64;

// NOTE(Oliver): we assume all nodes have to be pool allocated
// but maybe that's not always true

internal Node*
make_node(Pool* pool, Node_Type type){
    Node* result = (Node*)pool_allocate(pool);
    result->type = type;
    return result;
}


internal Node*
make_node(Pool* pool, Node_Type type, char* name){
    Node* result = (Node*)pool_allocate(pool);
    u8* backing = (u8*)result + sizeof(Node);
    result->name = make_string(backing, name, 256);
    result->type = type;
    result->next = nullptr;
    result->prev = nullptr;
    return result;
}

internal Node*
make_dummy_node(Pool* pool){
    Node* result = make_node(pool, NODE_DUMMY);
    return result;
}

internal Node*
make_scope_node(Pool* pool, char* name){
    Node* result = make_node(pool, NODE_SCOPE, name);
    result->scope.statements = make_node(pool, NODE_DUMMY);
    return result;
}


internal Node*
make_type_usage_node(Pool* pool, char* name){
    Node* result = make_node(pool, NODE_TYPE_USAGE, name);
    result->type_usage.type_reference = _u16;
    result->type_usage.number_of_pointers = 0;
    return result;
}

internal Node*
make_function_node(Pool* pool, char* name){
    Node* result = make_node(pool, NODE_FUNCTION, name);
    result->function.parameters = make_dummy_node(pool);
    result->function.scope = make_scope_node(pool, "scope");
    result->function.scope->next = make_dummy_node(pool);
    result->function.return_type = make_type_usage_node(pool, name);
    return result;
}

internal Node*
make_token_node(Pool* pool, char* name){
    Node* result = make_node(pool, NODE_TOKEN, name);
    result->token.reference = nullptr;
    return result;
}

internal Node*
make_token_misc_node(Pool* pool, char* name){
    Node* result = make_token_node(pool, name);
    result->token.token_type = TOKEN_MISC;
    return result;
}

internal Node*
make_token_literal_node(Pool* pool, char* name){
    Node* result = make_token_node(pool, name);
    result->token.token_type = TOKEN_LITERAL;
    return result;
}

internal Node*
make_token_reference_node(Pool* pool, char* name){
    Node* result = make_token_node(pool, name);
    result->token.token_type = TOKEN_REFERENCE;
    return result;
}

internal Node*
make_struct_node(Pool* pool, char* name){
    Node* result = make_node(pool, NODE_STRUCT, name);
    result->_struct.members = make_dummy_node(pool);
    return result;
}

internal Node*
make_declaration_node(Pool* pool, char* name){
    Node* result = make_node(pool, NODE_DECLARATION, name);
    result->declaration.type_usage = make_type_usage_node(pool, name);
    return result;
}

internal Node*
make_literal_node(Pool* pool, int value){
    Node* result = make_node(pool, NODE_LITERAL, "literal");
    result->literal.lit_type = LIT_INTEGER;
    result->literal._int = value;
    return result;
}

internal Node*
make_loop_node(Pool* pool, char* name){
    Node* result = make_node(pool, NODE_LOOP, name);
    result->loop.min = make_literal_node(pool, 0);
    result->loop.max = make_literal_node(pool, 10);
    result->loop.scope = make_scope_node(pool, name);
    return result;
}

internal Node*
make_binary_node(Pool* pool, char* name){
    Node* result = make_node(pool, NODE_BINARY, name);
    
    return result;
}

internal Node*
make_conditional_node(Pool* pool, char* name){
    Node* result = make_node(pool, NODE_CONDITIONAL, name);
    result->conditional.condition = make_binary_node(pool, "expr");
    auto bin = result->conditional.condition;
    bin->binary.left = make_literal_node(pool, 20);
    bin->binary.right = make_literal_node(pool, 9);
    bin->binary.op_type = OP_GTE;
    
    result->conditional.scope = make_scope_node(pool, name);
    result->conditional._else_if = nullptr;
    result->conditional._else = nullptr;
    
    return result;
}

internal void
insert_node_at(Node* node, Node* at){
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
remove_node_at(Node* at){
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