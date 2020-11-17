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

#define PANEL_MARGIN_X 60
#define PANEL_MARGIN_Y 5

#define PANEL_BORDER 5

struct Panel {
    Panel_Split_Type split_type;
    Panel_Type type;
    f32 split_ratio;
    Panel* first; 
    Panel* second;
    Panel* parent;
    Presenter* presenter;
};

enum Widget_Type {
    WIDGET_TEXT_BOX,
    WIDGET_BUTTON_ICON,
    WIDGET_BUTTON,
    WIDGET_PANEL,
    WIDGET_CHECKBOX,
};

struct Widget {
    UI_ID id;
    String8 string;
    
    Widget* sibling;
    Widget* child;
    Widget* parent;
    v4f rect;
    v4f source_rect;
    v4f target_rect;
    
    Widget_Type type;
    union {
        struct {
            Panel_Split_Type split_type;
            Panel_Type type;
            f32 split_ratio;
            Presenter* presenter;
        } panel;
        
        struct {
            
        } button;
        
        struct {
            
        } checkbox;
        
        struct {
            
        } button_icon;
        
        struct {
            
        } text_box;
    };
};

struct UI_State {
    UI_ID hover_id;
    UI_ID clicked_id;
    
    Arena frame_arena;
    
    struct Storage {
        Arena arena;
        Widget* head;
        Widget* tail;
    };
    
    Storage widgets[2];
    bool widget_frame;
    
    Theme theme;
    
    f32 round_amount;
};

typedef u64 UI_ID;
