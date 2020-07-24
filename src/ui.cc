typedef u64 UI_ID;




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
    
    Closure right_click;
    Closure middle_click;
};

global struct {
    Widget* widgets;
    UI_ID hover_id;
    UI_ID clicked_id;
    UI_ID menu_id;
    
    
    Arena frame_arena;
    Arena parameter_arena;
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
    
    //auto widget = (Widget*)arena_allocate(&ui_state.frame_arena, sizeof(Widget));
    auto widget = (Widget*)calloc(1, sizeof(Widget));
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
    
    enum Click_Type {
        CLICK_LEFT,
        CLICK_LEFT_DOUBLE,
        CLICK_RIGHT,
        CLICK_MIDDLE,
    };
    Click_Type click_type;
    for(Widget* widget = ui_state.widgets; widget; widget = widget->next){
        if(is_mouse_in_rect(widget->x, widget->y, widget->width, widget->height)){
            hovered = true;
            ui_state.hover_id = widget->id;
            
            if(platform.mouse_left_down){
                active = widget;
                ui_state.clicked_id = widget->id;
                click_type = CLICK_LEFT;
            }
#if 0
            else if(platform.mouse_right_clicked){
                active = widget;
                click_type = CLICK_RIGHT;
            }else if(platform.mouse_left_double_clicked){
                active = widget;
                click_type = CLICK_LEFT_DOUBLE;
            }
#endif
        }
    }
    if(!hovered){
        ui_state.hover_id = -1;
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

struct Panel {
    Panel_Split_Type type;
    f32 split_ratio;
    Panel* children[2];
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
            switch(root->children[i]->type){
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
draw_panels(Panel* root, int posx, int posy, int width, int height, u32 colour = 0xFFFFFFFF){
    if(!root) return;
    auto id = gen_unique_id("panel");
    f32 new_width = width;
    f32 new_height = height;
    f32 new_posx = posx;
    f32 new_posy = posy;
    
    for(int i = 0; i < 2; i++){
        
        if(root->children[i]){
            switch(root->children[i]->type){
                case PANEL_SPLIT_HORIZONTAL:{
                    new_height *= root->children[i]->split_ratio;
                    new_posy += new_height;
                    draw_panels(root->children[i], posx, new_posy, new_width, height-new_height, colour);
                }break;
                case PANEL_SPLIT_VERTICAL:{
                    new_width *= root->children[i]->split_ratio;
                    new_posx += new_width;
                    draw_panels(root->children[i], new_posx, posy, width-new_width, new_height, colour);
                }break;
            }
        }
        new_posx = 0;
        new_posy = 0;
    }
    
    //push_rectangle(posx,posy+PANEL_BORDER, PANEL_BORDER, new_height-PANEL_BORDER, 0, 0xFF0000FF);
    if(!panel_resize && is_mouse_dragged(posx-PANEL_BORDER,posy+PANEL_BORDER, PANEL_BORDER*2, new_height-PANEL_BORDER)){
        panel_resize = 1;
        old_width = width;
        active_panel = root;
        split_ratio = root->split_ratio;
        SDL_Cursor* cursor;
        cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
        SDL_SetCursor(cursor);
        
    }
    
    
    if(!panel_resize){
        SDL_Cursor* cursor;
        cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
        SDL_SetCursor(cursor);
    }else if(panel_resize){
        f32 delta =  (platform.mouse_x-platform.mouse_drag_x)/old_width;
        if(root == active_panel){
            root->split_ratio = clampf(split_ratio + split_ratio*delta, 0, 1);
        }else {
            root->split_ratio = clampf(split_ratio - delta, 0, 1);
        }
    }
    auto callback = [](u8* parameters){
        return;
    };
    Closure closure = make_closure(callback, 0);
    auto widget = _push_widget(posx+PANEL_BORDER, posy+PANEL_BORDER,
                               new_width-PANEL_BORDER*2, new_width-PANEL_BORDER*2, id,
                               closure);
    push_rectangle(posx+PANEL_BORDER,posy+PANEL_BORDER, 
                   new_width-PANEL_BORDER*2, new_height-PANEL_BORDER*2, 5, colour);
    
}

internal void 
split_panel(Panel* panel, f32 split_ratio, Panel_Split_Type type){
    if(!panel) return;
    
    if(panel->children[0] && panel->children[1]){
        return;
    }else if(panel->children[0]){
        //panel->children[1] = (Panel*)arena_allocate(&platform.permanent_arena, sizeof(Panel));
        panel->children[1] = (Panel*)calloc(1, sizeof(Panel));
        panel->children[1]->split_ratio = split_ratio;
        panel->children[1]->type = type;
    }else {
        //panel->children[0] = (Panel*)arena_allocate(&platform.permanent_arena, sizeof(Panel));
        panel->children[0] = (Panel*)calloc(1, sizeof(Panel));
        panel->children[0]->split_ratio = split_ratio;
        panel->children[0]->type = type;
    }
    
}

struct Animation_State {
    UI_ID id = -1;
    f32 x_offset = 0;
    f32 y_offset = 0;
    f32 x_scale = 1.0f;
    f32 y_scale = 1.0f;
    u64 last_updated = 0;
};

#define ANIM_STATE_SIZE 1024
global Animation_State animation_state[ANIM_STATE_SIZE];

internal Animation_State*
init_animation_state(UI_ID id){
    u64 tick = platform.tick;
    int index = 0;
    for(int i = 0; i < ANIM_STATE_SIZE; i++){
        if(animation_state[i].last_updated < platform.tick){
            tick = animation_state[i].last_updated;
            index = i;
        }
    }
    Animation_State state;
    animation_state[index] = state;
    animation_state[index].id = id;
    animation_state[index].last_updated = platform.tick;
    return &animation_state[index];
}

internal Animation_State*
get_animation_state(UI_ID id){
    u64 tick = platform.tick;
    u64 index = 0;
    bool found = false;
    for(int i = 0; i < ANIM_STATE_SIZE; i++){
        
        if(animation_state[i].id == id && 
           (platform.tick - animation_state[i].last_updated) < 2){
            return &animation_state[i];
        }
    }
    return nullptr;
}
