typedef u64 UI_ID;

struct Theme {
    Colour background;
    Colour text;
    
    Colour sub_colour;
    Colour border;
    
    Colour text_comment;
    Colour text_function;
    Colour text_type;
    Colour text_literal;
    Colour text_misc;
    
    Colour cursor;
    
    Colour error;
    
};

#define PROPERTIES_MAX 256

enum Widget_Property {
    WP_RENDER_TEXT,
    WP_RENDER_TRIANGLE,
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
    WP_FIXED_SIZE,
    WP_RENDER_HOOK,
    
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
    
    void (*render_hook)(Widget* widget);
};

struct Widget_Update {
    
    b32 clicked;
};

struct Layout {
    Layout* prev;
    Widget* widget;
};

enum Panel_Split_Type {
    PANEL_SPLIT_VERTICAL,
    PANEL_SPLIT_HORIZONTAL,
};

enum Panel_Type {
    PANEL_EDITOR,
    PANEL_PROPERTIES,
};

struct Panel {
    Panel_Split_Type split_type;
    Panel_Type type;
    f32 split_ratio;
    Panel* first; 
    Panel* second;
    Panel* parent;
    
    Widget* widgets[2];
    
};


struct UI_State {
    UI_ID hover_id;
    UI_ID clicked_id;
    
    
    bool widget_frame;
    
    Theme theme;
    
    f32 round_amount;
    
    Layout* layout_stack;
    
    
    Widget* root;
};

typedef u64 UI_ID;

#define MAX_WIDGETS 1024
