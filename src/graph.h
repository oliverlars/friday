
enum Ast_Type {
    AST_INVALID=-1,
    AST_BINARY,
    AST_UNARY,
    AST_LITERAL,
    AST_STRUCT,
    AST_ENUM,
    AST_UNION,
    AST_SCOPE,
    AST_TYPE_USAGE,
    AST_DECLARATION,
    AST_IDENTIFIER,
    AST_FUNCTION,
    AST_CONDITIONAL,
    AST_LOOP,
    AST_CALL,
    AST_TOKEN,
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

struct Ast_Node {
    Ast_Type type;
    
    String8 name;
    
    Ast_Node* next = nullptr;
    Ast_Node* prev = nullptr;
    
    union {
        struct {
            Operator_Type op_type;
            Ast_Node* left;
            Ast_Node* right;
        }binary;
        
        struct {
            Ast_Node* operand;
        }unary;
        
        struct {
            Literal_Type lit_type;
            union {
                f32 _float;
                s32 _int;
            };
        } literal;
        
        struct {
            Ast_Node* members;
        }_struct;
        
        struct {
            Ast_Node* members;
        }_union;
        
        struct {
            Ast_Node* members;
        }_enum;
        
        struct {
            Ast_Node* return_type;
            Ast_Node* parameters;
            Ast_Node* scope;
        } function;
        
        struct {
            Ast_Node* expression;
            Ast_Node* type_usage;
            bool is_initialised = false;
        } declaration;
        
        struct {
            s64 number_of_pointers;
            Ast_Node* type_reference;
        } type_usage;
        
        struct {
            Ast_Node* statements;
            Ast_Node* outer;
        } scope;
        
        struct {
            Ast_Node* condition;
            Ast_Node* scope;
            Ast_Node* _else_if;
            Ast_Node* _else;
        } conditional;
        
        union {
            struct {
                Ast_Node* min;
                Ast_Node* max;
                Ast_Node* scope;
            };
        }loop;
        
        struct {
            Ast_Node* who_called_me;
            Ast_Node* arguments;
        } call;
        
        struct {
            Ast_Node* reference;
            Token_Type token_type;
        } token;
    };
};
