

enum Cursor_Direction {
    CURSOR_UP,
    CURSOR_DOWN,
    CURSOR_LEFT,
    CURSOR_RIGHT,
};

enum Present_Mode {
    PRESENT_EDIT,
    PRESENT_CREATE,
    PRESENT_EDIT_TYPE,
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
};

struct Cursor {
    Present_Node* at;
    String8* string;
    Arc_Node* arc;
    v2f pos;
};

global Cursor cursor;

Presenter_State* presenter;