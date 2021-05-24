
enum Ast_Type {
    AST_INVALID=0,
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
    AST_IF,
    AST_LOOP,
    AST_CALL,
    AST_TOKEN,
    AST_DUMMY,
    AST_EXPR,
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


enum Arc_Property {
    AP_AST,
    AP_AST_TAG,
    AP_SELECTABLE,
};

#define ARC_PROPERTIES_MAX 256

enum Ast_Tag {
    AT_INVALID, 
    AT_PARAMS,
    AT_RETURN_TYPE,
    AT_BODY,
};

struct Arc_Node {
    
    String8 string;
    
    Arc_Node* prev_sibling;
    Arc_Node* next_sibling;
    Arc_Node* first_child;
    Arc_Node* last_child;
    Arc_Node* parent;
    
    u64 properties[(ARC_PROPERTIES_MAX+63)/64];
    
    Arc_Node* reference;
    Ast_Type ast_type;
    Ast_Tag ast_tag;
    Token_Type token_type;
    bool is_initialised;
    s64 number_of_pointers;
};

internal void remove_arc_node_at(Arc_Node** head, Arc_Node* at);

internal bool
arc_has_property(Arc_Node* arc, Arc_Property property);

internal void
arc_set_property(Arc_Node* arc, Arc_Property property);

internal void
arc_remove_property(Arc_Node* arc, Arc_Property property);


internal void
insert_arc_node_as_sibling(Arc_Node* at, Arc_Node* node);

internal b32
is_sub_node_of_ast_type(Arc_Node* node, Ast_Type type, Arc_Node** result = nullptr);

internal b32
is_direct_sub_node_of_ast_type(Arc_Node* node, Ast_Type type, Arc_Node** result = nullptr);

internal void
insert_arc_node_as_child(Arc_Node* at, Arc_Node* node);