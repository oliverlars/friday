typedef u64 UI_ID;

struct Theme {
    Colour background;
    Colour darker_background;
    Colour text;
    
    Colour sub_colour;
    Colour border;
    
    Colour select;
    
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
    WP_RENDER_TEXT_SHADOW,
    WP_RENDER_BORDER,
    WP_RENDER_CORNERS,
    WP_RENDER_DOUBLE_BORDER,
    WP_RENDER_BACKGROUND,
    WP_RENDER_UNDERLINE,
    WP_HOVER_RENDER_BACKGROUND,
    WP_HOVER_RENDER_BORDER,
    WP_HOVER_INFLATE,
    WP_LERP_POSITION,
    WP_LERP_COLOURS,
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
    WP_TEXT_EDIT,
    WP_CONTAINER,
    WP_DRAGGABLE,
    WP_WRAP,
    WP_SCROLLING,
    WP_MANUAL_LAYOUT,
    WP_ALT_STRING,
    WP_FIRST_TRANSITION,
    WP_GHOST_LAYOUT,
    WP_ON_TOP,
    WP_OVERLAP,
    WP_FIT_TO_CHILDREN,
};

struct Widget_Style {
    v4f text_colour;
    v4f border_colour;
    v4f background_colour;
    f32 font_scale;
    f32 rounded_corner_amount;
};


struct Arc_Node;

struct Widget {
    UI_ID id;
    String8 string;
    String8 alt_string;
    
    Widget* next_hash;
    
    Widget* prev_sibling;
    Widget* next_sibling;
    Widget* first_child;
    Widget* last_child;
    Widget* parent;
    
    u64 properties[(PROPERTIES_MAX-63)/64];
    
    Arc_Node* arc;
    int present_pos;
    
    v2f min;
    v2f max;
    v2f pos;
    f32 hot_transition;
    f32 active_transition;
    f32 first_transition;
    
    b32 checked;
    f32 scroll_amount;
    f32 value;
    b32 dont_lerp_children;
    
    b32 dragging;
    
    Widget_Style style;
    
    void (*render_hook)(Widget* widget);
};

struct Widget_Update {
    b32 clicked;
    b32 left_dragged;
    b32 middle_dragged;
    b32 hovered;
    v2f delta;
    v2f clicked_position;
    v2f pos;
    v2f size;
    
    b32 was_active;
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
    PANEL_DEBUG,
    PANEL_CONSOLE,
    PANEL_HEADER,
};

struct Panel {
    b32 is_dragging;
    Panel_Split_Type split_type;
    Panel_Type type;
    f32 split_ratio;
    Panel* first;
    Panel* second;
    Panel* parent;
};

#define MAX_WIDGETS 4096
#define MAX_TABLE_WIDGETS 2*MAX_WIDGETS

global s64 number_of_widgets;


struct ID_Node {
    UI_ID id;
    ID_Node* prev;
};

struct Style_Node {
    Widget_Style style;
    Style_Node* prev;
};

struct UI_State {
    
    Theme theme;
    
    
    f32 round_amount;
    
    Layout* layout_stack;
    ID_Node* id_stack;
    Style_Node* style_stack;
    
    UI_ID hot;
    UI_ID active;
    
    Widget* root;
    
    s32 current_window;
    
    Widget** widget_table;
    Widget** last_widget_table;
    
    Panel* panel;
    
    Widget** previous_windows;
    int previous_window_count;
    
    Widget** windows;
    int window_count;
    
    b32 popup;
    v4f popup_rect;
    Panel* popup_panel;
    
    struct {
        int capacity;
        
        union{
            String8 string;
            struct{
                int length;
                char* text;
            };
        };
    } editing_string;
    
    int cursor_pos;
    
    v2f drag_pos;
};

global f32 time_per_gui_update;

typedef u64 UI_ID;


#define UI_GHOST defer_loop(push_widget_ghost(), pop_layout())
#define UI_ROW defer_loop(push_widget_row(), pop_layout())
#define UI_COLUMN defer_loop(push_widget_column(), pop_layout())
#define UI_WINDOW(rect, fmt, ...) defer_loop(ui_window(rect, fmt, ##__VA_ARGS__), pop_widget_window()) 
#define UI_POPUP(rect, fmt, ...) defer_loop(ui_popup(rect, fmt, ##__VA_ARGS__), pop_widget_window()) 
#define UI_POPUP(rect, fmt, ...) defer_loop(ui_popup(rect, fmt, ##__VA_ARGS__), pop_widget_window()) 
#define UI_CONTAINER(fmt, ...) defer_loop(ui_container(fmt, ##__VA_ARGS__), pop_widget_container()) 
#define UI_WIDTHFILL defer_loop(push_widget_widthfill(), pop_layout())
#define UI_HEIGHTFILL defer_loop(push_widget_heightfill(), pop_layout())
#define UI_WRAP defer_loop(push_widget_wrap(), pop_layout())
#define UI_STYLE(style) defer_loop(push_style(style), pop_style())
#define UI_PAD(p) defer_loop(push_widget_padding(p), pop_layout())
#define ID(fmt, ...) defer_loop(push_id(generate_id(fmt, ##__VA_ARGS__)), pop_id())



#define ForEachWidgetChild(w) for(auto it = w->first_child; it; it = it->next_sibling)
#define ForEachWidgetSibling(w) for(auto it = w; it; it = it->next_sibling)

global s64 max_hash_nexts;