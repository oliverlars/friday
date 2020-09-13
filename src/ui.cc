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

global Theme theme;

internal void
load_theme_ayu(){
    
    theme.background.packed = 0x070707ff;
    theme.panel.packed = 0x161925ff;
    theme.menu.packed = 0x0D1012ff;
    theme.text.packed = 0xE7E7E7ff;
    theme.text_comment.packed = 0xffc2d94d;
    theme.text_literal.packed = 0x31A231ff;
    theme.text_function.packed = 0x21AFD4ff;
    theme.text_type.packed = 0xDC593Fff;
    theme.text_misc.packed = 0x646464ff;
    theme.cursor.packed = 0xe08c17ff;
    theme.error.packed = 0xffcc3333;
    
    theme.view_button.packed = 0x0D1012ff;
    theme.button_highlight.packed = 0x292D3Dff;
    
}

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
    Closure hover;
    Closure right_click;
    Closure middle_click;
    
    b32* clicked;
};

global struct {
    Widget* widgets = nullptr;
    Widget* widgets_tail = nullptr;
    UI_ID hover_id;
    UI_ID clicked_id;
    UI_ID menu_id;
    
    
    Arena frame_arena;
    Arena parameter_arena;
    
    f32 x_start;
    f32 y_start;
    f32 x_offset;
    f32 y_offset;
} ui_state;

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


internal Closure 
make_closure(void(*callback)(unsigned char* parameters), int num, ...) {
    
    Closure closure;
    closure.parameters = (u8*)arena_allocate(&ui_state.parameter_arena, MAX_PARAM_SIZE);
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

#define HASH_INITIAL 2166136261

internal void
hash32(u64* hash, char* data, int size){
    
    while(size--){
        *hash = (*hash ^ *data++) * 16777619;
    }
}

internal UI_ID
gen_unique_id(char* label){
    int size = strlen(label);
    UI_ID result = 0;
    hash32(&result, label, size);
    return result;
}

internal UI_ID
gen_unique_id(String8 label){
    int size = label.length;
    UI_ID result = 0;
    hash32(&result, label.text, size);
    return result;
}

internal UI_ID
gen_id(char* label){
    
    u64 id = (u64)(void*)label;
    return (UI_ID)id;
}

internal UI_ID
gen_id(String8 label){
    
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
    widget->clicked = nullptr;
    
    if(!ui_state.widgets){
        ui_state.widgets = widget;
        ui_state.widgets_tail = ui_state.widgets;
    }else{
        ui_state.widgets_tail->next = widget;
        ui_state.widgets_tail = ui_state.widgets_tail->next;
    }
    return widget;
}


internal Widget* 
ui_push_widget(f32 x, f32 y, f32 width, f32 height, UI_ID id,
               Closure closure, bool has_parameters = false){
    
    return _push_widget(x, y, width, height, id, closure, has_parameters);
}

internal bool
is_mouse_in_rect(f32 x, f32 y, f32 width, f32 height){
    return platform.mouse_x <= (x + width) && platform.mouse_x >= x &&
        platform.mouse_y <= (y + height) && platform.mouse_y >= y;
}

#define PANEL_MARGIN 5

internal void
ui_begin_panel(f32 x, f32 y, f32 width, f32 height){
    ui_state.x_start = x + PANEL_MARGIN;
    ui_state.y_start = height - (y - PANEL_MARGIN);
    ui_state.x_offset = 0;
    ui_state.y_offset = 0;
}

internal void
ui_end_panel(){
    ui_state.x_start = 0;
    ui_state.y_start = 0;
    ui_state.x_offset = 0;
    ui_state.y_offset = 0;
}

internal f32
ui_get_x(){
    return ui_state.x_start + ui_state.x_offset;
}

internal f32
ui_get_y(){
    return ui_state.y_start + ui_state.y_offset;
}

internal void
ui_new_line(){
    ui_state.x_offset = 0;
    ui_state.y_offset -= 60.0f;
}


internal void
process_widgets_and_handle_events(){
    
    Widget* active = nullptr;
    Widget* active_hover = nullptr;
    
    bool hovered = false;
    
    enum Click_Type {
        CLICK_LEFT,
        CLICK_LEFT_DOUBLE,
        CLICK_RIGHT,
        CLICK_MIDDLE,
    };
    Click_Type click_type;
    for(Widget* widget = ui_state.widgets; widget->next; widget = widget->next){
        if(is_mouse_in_rect(widget->x, widget->y, widget->width, widget->height)){
            hovered = true;
            ui_state.hover_id = widget->id;
            
            active_hover = widget;
            if(platform.mouse_left_clicked){
                if(widget->clicked){
                    *widget->clicked  = !*widget->clicked;
                }
                if(ui_state.clicked_id == widget->id){
                    ui_state.clicked_id = -1;
                }else{
                    active = widget;
                    ui_state.clicked_id = widget->id;
                    click_type = CLICK_LEFT;
                }
            }
            
        }
    }
    if(!hovered){
        ui_state.hover_id = -1;
    }else {
        
    }
    if(active_hover){
        if(active_hover->hover.callback){
            active_hover->hover.callback(active_hover->hover.parameters);
        }
    }
    if(active){
        if(active->closure.callback){
            active->closure.callback(active->closure.parameters);
        }
        
    }
}

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
    Panel* children[2];
    Presenter* presenter;
};

internal void push_rectangle(f32 x, f32 y, f32 width, f32 height, f32 radius, u32 colour);

#define PANEL_BORDER 5

static bool panel_hover = 0;
static bool panel_resize = 0;
Panel* active_panel = nullptr;
static f32 old_width = 0;
static f32 split_ratio = 0;

internal bool
is_mouse_dragged(f32 x, f32 y, f32 width, f32 height){
    return platform.mouse_drag && platform.mouse_drag_x >= x && platform.mouse_drag_x <= x + width &&
        platform.mouse_drag_y >= y && platform.mouse_drag_y <= y + height;
    
}

internal void
process_panels(Panel* root, f32 delta_split){
    if(!root) return;
    for(int i = 0; i < 2;i++){
        if(root->children[i]){
            switch(root->children[i]->split_type){
                case PANEL_SPLIT_HORIZONTAL:{
                    root->children[i]->split_ratio = delta_split;
                    process_panels(root->children[i], delta_split);
                }break;
                case PANEL_SPLIT_VERTICAL:{
                    root->children[i]->split_ratio -= delta_split;
                    process_panels(root->children[i], delta_split);
                }break;
            }
        }
    }
}

internal void 
split_panel(Panel* panel, f32 split_ratio, Panel_Split_Type split_type, Panel_Type type){
    if(!panel) return;
    
    if(panel->children[0] && panel->children[1]){
        return;
    }else if(panel->children[0]){
        //panel->children[1] = (Panel*)arena_allocate(&platform.permanent_arena, sizeof(Panel));
        panel->children[1] = (Panel*)calloc(1, sizeof(Panel));
        panel->children[1]->split_ratio = split_ratio;
        panel->children[1]->split_type = split_type;
        panel->children[1]->type = type;
    }else {
        //panel->children[0] = (Panel*)arena_allocate(&platform.permanent_arena, sizeof(Panel));
        panel->children[0] = (Panel*)calloc(1, sizeof(Panel));
        panel->children[0]->split_ratio = split_ratio;
        panel->children[0]->split_type = split_type;
        panel->children[0]->type = type;
    }
    
}

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

#define ANIM_STATE_SIZE 1024
global Animation_State animation_state[ANIM_STATE_SIZE];

internal Animation_State*
init_animation_state(UI_ID id){
    u64 tick = platform.tick;
    int index = 0;
    for(int i = 0; i < ANIM_STATE_SIZE; i++){
        if(animation_state[i].last_updated < tick){
            tick = animation_state[i].last_updated;
            index = i;
        }
    }
    Animation_State state = {};
    animation_state[index] = state;
    animation_state[index].id = id;
    animation_state[index].last_updated = platform.tick;
    animation_state[index].rect = v4f(0,0,0,0);
    animation_state[index].source_rect = v4f(0,0,0,0);
    animation_state[index].target_rect = v4f(0,0,0,0);
    return &animation_state[index];
}

internal Animation_State*
get_animation_state(UI_ID id){
    for(int i = 0; i < ANIM_STATE_SIZE; i++){
        
        if(animation_state[i].id == id && 
           (platform.tick - animation_state[i].last_updated  < 2)){
            return &animation_state[i];
        }
    } 
    return nullptr;
}

internal void
update_animation_state(Animation_State* anim_state, f32 x_offset, f32 y_offset, f32 x_scale, f32 y_scale){
    if(!anim_state) return; 
    
    anim_state->x_offset += x_offset;
    anim_state->y_offset += y_offset;
    anim_state->x_scale += x_scale;
    anim_state->y_scale += y_scale;
    
    anim_state->last_updated = platform.tick;
}

internal bool
unanimate(Animation_State* anim_state){
    if(!anim_state) return false;
    anim_state->rect.x += (anim_state->source_rect.x - anim_state->rect.x)*platform.delta_time*8.f;
    anim_state->rect.y += (anim_state->source_rect.y - anim_state->rect.y)*platform.delta_time*8.f;
    anim_state->rect.z += (anim_state->source_rect.z - anim_state->rect.z)*platform.delta_time*8.f;
    anim_state->rect.w += (anim_state->source_rect.w - anim_state->rect.w)*platform.delta_time*8.f;
    anim_state->last_updated = platform.tick;
    return rects_similar(anim_state->rect, anim_state->source_rect, 1.0f);
}

internal bool
animate(Animation_State* anim_state){
    if(!anim_state) return false; 
    anim_state->rect.x += (anim_state->target_rect.x - anim_state->rect.x)*platform.delta_time*8.f;
    anim_state->rect.y += (anim_state->target_rect.y - anim_state->rect.y)*platform.delta_time*8.f;
    anim_state->rect.z += (anim_state->target_rect.z - anim_state->rect.z)*platform.delta_time*8.f;
    anim_state->rect.w += (anim_state->target_rect.w - anim_state->rect.w)*platform.delta_time*8.f;
    
    anim_state->last_updated = platform.tick;
    return rects_similar(anim_state->rect, anim_state->target_rect, 1.0f);
}

