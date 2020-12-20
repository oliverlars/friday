
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
