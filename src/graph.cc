
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
    NODE_DUMMY,
};

enum Operator_Type {
    OP_PLUS,
    OP_MINUS,
    OP_DIVIDE,
    OP_MULTIPLY,
};

enum Literal_Type {
    LIT_FLOAT,
    LIT_INTEGER,
    LIT_STRING,
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
        } scope;
        
        struct {
            Node* condition;
            Node* scope;
            Node* else_if;
            Node* _else;
        } conditional;
        
        struct {
            Node* init;
            Node* condition;
            Node* inc;
            Node* scope;
        }_for;
        
        struct {
            Node* who_called_me;
            Node* arguments;
        } call;
        
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
make_function_node(Pool* pool, char* name){
    Node* result = make_node(pool, NODE_FUNCTION, name);
    result->function.parameters = make_dummy_node(pool);
    result->function.scope = make_scope_node(pool, "scope");
    result->function.scope->next = make_dummy_node(pool);
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
    result->declaration.type_usage = _u8;
    
    return result;
}