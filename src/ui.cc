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
    widget->id = generate_id(string);
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
        widget->parent = it;
        widget->next_sibling = nullptr;
    }else {
        it->first_child = widget;
        it->last_child = widget;
        widget->parent = it;
    }
    
    return widget;
}

internal Widget*
push_widget(String8 string){
    Widget* widget = push_widget();
    widget->string = string;
    return widget;
}

internal Widget_Update
update_widget(Widget* widget){
    Widget_Update result = {};
    if(widget_has_property(widget, WP_CLICKABLE)){
        
    }
    
    if(widget_has_property(widget, WP_RENDER_TEXT)){
        v4f bbox = get_text_bbox({}, widget->string);
        widget->min = bbox.size;
    }
    
    if(widget_has_property(widget, WP_ROW)){
        widget->min = widget->parent->min;
    }
    
    if(widget_has_property(widget, WP_COLUMN)){
        widget->min = widget->parent->min;
    }
    
    if(widget_has_property(widget, WP_WIDTHFILL)){
        widget->min = widget->parent->min;
    }
    
    return result;
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
    update_widget(widget);
}

internal void
push_widget_column(){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_COLUMN);
    update_widget(widget);
}

internal void
push_widget_widthfill(){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_WIDTHFILL);
    update_widget(widget);
}

internal void
push_widget_padding(v2f padding){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    widget->min = widget->parent->min;
    widget->min.x -= padding.width*2;
    widget->min.y -= padding.height*2;
    widget_set_property(widget, WP_PADDING);
    update_widget(widget);
}

internal void
push_widget_window(v4f rect, char* label){
    auto widget = push_widget(make_string(label));
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_WINDOW);
    widget_set_property(widget, WP_CLIP);
    widget->min = rect.size;
    widget->pos = rect.pos;
    update_widget(widget);
    push_widget_padding(v2f(10, 10));
}

#define UI_ROW defer_loop(push_widget_row(), pop_layout())
#define UI_COLUMN defer_loop(push_widget_column(), pop_layout())
#define UI_WINDOW(rect, label) defer_loop(push_widget_window(rect, label), pop_layout())
#define UI_WIDTHFILL defer_loop(push_widget_widthfill(), pop_layout())
#define UI_HEIGHTFILL defer_loop(push_widget_heightfill(), pop_layout())

#define ForEachWidgetChild(w) for(auto it = w->first_child; it; it = it->next_sibling)
#define ForEachWidgetSibling(w) for(auto it = w; it; it = it->next_sibling)

internal v2f layout_widgets(Widget* widget, v2f pos);

internal v2f
layout_row(Widget* widget, v2f pos){
    
    v2f size = {};
    v2f start_pos = pos;
    ForEachWidgetSibling(widget){
        if(widget_has_property(it, WP_WIDTHFILL)){
            it->min.x -= (it->pos.x - start_pos.x);
            it->pos.x = pos.x;
        }
        v2f next_pos = layout_widgets(it, pos);
        pos.x += next_pos.width;
        size.width += next_pos.width;
        size.height = max(size.height, next_pos.height);
        
    }
    return size;
}

internal v2f
layout_column(Widget* widget, v2f pos){
    
    v2f size = {};
    
    ForEachWidgetSibling(widget){
        v2f next_pos = layout_widgets(it, pos);
        pos.y -= next_pos.height;
        size.height -= next_pos.height;
        
        size.width = max(size.width, next_pos.width);
    }
    return size;
}

internal v2f
layout_widthfill(Widget* widget, v2f pos){
    
    v2f size = {};
    
    f32 total_width = 0;
    int number_of_children = 0;
    
    ForEachWidgetSibling(widget) {
        v2f next_pos = layout_widgets(it, pos);
        pos.x += next_pos.width;
        total_width += next_pos.width;
        size.width += next_pos.width;
        size.height = max(size.height, next_pos.height);
        
        number_of_children++;
    }
    v2f available_space = widget->parent->min;
    assert(available_space.width - total_width >= 0);
    f32 width = (available_space.width - total_width)/(f32)number_of_children;
    f32 accum_pos = 0;
    
    ForEachWidgetSibling(widget) {
        it->min.width += width;
        it->pos.x += accum_pos;
        accum_pos += width;
    }
    
    return size;
}

internal v2f
layout_widgets(Widget* widget, v2f pos = {}){
    if(!widget) return {};
    
    if(widget_has_property(widget, WP_WINDOW)){
        pos = widget->pos;
        v2f size = layout_widgets(widget->first_child, widget->pos);
        widget->pos.y -= size.y;
    }
    
    if(widget_has_property(widget, WP_PADDING)){
        v2f size = widget->parent->min;
        v2f padding = widget->min;
        pos.x += (size.width - padding.width)/2.0f;
        pos.y -= (size.height - padding.height)/2.0f;
        
        
        layout_widgets(widget->first_child, pos);
    }
    
    if(widget_has_property(widget, WP_COLUMN)){
        return layout_column(widget->first_child, pos);
    }
    
    if(widget_has_property(widget, WP_ROW)){
        return layout_row(widget->first_child, pos);
    }
    
    if(widget_has_property(widget, WP_WIDTHFILL)){
        
        return layout_widthfill(widget->first_child, pos);
    }
    
    widget->pos = pos;
    
    return widget->min;
}

internal void
render_widgets(Widget* widget){
    if(!widget) return;
    
    if(widget_has_property(widget, WP_WINDOW)){
        widget->pos.y -= widget->min.height;
        v4f bbox = v4f2(widget->pos, widget->min);
        push_rectangle(bbox, 3, ui->theme.panel);
        
    }
    if(widget_has_property(widget, WP_CLIP)){
        RENDER_CLIP(v4f2(widget->pos, widget->min)){
            ForEachWidgetChild(widget){
                render_widgets(it);
            }
        }
    }else {
        ForEachWidgetChild(widget){
            render_widgets(it);
        }
    }
    
    if(widget_has_property(widget, WP_PADDING)){
        v4f bbox = v4f2(widget->pos, widget->min);
    }
    
    if(widget_has_property(widget, WP_RENDER_TEXT)){
        widget->pos.y -= widget->min.height;
        v4f bbox = v4f2(widget->pos, widget->min);
        if(widget_has_property(widget, WP_RENDER_BACKGROUND)){
            push_rectangle(bbox, 1, ui->theme.panel);
        }
        if(widget_has_property(widget, WP_RENDER_BORDER)){
            f32 border_size = 0;
            push_rectangle_outline(bbox, 1, 3, ui->theme.text);
            bbox.x += 4;
        }
        push_string(bbox.pos, widget->string, ui->theme.text);
    }
    
}

internal b32
button(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&ui->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_TEXT);
    widget_set_property(widget, WP_RENDER_BORDER);
    auto result = update_widget(widget);
    return result.clicked;
}


internal void
label(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&ui->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_TEXT);
}