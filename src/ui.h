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



struct Closure {
    u8* parameters;
    void(*callback)(u8* parameters);
};

enum Click_Type {
    CLICK_LEFT,
    CLICK_RIGHT,
    CLICK_DRAG,
    CLICK_COUNT, 
};

struct Widget {
    f32 x, y;
    f32 width, height;
    UI_ID id;
    Widget* next;
    Widget* last;
    
    Click_Type click_type;
    
    Closure closures[CLICK_COUNT];
    
    b32* clicked;
};



enum Panel_Split_Type {
    PANEL_SPLIT_VERTICAL,
    PANEL_SPLIT_HORIZONTAL,
};

enum Panel_Type {
    PANEL_EDITOR,
    PANEL_PROPERTIES,
};

struct Presenter;
internal void present(Presenter* presenter);
internal void reset_presenter(Presenter* presenter);

struct Panel {
    Panel_Split_Type split_type;
    Panel_Type type;
    f32 split_ratio;
    Panel* first; 
    Panel* second;
    Panel* parent;
    Presenter* presenter;
};

struct UI_State {
    Widget* widgets = nullptr;
    Widget* widgets_tail = nullptr;
    UI_ID hover_id;
    UI_ID clicked_id;
    UI_ID right_clicked_id;
    UI_ID drag_id;
    UI_ID menu_id;
    
    
    Arena frame_arena;
    Arena parameter_arena;
    
    f32 x_start;
    f32 y_start;
    f32 x_offset;
    f32 y_offset;
    
    bool menu_open;
    f32 menu_x;
    f32 menu_y;
    
    Panel* active_panel;
    
    Panel* resize_panel;
    bool panel_is_resizing;
    
    bool show_splash_screen;
    
    Theme theme;
};

struct Arg_Type {
    u8* arg;
    u64 size;
};

Arg_Type
make_arg_type(u8* arg, int size){
    Arg_Type result;
    result.arg = arg;
    result.size = size;
    return result;
}

#define MAX_PARAM_SIZE 512

#define arg(x) make_arg_type((unsigned char*)&x, sizeof(x))
#define get_arg(params, type) *((type*)params); params += sizeof(type)


struct Animation_State {
    UI_ID id = U64Max;
    f32 x_offset = 0;
    f32 y_offset = 0;
    f32 x_scale = 0;
    f32 y_scale = 0;
    
    
    u64 last_updated = 0;
    
    v4f source_rect;
    v4f target_rect;
    v4f rect;
};

typedef u64 UI_ID;
