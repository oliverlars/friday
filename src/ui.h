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
    WP_SPACING,
    WP_CUSTOM_DATA,
};

struct Widget {
    UI_ID id;
    String8 string;
    
    Widget* next_hash;
    
    Widget* prev_sibling;
    Widget* next_sibling;
    Widget* first_child;
    Widget* last_child;
    Widget* parent;
    
    v2f min;
    v2f max;
    v2f pos;
    u64 properties[PROPERTIES_MAX/64 + 1];
    
    f32 hot_transition;
    f32 active_transition;
    
    f32 font_scale;
    
    b32 checked;
    
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
    PANEL_STATUS,
};

struct Panel {
    Panel_Split_Type split_type;
    Panel_Type type;
    f32 split_ratio;
    Panel* first; 
    Panel* second;
    Panel* parent;
    
};

#define MAX_WIDGETS 4096

struct UI_State {
    UI_ID hover_id;
    UI_ID clicked_id;
    
    
    bool widget_frame;
    
    Theme theme;
    
    f32 round_amount;
    
    Layout* layout_stack;
    
    UI_ID hot;
    UI_ID active;
    
    Widget* root;
    
    Widget* widget_table[MAX_WIDGETS];
    Pool widget_pool;
    
    Panel* panel;
};

typedef u64 UI_ID;


#define UI_ROW defer_loop(push_widget_row(), pop_layout())
#define UI_COLUMN defer_loop(push_widget_column(), pop_layout())
#define UI_WINDOW(rect, titlebar, text) defer_loop(ui_window(rect, titlebar, text), pop_widget_window()) 
#define UI_WIDTHFILL defer_loop(push_widget_widthfill(), pop_layout())
#define UI_HEIGHTFILL defer_loop(push_widget_heightfill(), pop_layout())
#define UI_PAD(p) defer_loop(push_widget_padding(p), pop_layout())

#define ForEachWidgetChild(w) for(auto it = w->first_child; it; it = it->next_sibling)
#define ForEachWidgetSibling(w) for(auto it = w; it; it = it->next_sibling)
