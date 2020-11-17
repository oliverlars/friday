global UI_State* ui = 0;

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
    
    ui->theme.background.packed = 0x070707ff;
    ui->theme.panel.packed = 0x161925ff;
    ui->theme.menu.packed = 0x0D1012ff;
    ui->theme.text.packed = 0xE7E7E7ff;
    ui->theme.text_comment.packed = 0xffc2d94d;
    ui->theme.text_literal.packed = 0x31A231ff;
    ui->theme.text_function.packed = 0x21AFD4ff;
    ui->theme.text_type.packed = 0xDC593Fff;
    ui->theme.text_misc.packed = 0x646464ff;
    ui->theme.cursor.packed = 0xe08c17ff;
    ui->theme.error.packed = 0xffcc3333;
    
    ui->theme.view_button.packed = 0x0D1012ff;
    ui->theme.button_highlight.packed = 0x292D3Dff;
    
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
}

internal Widget*
make_widget(String8 string){
    auto result = (Widget*)arena_allocate(&ui->widgets[ui->widget_frame].arena, sizeof(Widget));
    result->string = string;
    result->id = gen_unique_id(string);
    return result;
}

internal Widget*
push_widget(String8 string){
    auto widget = make_widget(string);
    if(!ui->widgets[ui->widget_frame].head){
        ui->widgets[ui->widget_frame].head = widget;
        ui->widgets[ui->widget_frame].tail = widget;
    }else {
        ui->widgets[ui->widget_frame].tail->child = widget;
        ui->widgets[ui->widget_frame].tail = widget;
    }
    return widget;
}

internal Widget*
push_button_widget(String8 string){
    auto widget = push_widget(string);
    widget->type = WIDGET_BUTTON;
    widget->rect = get_text_bbox({}, string);
    return widget;
}

internal void
ui_begin_panel(char* label, v4f rect){
    auto widget = push_widget(make_string(label));
    widget->rect = rect;
    push_clip_range_begin(rect);
}

internal void
ui_end_panel(){
    
    push_clip_range_end();
}

#define ui_panel(label, rect) defer_loop(ui_begin_panel(label, rect), ui_end_panel())

internal void 
split_panel(Panel* panel, f32 split_ratio, Panel_Split_Type split_type, Panel_Type type){
    if(!panel) return;
    
    assert(!panel->first && !panel->second);
    
    panel->first = (Panel*)arena_allocate(&platform->permanent_arena, sizeof(Panel));
    panel->second = (Panel*)arena_allocate(&platform->permanent_arena, sizeof(Panel));
    
    panel->first->split_ratio = split_ratio;
    panel->second->split_ratio = 1.0f - split_ratio;
    
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

internal void
render_widgets(Widget* widget) {
    switch(widget->type){
        case WIDGET_BUTTON:{
            push_rectangle(widget->rect, 5, ui->theme.panel);
            push_string(v2f(widget->rect.x, widget->rect.y), widget->string, ui->theme.text);
        }break;
    }
}

internal bool
update_widget(Widget* widget){
    bool last_frame = !ui->widget_frame;
    for(auto it = ui->widgets[last_frame].head; it; it = it->child){
        if(it->id == widget->id){
            if(is_in_rect(platform->mouse_position, it->rect)){
                return true;
            }
        }
    }
    
    return false;
}

#define ui_widthfill defer_loop(ui_push_widthfill(), ui_pop_widthfill())
#define ui_heightfill defer_loop(ui_push_heightfill(), ui_pop_heightfill())
#define ui_row defer_loop(ui_push_row(), ui_pop_row())

internal void
ui_layout_and_render(){
    auto head = ui->widgets[ui->widget_frame].head;
    auto tail = ui->widgets[ui->widget_frame].tail;
    for(auto it = head; it; it = it->child){
        render_widgets(it);
    }
}

internal bool
button(char* text){
    auto widget = push_button_widget(make_string(text));
    return update_widget(widget);
}
