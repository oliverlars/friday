global UI_State* ui_state = 0;

internal b32
has_left_clicked(Platform_Event **event_out){
    b32 result = 0;
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_MOUSE_PRESS && event->mouse_button == MOUSE_BUTTON_LEFT){
            *event_out = event;
            result = 1;
        }
    }
    return(result);
}

internal b32
has_left_clicked(){
    Platform_Event* event = 0;
    b32 result = has_left_clicked(&event);
    if (result){
        platform_consume_event(event);
    }
    return(result);
}

internal b32
has_right_clicked(Platform_Event **event_out){
    b32 result = 0;
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_MOUSE_PRESS && event->mouse_button == MOUSE_BUTTON_RIGHT){
            *event_out = event;
            result = 1;
        }
    }
    return(result);
}

internal b32
has_right_clicked(){
    Platform_Event* event = 0;
    b32 result = has_right_clicked(&event);
    if (result){
        platform_consume_event(event);
    }
    return(result);
}

internal void
load_theme_ayu(){
    
    ui_state->theme.background.packed = 0x070707ff;
    ui_state->theme.panel.packed = 0x161925ff;
    ui_state->theme.menu.packed = 0x0D1012ff;
    ui_state->theme.text.packed = 0xE7E7E7ff;
    ui_state->theme.text_comment.packed = 0xffc2d94d;
    ui_state->theme.text_literal.packed = 0x31A231ff;
    ui_state->theme.text_function.packed = 0x21AFD4ff;
    ui_state->theme.text_type.packed = 0xDC593Fff;
    ui_state->theme.text_misc.packed = 0x646464ff;
    ui_state->theme.cursor.packed = 0xe08c17ff;
    ui_state->theme.error.packed = 0xffcc3333;
    
    ui_state->theme.view_button.packed = 0x0D1012ff;
    ui_state->theme.button_highlight.packed = 0x292D3Dff;
    
}


internal Closure 
make_closure(void(*callback)(unsigned char* parameters), int num, ...) {
    
    Closure closure;
    closure.parameters = (u8*)arena_allocate(&ui_state->parameter_arena, MAX_PARAM_SIZE);
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

internal UI_ID
gen_unique_id(void* data){
    u64 id = (u64)data;
    return (UI_ID)id;
}

internal UI_ID
gen_unique_id(void* data, int num_bytes){
    UI_ID result = 0;
    hash32(&result, (char*)data, num_bytes);
    return (UI_ID)result;
}

internal void
ui_begin(){
    ui_state->widgets = nullptr;
}

internal Widget* 
push_widget(v4f rect, UI_ID id, 
            Closure closure, Click_Type click_type = CLICK_LEFT){
    
    auto widget = (Widget*)arena_allocate_zero(&renderer->frame_arena, sizeof(Widget));
    widget->rect = rect;
    widget->id = id;
    widget->next = nullptr;
    widget->closures[click_type] = closure;
    widget->clicked = nullptr;
    widget->click_type = click_type;
    
    if(!ui_state->widgets){
        ui_state->widgets = widget;
        ui_state->widgets_tail = ui_state->widgets;
    }else{
        ui_state->widgets_tail->next = widget;
        ui_state->widgets_tail = ui_state->widgets_tail->next;
    }
    return widget;
}


#define PANEL_MARGIN_X 60
#define PANEL_MARGIN_Y 5

internal void
ui_begin_panel(f32 x, f32 y, f32 width, f32 height){
    ui_state->x_start = x + PANEL_MARGIN_X;
    ui_state->y_start = height - (y + PANEL_MARGIN_Y);
    ui_state->x_offset = 0;
    ui_state->y_offset = 0;
    push_clip_range_begin(v4f(x,y, width, height));
}

internal void
ui_end_panel(){
    ui_state->x_start = 0;
    ui_state->y_start = 0;
    ui_state->x_offset = 0;
    ui_state->y_offset = 0;
    push_clip_range_end();
}

internal f32
ui_get_x(){
    return ui_state->x_start + ui_state->x_offset;
}

internal f32
ui_get_y(){
    return ui_state->y_start + ui_state->y_offset;
}

internal void
ui_new_line(){
    ui_state->x_offset = 0;
    ui_state->y_offset -= 60.0f;
}


internal void
ui_process_widgets_and_handle_events(){
    // TODO(Oliver): this should handle more than just events
    // make it also handle the drawing and clipping
    // would solve a few issues i have
    Widget* active = nullptr;
    Widget* active_hover = nullptr;
    
    bool hovered = false;
    
    for(Widget* widget = ui_state->widgets; widget; widget = widget->next){
        if(is_in_rect(platform->mouse_position, widget->rect)){
            hovered = true;
            ui_state->hover_id = widget->id;
            
            active_hover = widget;
            if(has_left_clicked()){
                if(widget->clicked){
                    *widget->clicked  = !*widget->clicked;
                }
                if(ui_state->clicked_id == widget->id){
                    ui_state->clicked_id = HASH_INITIAL;
                }else{
                    active = widget;
                    ui_state->clicked_id = widget->id;
                }
            }else if(has_right_clicked()){
                if(ui_state->right_clicked_id == widget->id){
                    ui_state->right_clicked_id = HASH_INITIAL;
                }else {
                    //active = widget;
                    ui_state->right_clicked_id = widget->id;
                }
            }
            
        }
    }
    if(!hovered){
        ui_state->hover_id = HASH_INITIAL;
    }
    if(active){
        auto click_type = active->click_type;
        if(active->closures[click_type].callback){
            active->closures[click_type]
                .callback(active->closures[click_type].parameters);
        }
    }
    
}

#define PANEL_BORDER 5

static bool panel_hover = 0;
static bool panel_resize = 0;
Panel* active_panel = nullptr;
static f32 old_width = 0;
static f32 split_ratio = 0;

internal void 
split_panel(Panel* panel, f32 split_ratio, Panel_Split_Type split_type, Panel_Type type){
    if(!panel) return;
    
    assert(!panel->first && !panel->second);
    
    panel->first = (Panel*)arena_allocate(&platform->permanent_arena, sizeof(Panel));
    panel->second = (Panel*)arena_allocate(&platform->permanent_arena, sizeof(Panel));
    
    panel->first->split_ratio = split_ratio;
    panel->second->split_ratio = 1.0f- split_ratio;
    
    panel->first->parent = panel;
    panel->second->parent = panel;
    
    panel->first->type = panel->type;
    panel->second->type = type;
    
    panel->first->split_type = split_type;
    panel->second->split_type = split_type;
    panel->first->presenter = panel->presenter;
}

internal void 
delete_split(Panel* panel){
    if(!panel) return;
    panel->parent->first = nullptr;
    panel->parent->second = nullptr;
}

#define ANIM_STATE_SIZE 1024
global Animation_State animation_state[ANIM_STATE_SIZE];

internal Animation_State*
init_animation_state(UI_ID id){
    u64 tick = platform->get_time();
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
    animation_state[index].last_updated = platform->get_time();
    animation_state[index].rect = v4f(0,0,0,0);
    animation_state[index].source_rect = v4f(0,0,0,0);
    animation_state[index].target_rect = v4f(0,0,0,0);
    return &animation_state[index];
}

internal Animation_State*
get_animation_state(UI_ID id){
    for(int i = 0; i < ANIM_STATE_SIZE; i++){
        
        if(animation_state[i].id == id && 
           (platform->get_time() - animation_state[i].last_updated  < 2)){
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
    
    anim_state->last_updated = platform->get_time();
}

internal void
unanimate(Animation_State* anim_state){
    if(!anim_state) return;
    lerp_rects(&anim_state->rect, anim_state->source_rect, 0.2f);
    
    anim_state->last_updated = platform->get_time();
}

internal void
animate(Animation_State* anim_state){
    if(!anim_state) return; 
    lerp_rects(&anim_state->rect, anim_state->target_rect, 0.2f);
    
    anim_state->last_updated = platform->get_time();
}


internal UI_ID
button(v2f pos, char* text, Closure closure){
    auto id = gen_unique_id(text);
    
    v4f bbox = get_text_bbox(pos.x, pos.y, text);
    
    auto widget = push_widget(bbox, id, closure);
    
    if(id == ui_state->clicked_id){
        push_string(pos, text, ui_state->theme.text.packed);
        push_rectangle(bbox, 10, ui_state->theme.button_highlight.packed);
    }else if(id == ui_state->hover_id){
        push_rectangle(bbox, 10, ui_state->theme.button_highlight.packed);
        push_string(pos, text, ui_state->theme.text.packed);
    }else{
        push_string(pos, text, ui_state->theme.text.packed);
    }
    
    return id;
}
