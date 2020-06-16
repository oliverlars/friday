
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
    NODE_CALL,
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
    
    String8 name;
    
    Node* next = nullptr;
    Node* previous = nullptr;
    
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
            Node* declaration;
            Node* type_usage;
        } declaration;
        
        struct {
            s64 number_of_pointers;
            Node* type_reference;
        } type_usage;
        
        struct {
            Node* statements;
        } scope;
        
        struct {
            Node* who_called_me;
            Node* arguments;
        } call;
    };
};
// NOTE(Oliver): we assume all nodes have to be pool allocated
// but maybe that's not always true

Node*
make_node(Pool* pool, Node_Type type){
    Node* result = (Node*)pool_allocate(pool);
    result->type = type;
    return result;
}