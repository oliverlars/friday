
typedef u64 UI_ID;

struct Widget {
    f32 x, y;
    f32 width, height;
    UI_ID id;
    Widget* next;
    Widget* last;
    
    void* parameters;
    void(*callback)(void* parameters);
};

global struct {
    Widget* widgets;
    UI_ID hover_id;
    UI_ID clicked_id;
    
    Arena frame_arena;
    Arena parameter_arena;
} ui_state;

global void* current_parameter_list;
global void* push_parameter_list;

const int MAX_PARAM_SIZE = 128;

internal void
boss_start_params(){
    current_parameter_list = arena_allocate(&ui_state.parameter_arena, MAX_PARAM_SIZE);
    push_parameter_list = current_parameter_list;
}

struct Node;

internal void
boss_push_param_node(Node* node){
    auto param = (Node**)push_parameter_list;
    *param = node;
}


internal UI_ID
gen_unique_id(char* label){
    
    u64 id = (u64)(void*)label;
    char* ptr = label;
    while(ptr && *ptr){
        id += (u64)(void*)ptr;
        ptr++;
    }
    
    return (UI_ID)id;
}


internal UI_ID
gen_unique_id(String8 label){
    
    u64 id = (u64)(void*)label.text;
    for(int i = 0; i < label.length; i++){
        id += (u64)(void*)label.text[i];
    }
    
    return (UI_ID)id;
}

internal Widget* 
_push_widget(f32 x, f32 y, f32 width, f32 height, UI_ID id, 
             void(*callback)(void* parameters),
             bool has_parameters = false){
    auto widget = (Widget*)arena_allocate(&ui_state.frame_arena, sizeof(Widget));
    widget->x = x;
    widget->y = y;
    widget->width = width;
    widget->height = height;
    widget->id = id;
    widget->next = nullptr;
    widget->callback = callback;
    if(has_parameters){
        widget->parameters = current_parameter_list;
    }
    
    if(ui_state.widgets){
        Widget* widgets;
        for(widgets = ui_state.widgets; widgets->next; widgets = widgets->next){
            
        }
        widgets->next = widget;
    }else{
        ui_state.widgets = widget;
    }
    
    
    return widget;
}


internal bool
is_mouse_in_rect(f32 x, f32 y, f32 width, f32 height){
    return platform.mouse_x <= (x + width) && platform.mouse_x >= x &&
        platform.mouse_y <= (y + height) && platform.mouse_y >= y;
}


internal void
process_widgets_and_handle_events(){
    
    Widget* active = nullptr;
    for(Widget* widget = ui_state.widgets; widget; widget = widget->next){
        if(is_mouse_in_rect(widget->x, widget->y, widget->width, widget->height)
           ){
            ui_state.hover_id = widget->id;
            if(platform.mouse_left_clicked){
                active = widget;
                ui_state.clicked_id = widget->id;
            }
        }
    }
    
    if(active){
        active->callback(active->parameters);
    }
    
    arena_reset(&ui_state.frame_arena);
    ui_state.widgets = nullptr;
    
}

internal inline void
push_rectangle(f32 x, f32 y, f32 width, f32 height, f32 radius, u32 colour = 0xFF00FFFF);

internal void 
button(f32 x, f32 y, f32 width, f32 height, u32 colour, void(*callback)(void*
                                                                        parameters)){
    auto widget = _push_widget(x, y, width, height, gen_unique_id("Test"), callback);
    widget->callback = callback;
    push_rectangle(x, y, width, height, 0.2, colour);
}


// NOTE(Oliver): 0xAABBGGRR 
union Colour {
    u32 packed;
    struct {
        u8 a;
        u8 b;
        u8 g;
        u8 r;
    };
};

struct Theme {
    Colour base;
    Colour base_margin;
    Colour menu;
    Colour text;
    Colour text_light;
    Colour text_comment;
    Colour text_function;
    Colour text_type;
    Colour cursor;
    Colour error;
};

global Theme theme;

internal void
load_theme_ayu(){
    
    theme.base.packed = 0x0f1419ff;
    theme.base_margin.packed = 0x0a0e12ff;
    theme.menu.packed = 0x13181dff;
    theme.text.packed = 0xFFFFFFff;
    theme.text_light.packed = 0x262c33ff;
    theme.text_comment.packed = 0xffc2d94d;
    theme.text_function.packed = 0xff5ac2ff;
    theme.text_type.packed = 0xffff29719;
    theme.cursor.packed = 0xe08c17ff;
    theme.error.packed = 0xffcc3333;
}
