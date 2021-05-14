

enum Cursor_Direction {
    CURSOR_UP,
    CURSOR_DOWN,
    CURSOR_LEFT,
    CURSOR_RIGHT,
};

enum Present_Mode {
    P_EDIT,
    P_CREATE,
};

enum Present_Context {
    PC_PARAMS,
    PC_TYPE,
    PC_EXPR,
    PC_NAME,
};

// NOTE(Oliver): this helps us traverse the editable strings easier
struct Present_Node {
    Present_Node* next_hash;
    
    Present_Node* parent;
    Present_Node* next_sibling;
    Present_Node* prev_sibling;
    Present_Node* child;
    
    String8 string;
    UI_ID id;
    Ast_Node* node;
};

struct Presenter_State {
    
    Present_Node* lines;
    Present_Node* last_lines;
    
    Present_Node* line;
    
    Present_Node** table;
    Present_Node** last_table;
    
    Present_Mode mode;
    Present_Context context;
    
    
};
global Arc_Node* highlight_reference;

struct Cursor {
    Present_Node* at;
    String8* string;
    Arc_Node* arc;
    v2f pos;
};

global Cursor cursor;

Presenter_State* presenter;