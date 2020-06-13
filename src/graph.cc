
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
    
    // HACK(Oliver)
    char* name;
    u8 name_length;
    
    Node* next = nullptr;
    Node* previous = nullptr;
};

struct Node_Binary : Node {
    Node_Binary() { type = NODE_BINARY; }
    
    Operator_Type op_type;
    
    Node* left;
    Node* right;
};

struct Node_Unary : Node {
    Node_Unary() { type = NODE_UNARY; }
    
    Node* operand;
};

struct Node_Literal : Node {
    Node_Literal() { type = NODE_LITERAL; }
    
    Literal_Type lit_type;
    union {
        f32 _float; 
        s32 _int;
    };
};

struct Node_Struct : Node {
    Node_Struct() { type = NODE_STRUCT; }
    
    Node* members;
};

struct Node_Union : Node {
    Node_Union() { type = NODE_UNION; }
    
    Node* members;
};

struct Node_Enum : Node {
    Node_Enum() { type = NODE_ENUM; }
    
    Node* members;
};

struct Node_Function : Node {
    Node_Function() { type = NODE_FUNCTION; }
    
    Node* return_type;
    Node* parameters;
    Node* scope;
};

struct Node_Declaration : Node {
    Node_Declaration() { type = NODE_DECLARATION; }
    
    Node* declaration;
};

struct Node_Type_Usage : Node {
    Node_Type_Usage() { type = NODE_TYPE_USAGE; }
    
    s64 number_of_pointers;
    Node* type_reference;
};

struct Node_Scope : Node {
    Node_Scope() { type = NODE_SCOPE; }
    
    Node* statements;
};

struct Node_Call : Node {
    Node_Call() { type = NODE_CALL; }
    
    Node* who_called_me;
    Node* arguments;
};
