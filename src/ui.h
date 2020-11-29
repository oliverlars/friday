typedef u64 UI_ID;

struct Theme {
    Colour background;
    Colour panel;
    
    Colour tab;
    Colour tab_pressed;
    
    Colour icon;
    
    Colour button_highlight;
    
    Colour menu;
    
    Colour text;
    
    Colour text_comment;
    Colour text_function;
    Colour text_type;
    Colour text_literal;
    Colour text_misc;
    
    Colour cursor;
    
    Colour error;
    
    Colour view_button;
};

#define PROPERTIES_MAX 256

enum Widget_Property {
    WP_RENDER_TEXT,
    WP_RENDER_BORDER,
    WP_RENDER_BACKGROUND,
    WP_CLICKABLE,
    WP_ROW,
    WP_COLUMN,
    WP_WINDOW,
    WP_WIDTHFILL,
    WP_HEIGHTFILL,
    WP_PADDING,
    WP_CLIP,
};

struct Widget {
    UI_ID id;
    String8 string;
    
    Widget* prev_sibling;
    Widget* next_sibling;
    Widget* first_child;
    Widget* last_child;
    Widget* parent;
    
    v2f min;
    v2f max;
    v2f pos;
    u64 properties[PROPERTIES_MAX/64 + 1];
};

struct Widget_Update {
    
    b32 clicked;
};

struct Layout {
    Layout* prev;
    Widget* widget;
};



struct UI_State {
    UI_ID hover_id;
    UI_ID clicked_id;
    
    Arena frame_arena;
    
    Arena widget_arena[2];
    bool widget_frame;
    
    Widget* widgets[2];
    
    Theme theme;
    
    f32 round_amount;
    
    Layout* layout_stack;
    Widget* root;
    
    Pool widget_pool;
};

typedef u64 UI_ID;
