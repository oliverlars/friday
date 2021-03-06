

enum Cursor_Direction {
    CURSOR_NONE,
    CURSOR_UP,
    CURSOR_DOWN,
    CURSOR_LEFT,
    CURSOR_RIGHT,
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
    v2f v0;
    v2f v1;
    v2f v2;
    Cursor_Direction direction;
    int direction_count;
};

struct Presenter_State {
    
    
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
    
    Arc_Node** delete_queue;
    int delete_queue_size;
    
    v4f select_first_rect;
    v4f select_second_rect;
    
    f32 indent_level;
    
    // HACK(Oliver): this is because we don't have the widget ids
    // when setting the next cursor pos, maybe this can be changed
    // but for now this'll do
    b32 find_next_text_id;
    
};
global Arc_Node* highlight_reference;


Presenter_State* presenter;

internal String8 
find_matching_reference_in_composite(Arc_Node* node, b32* found);
