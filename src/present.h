

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
    
    Cursor_Direction direction;
};
global Arc_Node* highlight_reference;

struct Cursor {
    String8* string;
    Arc_Node* at;
    v2f pos;
};

global Cursor cursor;

Presenter_State* presenter;