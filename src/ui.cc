global UI_State* ui = 0;

internal Arena*
current_frame_widget_arena() {
    return &ui->widget_arena[ui->widget_frame];
}

internal Arena*
last_frame_widget_arena() {
    return &ui->widget_arena[!ui->widget_frame];
}

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
generate_id(char* label){
    int size = strlen(label);
    UI_ID result = 0;
    hash32(&result, label, size);
    return result;
}

internal UI_ID
generate_id(String8 label){
    int size = label.length;
    UI_ID result = 0;
    hash32(&result, label.text, size);
    return result;
}

internal bool
widget_has_property(Widget* widget, Widget_Property property){
    return !!(widget->properties[property / 64] & ((u64)1 << (property % 64)));
}

internal void
widget_set_property(Widget* widget, Widget_Property property){
    widget->properties[property / 64] |= ((u64)1 << (property % 64));
}

internal void
widget_remove_property(Widget* widget, Widget_Property property){
    widget->properties[property / 64] &= ~((u64)1 << (property % 64));
}

internal Widget*
make_widget(){
    
    Widget* widget = push_type_zero(&ui->frame_arena, Widget);
    if(!ui->root){
        ui->root = widget;
    }
    widget->min = v2f(20, 20);
    
    return widget;
}

internal Widget*
make_widget(String8 string){
    Widget* widget = make_widget();
    widget->string = string;
    return widget;
}

internal Layout*
push_layout(Widget* widget){
    auto layout = push_type_zero(&ui->frame_arena, Layout);
    
    if(!ui->layout_stack){
        ui->layout_stack = layout;
    }else {
        layout->prev = ui->layout_stack;
        ui->layout_stack = layout;
    }
    layout->widget = widget;
    
    return layout;
}

internal Widget*
push_widget(){
    Widget* widget = make_widget();
    
    if(!ui->layout_stack){
        //push_layout(widget);
        return widget;
    }
    
    auto it = ui->layout_stack->widget;
    if(it->last_child){
        it->last_child->next_sibling = widget;
        widget->prev_sibling = it->last_child;
        it->last_child = widget;
        widget->next_sibling = nullptr;
    }else {
        it->first_child = widget;
        it->last_child = widget;
    }
    
    return widget;
}

internal Widget*
push_widget(String8 string){
    Widget* widget = push_widget();
    widget->string = string;
    return widget;
}

internal void
pop_layout(){
    if(ui->layout_stack){
        ui->layout_stack = ui->layout_stack->prev;
    }else{
    }
}

internal void
push_widget_row(){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_ROW);
}

internal void
push_widget_column(){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_COLUMN);
}


#define UI_ROW defer_loop(push_widget_row(), pop_layout())
#define UI_COLUMN defer_loop(push_widget_column(), pop_layout())
#define UI_WINDOW defer_loop(push_widget_window(), pop_layout())

internal v2f layout_and_render(Widget* widget, v2f pos);

internal void
layout_row(Widget* widget, v2f pos){
    if(!widget) return;
    for(auto it = widget; it; it = it->next_sibling){
        v2f next_pos = layout_and_render(it, pos);
        pos.x += next_pos.width;
    }
}

internal void
layout_column(Widget* widget, v2f pos){
    if(!widget) return;
    for(auto it = widget; it; it = it->next_sibling){
        pos.y -= layout_and_render(it, pos).height;
    }
}

internal v2f
layout_and_render(Widget* widget, v2f pos){
    if(!widget) return {};
    if(widget_has_property(widget, WP_COLUMN)){
        layout_column(widget->first_child, pos);
    }
    else if(widget_has_property(widget, WP_ROW)){
        layout_row(widget->first_child, pos);
    }
    
    if(widget_has_property(widget, WP_RENDER_TEXT)){
        v4f bbox = get_text_bbox(pos, widget->string, 1);
        widget->min = bbox.size;
        if(widget_has_property(widget, WP_RENDER_BACKGROUND)){
            push_rectangle(bbox, 1, ui->theme.panel);
        }
        if(widget_has_property(widget, WP_RENDER_BORDER)){
            push_rectangle_outline(bbox, 5, 3, ui->theme.text);
        }
        push_string(pos, widget->string, ui->theme.panel);
    }
    return widget->min;
}

internal void
button(char* label){
    String8 string = make_string(label);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_TEXT);
    widget_set_property(widget, WP_RENDER_BORDER);
}