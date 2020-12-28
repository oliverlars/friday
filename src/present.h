

enum Cursor_Direction {
    CURSOR_UP,
    CURSOR_DOWN,
    CURSOR_LEFT,
    CURSOR_RIGHT,
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
};

struct Presenter_State {
    Present_Node* lines;
    Present_Node* last_lines;
    
    Present_Node* line;
    
    Present_Node** table;
    Present_Node** last_table;
};

struct Cursor {
    Present_Node* at;
    String8* string;
};

global Cursor cursor;

Presenter_State* presenter;