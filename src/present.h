

enum Cursor_Direction {
    CURSOR_NONE,
    CURSOR_UP,
    CURSOR_DOWN,
    CURSOR_LEFT,
    CURSOR_RIGHT,
};

enum Present_Mode {
    P_EDIT,
    P_CREATE,
};

struct Present_Node {
    Arc_Node* node;
    bool newline;
};

struct Presenter_State {
    
    Present_Mode mode;
    
    Present_Node* buffer;
    int buffer_pos;
    int buffer_index;
    
    Cursor_Direction direction;
};
global Arc_Node* highlight_reference;

struct Cursor {
    String8* string;
    Arc_Node* at;
    UI_ID text_id;
    v2f pos;
};

global Cursor last_cursor;
global Cursor cursor;

Presenter_State* presenter;