

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
};

struct Line_Info {
    int start;
    int end;
};

struct Presenter_State {
    
    Present_Mode mode;
    
    Present_Node* buffer;
    int buffer_pos;
    int buffer_index;
    
    Line_Info* lines;
    int line_pos;
    int line_index;
    
    int number_of_deletions;
    int number_of_deletions_before_cursor;
    int number_of_deletions_after_cursor;
    
    Cursor_Direction direction;
    int direction_count;
    
    Arc_Node* select_first;
    Arc_Node* select_second;
    
    v2f select_top_left;
    f32 select_furthest_right;
    f32 select_height;
    
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
global Cursor mark;

Presenter_State* presenter;