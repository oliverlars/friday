
enum Node_Type {
    NODE_INVALID=-1,
    NODE_BINARY,
    NODE_UNARY,
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


struct Node {
    Node_Type type;
    
    Node* next;
    Node* previous;
};

struct Node_Binary : Node {
    Node_Binary() { type = NODE_BINARY; }
    
    Node* left;
    Node* right;
};

struct Node_Unary : Node {
    Node_Unary() { type = NODE_UNARY; }
    
    Node* operand;
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
    
    s64 number_of_pointers;
    Node* type_reference;
};

struct Node_Call : Node {
    Node_Call() { type = NODE_CALL; }
    
    Node* who_called_me;
    Node* arguments;
};
