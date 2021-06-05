

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

struct Cursor {
    String8* string;
    Arc_Node* at;
    UI_ID text_id;
    
    int line_index;
    int buffer_index;
    
    v2f pos;
    
    Cursor_Direction direction;
    int direction_count;
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
    
    Arc_Node* select_first;
    Arc_Node* select_second;
    
    v2f select_top_left;
    f32 select_furthest_right;
    f32 select_height;
    
    int pos;
    int start_pos;
    int end_pos;
    
    union {
        struct {
            Cursor last_cursor;
            Cursor cursor;
            Cursor select_start;
            Cursor select_end;
        };
        Cursor cursors[4];
    };
    
};
global Arc_Node* highlight_reference;


Presenter_State* presenter;