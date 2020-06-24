typedef u64 UI_ID;


struct Closure {
    u8* parameters;
    void(*callback)(u8* parameters);
};

struct Widget {
    f32 x, y;
    f32 width, height;
    UI_ID id;
    Widget* next;
    Widget* last;
    
    
    Closure closure;
};

global struct {
    Widget* widgets;
    UI_ID hover_id;
    UI_ID clicked_id;
    
    Arena frame_arena;
    Arena parameter_arena;
} ui_state;

struct Arg_Type {
    u8* arg;
    int size;
};

Arg_Type
make_arg_type(u8* arg, int size){
    Arg_Type result;
    result.arg = arg;
    result.size = size;
    return result;
}

#define MAX_PARAM_SIZE 128

#define arg(x) make_arg_type((unsigned char*)&x, sizeof(x))
#define get_arg(params, type) *((type*)params); params += sizeof(type)


internal Closure 
make_closure(void(*callback)(unsigned char* parameters), int num, ...) {
    
    Closure closure;
    closure.parameters = (u8*)arena_allocate(&ui_state.frame_arena, MAX_PARAM_SIZE);
    closure.callback = callback;
    va_list args;
    
    va_start(args, num);
    
    Arg_Type arg_type;
    int offset = 0;
    for (int i = 0; i < num; i++) {
        arg_type = va_arg(args, Arg_Type);
        memcpy(closure.parameters + offset, arg_type.arg, arg_type.size);
        offset += arg_type.size;
    }
    va_end(args);
    
    return closure;
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
    return (UI_ID)id;
}

internal Widget* 
_push_widget(f32 x, f32 y, f32 width, f32 height, UI_ID id, 
             Closure closure, bool has_parameters = false){
    auto widget = (Widget*)arena_allocate(&ui_state.frame_arena, sizeof(Widget));
    widget->x = x;
    widget->y = y;
    widget->width = width;
    widget->height = height;
    widget->id = id;
    widget->next = nullptr;
    widget->closure = closure;
    
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
    bool hovered = false;
    for(Widget* widget = ui_state.widgets; widget; widget = widget->next){
        if(is_mouse_in_rect(widget->x, widget->y, widget->width, widget->height)){
            hovered = true;
            ui_state.hover_id = widget->id;
            if(platform.mouse_left_clicked){
                active = widget;
                ui_state.clicked_id = widget->id;
            }
        }
    }
    if(!hovered){
        ui_state.hover_id = -1;
    }
    
    if(active){
        active->closure.callback(active->closure.parameters);
    }
    
    arena_reset(&ui_state.frame_arena);
    ui_state.widgets = nullptr;
    
}

internal inline void
push_rectangle(f32 x, f32 y, f32 width, f32 height, f32 radius, u32 colour = 0xFF00FFFF);

internal void 
button(f32 x, f32 y, f32 width, f32 height, u32 colour, Closure closure){
    auto widget = _push_widget(x, y, width, height, gen_unique_id("Test"), closure);
    widget->closure = closure;
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
