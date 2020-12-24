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
has_left_clicked_dont_consume(){
    Platform_Event* event = 0;
    b32 result = has_left_clicked(&event);
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

internal b32
has_middle_clicked(Platform_Event **event_out){
    b32 result = 0;
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_MOUSE_PRESS && event->mouse_button == MOUSE_BUTTON_MIDDLE){
            *event_out = event;
            result = 1;
        }
    }
    return(result);
}

internal b32
has_middle_clicked(){
    Platform_Event* event = 0;
    b32 result = has_middle_clicked(&event);
    if (result){
        platform_consume_event(event);
    }
    return(result);
}

internal b32
has_mouse_moved(Platform_Event **event_out){
    b32 result = 0;
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_MOUSE_MOVE){
            *event_out = event;
            result = 1;
        }
    }
    return(result);
}

internal b32
has_mouse_moved(v2f* delta = 0) {
    Platform_Event* event = 0;
    b32 result = has_mouse_moved(&event);
    if(result){
        platform_consume_event(event);
        if(delta){
            *delta = event->delta;
        }
    }
    return result;
}

internal b32
has_pressed_key(Platform_Event** event_out, Key key){
    b32 result = 0;
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        
        if (event->type == PLATFORM_EVENT_KEY_PRESS){
            if(event->key == key){
                *event_out = event;
                result = 1;
            }
        }
    }
    return(result);
}

internal b32
has_pressed_key(Key key){
    Platform_Event *event = 0;
    b32 result = has_pressed_key(&event, key);
    if (result){
        platform_consume_event(event);
    }
    return(result);
}

internal b32
has_pressed_key_modified(Platform_Event** event_out, Key key, Key_Modifiers modifiers){
    b32 result = 0;
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        
        if (event->type == PLATFORM_EVENT_KEY_PRESS){
            if(event->key == key && event->modifiers == modifiers){
                *event_out = event;
                result = 1;
            }
        }
    }
    return(result);
}

internal b32
has_pressed_key_modified(Key key, Key_Modifiers modifiers){
    Platform_Event *event = 0;
    b32 result = has_pressed_key_modified(&event, key, modifiers);
    if (result){
        platform_consume_event(event);
    }
    return(result);
}


internal b32
has_mouse_dragged(Platform_Event **event_out, Mouse_Button button, v2f* delta){
    b32 result = 0;
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_MOUSE_DRAG && button == event->mouse_button){
            *event_out = event;
            platform_consume_event(event);
            if(delta){
                delta->x += event->delta.x;
                delta->y += event->delta.y;
            }
            
            result = 1;
        }
    }
    return result;
}

internal b32
has_mouse_dragged(Mouse_Button button, v2f* delta = 0) {
    Platform_Event* event = 0;
    b32 result = has_mouse_dragged(&event, button, delta);
    return result;
}

internal b32
has_mouse_scrolled(Platform_Event **event_out, v2f* delta){
    b32 result = 0;
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_MOUSE_SCROLL){
            *event_out = event;
            platform_consume_event(event);
            if(delta){
                delta->x += event->scroll.x;
                delta->y += event->scroll.y;
            }
            result = 1;
        }
    }
    return result;
}

internal b32
has_mouse_scrolled(v2f* delta = 0) {
    Platform_Event* event = 0;
    b32 result = has_mouse_scrolled(&event, delta);
    return result;
}


internal b32
has_input_character(Platform_Event **event_out){
    b32 result = 0;
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_CHARACTER_INPUT){
            *event_out = event;
            result = 1;
        }
    }
    
    return(result);
}

internal void
load_theme_ayu(){
    
#if 0    
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
#endif
    
}

internal void
load_theme_dots(){
    
    ui->theme.background.packed = 0x121520ff;
    ui->theme.text.packed = 0xE7E7E7ff;
    
    ui->theme.sub_colour.packed = 0x676e8aff;
    ui->theme.border.packed = 0xE7E7E7ff;
    
    ui->theme.text_comment.packed = 0xffc2d94d;
    ui->theme.text_literal.packed = 0x54CB8Fff;
    ui->theme.text_function.packed = 0x47C7F3ff;
    ui->theme.text_type.packed = 0xFED35Eff;
    ui->theme.text_misc.packed = 0x676E88ff;
    
    ui->theme.cursor.packed = 0xe08c17ff;
    ui->theme.error.packed = 0xffcc3333;
    
    
}

internal void
load_theme_dracula(){
    
    ui->theme.background.packed = 0x282A35ff;
    ui->theme.text.packed = 0xF2F2F2ff;
    
    ui->theme.sub_colour.packed = 0x676e8aff;
    ui->theme.border.packed = 0xE7E7E7ff;
    
    ui->theme.text_comment.packed = 0xffc2d94d;
    ui->theme.text_literal.packed = 0x5FFA87ff;
    ui->theme.text_function.packed = 0x90EAFAff;
    ui->theme.text_type.packed = 0xFDB677ff;
    ui->theme.text_misc.packed = 0x676E88ff;
    
    ui->theme.cursor.packed = 0xe08c17ff;
    ui->theme.error.packed = 0xffcc3333;
    
    
}

#define HASH_INITIAL 2166136261

internal void
hash32(u64* hash, char* data, int size){
    
    while(size--){
        *hash = (*hash ^ *data++) * 16777619;
    }
}

internal void
hash32(u64* hash, String8 string){
    
    while(string.length--){
        *hash = (*hash ^ *string.text++) * 16777619;
    }
}

internal void
push_id(UI_ID id){
    auto next_id = push_type_zero(&platform->frame_arena, ID_Node);
    if(ui->id_stack){
        hash32(&id, (char*)&ui->id_stack->id, sizeof(UI_ID));
        next_id->prev = ui->id_stack;
        ui->id_stack = next_id;
    }else {
        ui->id_stack = next_id;
    }
    next_id->id = id;
    
}

internal void
pop_id(){
    if(ui->id_stack){
        ui->id_stack = ui->id_stack->prev;
    }
}

internal UI_ID
generate_id(String8 label){
    int size = label.length;
    UI_ID id = 0;
    hash32(&id, label);
    if(ui->id_stack){
        hash32(&id, (char*)&ui->id_stack->id, sizeof(UI_ID));
    }
    return id;
}

internal UI_ID
generate_id(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    UI_ID id = 0;
    
    hash32(&id, string.text, string.length);
    if(ui->id_stack){
        hash32(&id, (char*)&ui->id_stack->id, sizeof(UI_ID));
    }
    return id;
}

internal bool
widget_has_property(Widget* widget, Widget_Property property){
    return !!(widget->properties[property / 64] & (1ll << (property % 64)));
}

internal void
widget_set_property(Widget* widget, Widget_Property property){
    widget->properties[property / 64] |= (1ll << (property % 64));
}

internal void
widget_remove_property(Widget* widget, Widget_Property property){
    widget->properties[property / 64] &= ~(1ll << (property % 64));
}

internal Widget*
get_widget(String8 string){
    if(string.length == 0) return nullptr;
    
    auto id = generate_id(string);
    auto hash = id & (MAX_TABLE_WIDGETS-1);
    Widget* widget = ui->widget_table[platform->frame][hash];
    
    if(!widget){
        widget = push_type_zero(&platform->frame_arena, Widget);
        ui->widget_table[platform->frame][hash] = widget;
    }else{
        
        do {
            if(string_eq(string, widget->string)){
                break;
            }
            if(!widget->next_hash){
                auto next_widget = push_type_zero(&platform->frame_arena, Widget);
                widget->next_hash = next_widget;
                ui->widget_table[platform->frame][hash] = next_widget;
                widget = next_widget;
                break;
            }
            
            widget = widget->next_hash;
        }while(widget);
    }
    
    if(ui->widget_table[!platform->frame]){
        auto last_widget = ui->widget_table[!platform->frame][hash];
        if(!last_widget){
            return widget;
        }
        
        do {
            if(string_eq(string, last_widget->string)){
                break;
            }
            last_widget = last_widget->next_hash;
        }while(last_widget);
        
        if(last_widget){
            widget->checked = last_widget->checked;
            widget->style = last_widget->style;
            widget->min = last_widget->min;
            widget->max = last_widget->max;
            widget->pos = last_widget->pos;
            widget->hot_transition = last_widget->hot_transition;
            widget->active_transition = last_widget->active_transition;
            widget->value = last_widget->value;
            widget->string = string;
            widget->id = id;
        }
        
    }
    
    return widget;
}

internal Widget*
make_widget(){
    auto widget = push_type_zero(&platform->frame_arena, Widget);
    return widget;
}

internal Widget*
make_widget(String8 string){
    auto widget = get_widget(string);
    widget->id = generate_id(string);
    widget->string = string;
    return widget;
}

internal Layout*
push_layout(Widget* widget){
    auto layout = push_type_zero(&platform->frame_arena, Layout);
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
    auto widget = make_widget();
    
    if(!ui->layout_stack && !ui->root){
        ui->root = widget;
        return widget;
    }else if(!ui->layout_stack && ui->root) {
        Widget* last = ui->root;
        for(; last->next_sibling; last = last->next_sibling);
        last->next_sibling = widget;
        widget->prev_sibling = last;
        
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
    auto widget = make_widget(string);
    
    if(!ui->layout_stack && !ui->root){
        ui->root = widget;
        return widget;
    }else if(!ui->layout_stack && ui->root) {
        Widget* last = ui->root;
        for(; last->next_sibling; last = last->next_sibling);
        last->next_sibling = widget;
        widget->prev_sibling = last;
        
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
    
    widget->string = string;
    return widget;
}


internal void present_cursor();

internal Widget_Update
update_widget(Widget* widget){
    Widget_Update result = {};
    
    Widget* last_widget = get_widget(widget->string);
    
    // NOTE(Oliver): gotta stop special casing for the widgets
    if(widget_has_property(widget, WP_WINDOW) ||
       widget_has_property(widget, WP_MANUAL_LAYOUT)){
        if(has_left_clicked_dont_consume()){
            ui->active = widget->id;
        }
    }
    else if(last_widget && widget_has_property(widget, WP_CLICKABLE)){
        v4f bbox = v4f2(last_widget->pos, last_widget->min);
        bbox.y -= bbox.height; //we draw widgets from top left not bottom left
        if(is_in_rect(platform->mouse_position, bbox) || ui->active == widget->id){
            v2f delta = {};
            if(!widget_has_property(widget, WP_CONTAINER) && has_left_clicked()){
                ui->active = widget->id;
                result.clicked = true;
                result.clicked_position = platform->mouse_position;
                result.pos = bbox.pos;
                result.size = bbox.size;
            }
            else if(has_mouse_dragged(MOUSE_BUTTON_LEFT, &delta)){
                log("potato");
                ui->active = widget->id;
                result.dragged = true;
                result.delta = delta;
                result.pos = bbox.pos;
                result.size = bbox.size;
            }
        }
    }
    
    if(widget_has_property(widget, WP_TEXT_EDIT)){
        clampi(&ui->cursor_pos, 0, ui->editing_string.length);
        if(has_pressed_key_modified(KEY_LEFT, KEY_MOD_CTRL)){
            if(ui->editing_string.text[ui->cursor_pos-1] == ' '){
                while(ui->editing_string.text[ui->cursor_pos-1] == ' '){
                    ui->cursor_pos--;
                }
            }else {
                while(ui->editing_string.text[ui->cursor_pos-1] != ' ' &&
                      ui->cursor_pos >= 0){
                    ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
                }
            }
        }
        
        if(has_pressed_key_modified(KEY_RIGHT, KEY_MOD_CTRL)){
            if(ui->editing_string.text[ui->cursor_pos] == ' '){
                while(ui->editing_string.text[ui->cursor_pos] == ' '){
                    ui->cursor_pos++;
                }
            }else{
                while(ui->editing_string.text[ui->cursor_pos] != ' ' &&
                      ui->cursor_pos <= ui->editing_string.length){
                    ui->cursor_pos++;
                }
            }
        }
        
        if(has_pressed_key(KEY_LEFT)){
            ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
        }
        if(has_pressed_key(KEY_RIGHT)){
            ui->cursor_pos = ui->cursor_pos < ui->editing_string.length ? ui->cursor_pos +1: ui->editing_string.length;
        }
        if(has_pressed_key_modified(KEY_BACKSPACE, KEY_MOD_CTRL)){
            if(ui->editing_string.text[ui->cursor_pos-1] == ' '){
                ui->cursor_pos--;
            }else {
                while(ui->editing_string.text[ui->cursor_pos-1] != ' ' &&
                      ui->cursor_pos >= 0){
                    pop_from_string(&ui->editing_string.string, ui->cursor_pos);
                    ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
                }
            }
        }
        
        if(has_pressed_key(KEY_BACKSPACE)){
            pop_from_string(&ui->editing_string.string, ui->cursor_pos);
            ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
        }
        
        if(has_pressed_key(KEY_END)){
            ui->cursor_pos = ui->editing_string.length;
        }
        
        if(has_pressed_key(KEY_HOME)){
            ui->cursor_pos = 0;
        }
        
        Platform_Event* event = 0;
        for (;platform_get_next_event(&event);){
            if (event->type == PLATFORM_EVENT_CHARACTER_INPUT){
                insert_in_string(&ui->editing_string.string,
                                 &(char)event->character,
                                 ui->cursor_pos++);
                platform_consume_event(event);
            }
        }
    }
    
    if(widget_has_property(widget, WP_RENDER_HOOK)){
        widget->min = get_text_bbox({}, widget->string).size;
    }
    
    if(widget_has_property(widget, WP_RENDER_TEXT)){
        widget->min = get_text_bbox({}, widget->string).size;
    }
    
    if(widget_has_property(widget, WP_RENDER_TRIANGLE)){
        v2f size = get_text_bbox({}, "V").size;
        size.width = size.height;
        widget->min = size;
    }
    
    if(widget_has_property(widget, WP_ROW)){
        widget->min = widget->parent->min;
    }
    
    if(widget_has_property(widget, WP_WRAP)){
        widget->min = widget->parent->min;
    }
    
    if(widget_has_property(widget, WP_COLUMN)){
        widget->min = widget->parent->min;
    }
    
    if(widget_has_property(widget, WP_WIDTHFILL)){
        widget->min = widget->parent->min;
    }
    
    if(widget_has_property(widget, WP_FIXED_SIZE)){
        widget->max = widget->min;
    }
    
    return result;
}

internal void
pop_layout(){
    if(!ui->layout_stack) return;
    
    ui->layout_stack = ui->layout_stack->prev;
}

internal void
push_widget_wrap(){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_WRAP);
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_LERP_POSITION);
    update_widget(widget);
}

internal void
push_widget_row(){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_ROW);
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_LERP_POSITION);
    update_widget(widget);
}

internal void
push_widget_column(){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_COLUMN);
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_LERP_POSITION);
    update_widget(widget);
}

internal void
push_widget_widthfill(){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_WIDTHFILL);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_SPACING);
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
push_widget_window(v4f rect, String8 string){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    
    // NOTE(Oliver): i must be able to just call
    // push_widget(string)???????????
    widget->id = generate_id(string);
    widget->string = string;
    
    widget_set_property(widget, WP_WINDOW);
    widget_set_property(widget, WP_CLIP);
    widget_set_property(widget, WP_RENDER_BACKGROUND);
    widget_set_property(widget, WP_CLICKABLE);
    
    widget->min = rect.size;
    widget->pos = rect.pos;
    update_widget(widget);
    push_widget_padding(v2f(10, 10));
    
}

internal void
pop_widget_window(){
    while(ui->layout_stack) pop_layout();
}

internal void
push_widget_container(String8 string){
    auto widget = push_widget(string);
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_CONTAINER);
    widget_set_property(widget, WP_RENDER_BACKGROUND);
    widget_set_property(widget, WP_RENDER_CORNERS);
    widget_set_property(widget, WP_DRAGGABLE);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_SCROLLING);
    widget_set_property(widget, WP_CLICKABLE);
    auto result = update_widget(widget);
    push_widget_padding(v2f(10, 10));
    if(result.dragged){
        widget->pos.x += result.delta.x;
        widget->pos.y += result.delta.y;
    }
}

internal void
pop_widget_container(){
    pop_layout();
    pop_layout();
    pop_layout();
}

internal v2f layout_widgets(Widget* widget, v2f pos);

#define PADDING 10
#define V2PADDING v2f(PADDING, PADDING)

internal v2f
layout_row(Widget* widget, v2f pos){
    
    v2f size = {};
    v2f start_pos = pos;
    v2f next_size = {};
    ForEachWidgetSibling(widget){
        if(widget_has_property(it, WP_WIDTHFILL)){
            it->min.x = it->parent->min.x - (pos.x - start_pos.x);
        }
        v2f next_size = layout_widgets(it, pos);
        if(next_size.x >= it->parent->pos.x + it->parent->min.width){
            pos.y -= next_size.height;
            pos.x = start_pos.x;
            next_size = layout_widgets(it, pos);
        }
        if(widget_has_property(it, WP_SPACING)){
            pos.x += PADDING;
            size.width += PADDING;
        }
        pos.x += next_size.width;
        
        size.width += next_size.width;
        size.height = max(size.height, next_size.height);
    }
    
    return size;
}

internal v2f
layout_wrap(Widget* widget, v2f pos){
    
    v2f size = {};
    v2f start_pos = pos;
    v2f next_size = {};
    f32 width = widget->parent->min.width;
    f32 end = start_pos.x + width;
    ForEachWidgetSibling(widget){
        if(widget_has_property(it, WP_WIDTHFILL)){
            it->min.x = it->parent->min.x - (pos.x - start_pos.x);
        }
        
        v2f next_size = layout_widgets(it, pos);
        if(pos.x + next_size.width >= end){
            if(widget_has_property(it, WP_SPACING)){
                pos.y -= PADDING;
                size.height += PADDING;
            }
            pos.y -= next_size.height;
            pos.x = start_pos.x;
            next_size = layout_widgets(it, pos);
            size.height += next_size.height;
            
        }else {
            
        }
        
        if(widget_has_property(it, WP_SPACING)){
            pos.x += PADDING;
            size.width += PADDING;
        }
        
        pos.x += next_size.width;
        
        size.height = max(size.height, next_size.height);
        size.width += next_size.width;
    }
    
    return size;
}

internal v2f
layout_column(Widget* widget, v2f pos){
    
    v2f size = {};
    
    ForEachWidgetSibling(widget){
        v2f next_size = layout_widgets(it, pos);
        pos.y -= next_size.height;
        if(widget_has_property(it, WP_SPACING)){
            pos.y -= PADDING;
            size.height += PADDING;
        }
        
        size.height += next_size.height;
        
        size.width = max(size.width, next_size.width);
    }
    
    return size;
}

internal v2f
layout_widthfill(Widget* widget, v2f pos){
    
    v2f size = {};
    
    f32 total_width = 0;
    int number_of_children = 0;
    v2f start_pos = pos;
    ForEachWidgetSibling(widget) {
        //v2f next_size = layout_widgets(it, pos);
        if(widget_has_property(it, WP_SPACING)){
            total_width += PADDING;
        }
        
        total_width += it->min.width;
        
        if(!widget_has_property(it, WP_FIXED_SIZE)){
            number_of_children++;
        }
        
    }
    f32 available_space =  widget->parent->min.width;
    f32 width = (available_space - total_width + PADDING)/(f32)number_of_children;
    
    
    width = width >= 0 ? width : 0;
    
    ForEachWidgetSibling(widget) {
        //log("first: %f", it->min.width);
        if(!widget_has_property(it, WP_FIXED_SIZE)){
            it->min.width += width;
        }
        v2f next_size = layout_widgets(it, pos);
        
        if(widget_has_property(widget, WP_SPACING)){
            pos.x += PADDING;
            size.width += PADDING;
        }
        
        pos.x += next_size.width;
        
        size.width += next_size.width;
        size.height = max(size.height, next_size.height);
    }
    
    return size;
}

internal v2f
layout_widgets(Widget* widget, v2f pos = v2f(0,0)){
    if(!widget) return {};
    
    if(widget_has_property(widget, WP_MANUAL_LAYOUT)){
        return {};
    }
    
    if(widget_has_property(widget, WP_WINDOW)){
        pos = widget->pos;
        layout_widgets(widget->first_child, pos);
    }
    
    if(widget_has_property(widget, WP_PADDING)){
        v2f size = widget->parent->min;
        v2f padding = widget->min;
        pos.x += (size.width - padding.width)/2.0f;
        pos.y -= (size.height - padding.height)/2.0f;
        widget->pos = pos;
        widget->min = layout_widgets(widget->first_child, pos);
        widget->min.width += (size.width - padding.width); 
        widget->min.height += (size.height - padding.height); 
        
    }
    
    
    if(widget_has_property(widget, WP_CONTAINER)){
        if(widget->pos.x == 0 && widget->pos.y == 0){
            widget->pos = pos;
        }
        pos = widget->pos;
        pos.y += widget->scroll_amount;
        v2f size = layout_widgets(widget->first_child, pos);
        widget->min = size;
        //log("container size is now: %f %f", size.width, size.height);
    }
    
    if(widget->pos.x && widget->pos.y && widget_has_property(widget, WP_LERP_POSITION)){
        lerp(&widget->pos.x, pos.x, 0.2f);
        lerp(&widget->pos.y, pos.y, 0.2f);
    }else if(!widget_has_property(widget, WP_CONTAINER)){
    }
    widget->pos = pos;
    
    if(widget_has_property(widget, WP_COLUMN)){
        return layout_column(widget->first_child, pos);
    }
    
    if(widget_has_property(widget, WP_WRAP)){
        return layout_wrap(widget->first_child, pos);
    }
    
    if(widget_has_property(widget, WP_ROW)){
        return layout_row(widget->first_child, pos);
    }
    
    if(widget_has_property(widget, WP_WIDTHFILL)){
        return layout_widthfill(widget->first_child, pos);
    }
    
    return widget->min;
}

internal void
widget_render_text(Widget* widget, Colour colour){
    v2f pos = widget->pos;
    pos.y -= widget->min.height;
    v4f bbox = get_text_bbox(pos, widget->string);
    bbox = v4f2(pos, widget->min);
    if(widget_has_property(widget, WP_RENDER_BACKGROUND)){
        push_rectangle(bbox, 1, ui->theme.background);
    }
    if(widget_has_property(widget, WP_RENDER_BORDER)){
        bbox = inflate_rect(bbox, widget->hot_transition*2.5f);
        v4f border_colour;
        if(widget->id == ui->hot){
            border_colour = v4f_from_colour(ui->theme.cursor);
            if(widget_has_property(widget, WP_LERP_COLOURS)){
                lerp_rects(&widget->style.border_colour, border_colour, 0.2f);
            }
        }else {
            border_colour = v4f_from_colour(ui->theme.border);
            if(widget_has_property(widget, WP_LERP_COLOURS)){
                lerp_rects(&widget->style.border_colour, border_colour, 0.05f);
            }
        }
        
        push_rectangle_outline(bbox, 1, 3, colour_from_v4f(widget->style.border_colour));
        widget->pos.x += 1;
        widget->pos.y -= 1;
    }
    
    f32 centre = widget->pos.x + widget->min.x/2.0f;
    f32 text_centre = get_text_width(widget->string)/2.0f;
    f32 text_x = centre - text_centre;
    
    push_string(v2f(text_x, bbox.y), widget->string, colour);
}

internal void
render_widgets(Widget* widget){
    if(!widget) return;
    
    if(widget_has_property(widget, WP_WINDOW)){
        widget->pos.y -= widget->min.height;
        
        v4f bbox = v4f2(widget->pos, widget->min);
        if(widget_has_property(widget, WP_RENDER_BACKGROUND)){
            push_rectangle(bbox, 3, ui->theme.background);
        }
        bbox = inflate_rect(bbox, 1.5);
        push_rectangle_outline(bbox, 1, 3, ui->theme.text);
        
    }
    
    if(widget_has_property(widget, WP_CONTAINER) &&
       widget_has_property(widget, WP_RENDER_CORNERS)){
        v4f bbox = v4f2(widget->pos, widget->min);
        //push_string(widget->pos, widget->string, ui->theme.text, 1.0f + widget->style.font_scale);
        bbox.y -= widget->min.height;
        push_rectangle_outline(bbox, 1, 3, ui->theme.text);
        v4f v = bbox;
        v.width -= PADDING*2;
        v.x += PADDING;
        v.height += PADDING;
        v.y -= PADDING/2.0f;
        push_rectangle(v, 1,ui->theme.background);
        
        v4f h = bbox;
        h.width += PADDING;
        h.x -= PADDING/2.0f;
        h.height -= PADDING*2;
        h.y += PADDING;
        push_rectangle(h, 1,ui->theme.background);
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
    
    v2f pos = widget->pos;
    pos.y -= widget->min.height;
    
    if(ui->hot == widget->id || is_in_rect(platform->mouse_position, v4f2(pos, widget->min))){
        lerp(&widget->hot_transition, 1.0f, 0.1f);
        ui->hot = widget->id; 
    }else {
        lerp(&widget->hot_transition, 0, 0.1f);
    }
    
    if(widget_has_property(widget, WP_RENDER_TEXT)){
        widget_render_text(widget, ui->theme.text);
    }
    if(widget_has_property(widget, WP_RENDER_TRIANGLE)){
        v4f bbox = v4f2(pos, widget->min);
        bbox = inflate_rect(bbox, widget->hot_transition*2.5f);
        if(widget_has_property(widget, WP_RENDER_BORDER)){
            if(widget->id == ui->hot){
                push_rectangle_outline(bbox, 1, 3, ui->theme.cursor);
            }else{ 
                push_rectangle_outline(bbox, 1, 3, ui->theme.border);
            }
            bbox.pos.x += 1;
            bbox.pos.y -= 1;
        }
        bbox.pos.x += bbox.size.width/6.0f;
        bbox.pos.y += bbox.size.width/3.0f;
        push_triangle(bbox.pos, bbox.size.width/3.0f, ui->theme.text);
    }
    
    if(widget_has_property(widget, WP_RENDER_HOOK)){
        widget->render_hook(widget);
    }
}

internal void
label(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_TEXT);
    widget_set_property(widget, WP_LERP_POSITION);
    update_widget(widget);
}

internal b32
arrow_dropdown(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    auto widget = push_widget(string);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_RENDER_TRIANGLE);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_FIXED_SIZE);
    widget_set_property(widget, WP_SPACING);
    auto result = update_widget(widget);
    if(result.clicked){
        widget->checked = !widget->checked;
    }
    return widget->checked;
}

internal b32
button(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    Widget* widget = push_widget(string);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_RENDER_TEXT);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_LERP_COLOURS);
    widget_set_property(widget, WP_LERP_POSITION);
    auto result = update_widget(widget);
    return result.clicked;
}

internal b32
check(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    Widget* widget;
    widget = push_widget(string);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_RENDER_TEXT);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_SPACING);
    auto result = update_widget(widget);
    if(result.clicked){
        widget->checked = !widget->checked;
    }
    return widget->checked;
}

internal b32
check(b32* checked, char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    Widget* widget;
    widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_TEXT);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_SPACING);
    auto result = update_widget(widget);
    if(result.clicked){
        *checked = !*checked;
    }
    return *checked;
}

internal b32
button_fixed(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_RENDER_TEXT);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_RENDER_BACKGROUND);
    widget_set_property(widget, WP_FIXED_SIZE);
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_LERP_COLOURS);
    auto result = update_widget(widget);
    return result.clicked;
}

internal void
xspacer(f32 space = 10.0f){
    
    auto widget = push_widget();
    widget->min = v2f(space, 0);
    widget->pos = v2f(0,0);
    widget_set_property(widget, WP_SPACING);
    update_widget(widget);
}

internal void
yspacer(f32 space = 10.0f){
    
    auto widget = push_widget();
    widget->min = v2f(0, space);
    widget_set_property(widget, WP_SPACING);
    update_widget(widget);
}

internal void
fslider(f32 min, f32 max, f32* value, char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_LERP_COLOURS);
    
    auto render_hook = [](Widget* widget) {
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        v4f bbox = v4f2(pos, widget->min);
        
        {
            v4f pc = bbox;
            pc.width *= (widget->value);
            push_rectangle(pc, 1, ui->theme.sub_colour);
        }
        if(widget_has_property(widget, WP_RENDER_BORDER)){
            bbox = inflate_rect(bbox, widget->hot_transition*2.5f);
            v4f border_colour;
            if(widget->id == ui->hot){
                border_colour = v4f_from_colour(ui->theme.cursor);
                if(widget_has_property(widget, WP_LERP_COLOURS)){
                    lerp_rects(&widget->style.border_colour, border_colour, 0.2f);
                }
            }else {
                border_colour = v4f_from_colour(ui->theme.border);
                if(widget_has_property(widget, WP_LERP_COLOURS)){
                    lerp_rects(&widget->style.border_colour, border_colour, 0.05f);
                }
            }
            push_rectangle_outline(bbox, 1, 3, colour_from_v4f(widget->style.border_colour));
            
            pos.x += 1;
            pos.y -= 1;
        }
        f32 centre = pos.x + widget->min.x/2.0f;
        String8 string = make_stringf(&platform->frame_arena, "%.2f", widget->value);
        f32 text_centre = get_text_width(string)/2.0f;
        f32 text_x = centre - text_centre;
        push_string(v2f(text_x, bbox.y), string, ui->theme.text);
    };
    
    widget->render_hook = render_hook;
    widget->value = (*value - min)/(max - min);
    auto result = update_widget(widget);
    if(result.clicked){
        f32 x = (result.clicked_position.x - result.pos.x)/result.size.width;
        *value = min + x*(max-min);
        widget->value = (*value - min)/(max - min);
    }
    if(result.dragged){
        f32 x = result.delta.x/result.size.width;
        *value += x*(max-min);
        clampf(value, min, max);
        widget->value = (*value - min)/(max - min);
    }
}

internal void
ui_window(v4f rect, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    push_widget_window(rect, string);
    push_widget_column();
}

internal void
ui_container(char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    push_widget_container(string);
    push_widget_column();
}

internal void
ui_panel_header(Panel* panel, char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    
    b32 dropdown = false;
    UI_ROW {
        label("%.*s", string.length, string.text);
        UI_WIDTHFILL{
            xspacer(20);
            dropdown = arrow_dropdown("change type%.*s", string.length, string.text);
        }
        
    }
    
    ID("%d", (int)panel) {
        if(dropdown){
            UI_WRAP {
                if(button_fixed("properties")){
                    panel->type = PANEL_PROPERTIES;
                }
                if(button_fixed("code editor")){
                    panel->type = PANEL_EDITOR;
                }
                if(button_fixed("debug info")){
                    panel->type = PANEL_DEBUG;
                }
            }
        }
    }
}

internal void
update_panel_split(Panel* parent, f32 split_ratio){
    parent->first->split_ratio = split_ratio;
    parent->second->split_ratio = 1.0f - split_ratio;
}

internal void
ui_panel_resize_widgets(Panel* panel, v4f rect, char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    rect = inflate_rect(rect, 10);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_DRAGGABLE);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_MANUAL_LAYOUT);
    
    
    widget->pos = rect.pos;
    widget->min = rect.size;
    
    auto result = update_widget(widget);
    
    if(result.dragged){
        log("potato");
        f32 delta = result.delta.x;
        f32 divisor = rect.width/panel->split_ratio;
        update_panel_split(panel->parent, platform->mouse_position.x/platform->window_size.x);
    }
}

internal b32
dropdown(char* fmt, ...){
    b32 result = 0;
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    UI_ROW {
        label("%.*s", string.length, string.text);
        result = arrow_dropdown("change type%.*s", string.length, string.text);
    }
    return result;
}

internal void
text_box(String8* string){
    auto widget = push_widget(*string);
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_LERP_COLOURS);
    widget_set_property(widget, WP_TEXT_EDIT);
    
    auto render_hook = [](Widget* widget) {
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        v4f bbox = v4f2(pos, widget->min);
        
        if(widget_has_property(widget, WP_RENDER_BORDER)){
            bbox = inflate_rect(bbox, widget->hot_transition*2.5f);
            v4f border_colour;
            if(widget->id == ui->hot){
                border_colour = v4f_from_colour(ui->theme.cursor);
                if(widget_has_property(widget, WP_LERP_COLOURS)){
                    lerp_rects(&widget->style.border_colour, border_colour, 0.2f);
                }
            }else {
                border_colour = v4f_from_colour(ui->theme.border);
                if(widget_has_property(widget, WP_LERP_COLOURS)){
                    lerp_rects(&widget->style.border_colour, border_colour, 0.05f);
                }
            }
            push_rectangle_outline(bbox, 1, 3, colour_from_v4f(widget->style.border_colour));
            
            pos.x += 1;
            pos.y -= 1;
        }
        
        if(ui->active == widget->id){
            String8 s = make_stringf(&platform->frame_arena, "%.*s", ui->editing_string.length, ui->editing_string.text);
            f32 centre = pos.x + widget->min.x/2.0f;
            f32 text_centre = get_text_width(s)/2.0f;
            f32 text_x = centre - text_centre;
            push_string(v2f(text_x, bbox.y), s, ui->theme.text);
            
            v2f cursor = {};
            cursor.x = text_x + get_text_width_n(s, ui->cursor_pos);
            cursor.y = bbox.y;
            push_rectangle(v4f2(cursor, v2f(2, widget->min.height)), 1, ui->theme.cursor);
        }else {
            f32 centre = pos.x + widget->min.x/2.0f;
            f32 text_centre = get_text_width(widget->string)/2.0f;
            f32 text_x = centre - text_centre;
            push_string(v2f(text_x, bbox.y), widget->string, ui->theme.text);
        }
    };
    
    widget->render_hook = render_hook;
    
    auto result = update_widget(widget);
    if(result.clicked){
        memcpy(ui->editing_string.text, string->text, string->length);
        ui->editing_string.length = string->length;
        ui->cursor_pos = string->length;
    }else {
        
    }
}

internal void 
split_panel(Panel* panel, f32 split_ratio, Panel_Split_Type split_type, Panel_Type type){
    if(!panel) return;
    
    assert(!panel->first && !panel->second);
    
    panel->first = (Panel*)push_type_zero(&platform->permanent_arena, Panel);
    panel->second = (Panel*)push_type_zero(&platform->permanent_arena, Panel);
    
    panel->first->split_ratio = split_ratio;
    panel->second->split_ratio = 1.0f- split_ratio;
    
    panel->first->parent = panel;
    panel->second->parent = panel;
    
    panel->first->type = panel->type;
    panel->second->type = type;
    
    panel->first->split_type = split_type;
    panel->second->split_type = split_type;
}

internal void present_keyword(char* fmt, ...);
internal void present_literal(char* fmt, ...);
internal void present_function(char* fmt, ...);
internal void present_id(char* fmt, ...);
internal void present_misc(char* fmt, ...);
internal void present(int present_style);

static int present_style;

internal void
render_panels(Panel* root, v4f rect){
    if(!root) return;
    
    if(root->first && root->second){
        switch(root->first->split_type){
            case PANEL_SPLIT_VERTICAL:{
                v4f first = rect;
                first.width *= root->first->split_ratio;
                render_panels(root->first, first);
                
                v4f second = rect;
                second.x += rect.width*root->first->split_ratio;
                second.width *= root->second->split_ratio;
                render_panels(root->second, second);
            }break;
            case PANEL_SPLIT_HORIZONTAL:{
                v4f first = rect;
                first.height *= root->first->split_ratio;
                render_panels(root->first, first);
                
                v4f second = rect;
                second.y -= rect.height*root->first->split_ratio;
                second.height *= root->second->split_ratio;
                render_panels(root->second, second);
            }break;
        }
    }else {
        
        assert(!root->first && !root->second);
        
        rect.x += PADDING;
        rect.y -= PADDING;
        rect.width -= PADDING*2;
        rect.height -= PADDING*2;
        if(root->type == PANEL_PROPERTIES){
            UI_WINDOW(rect, "Properties#%d", (int)root) {
                ui_panel_header(root, "Properties#%d", (int)root);
                ui_panel_resize_widgets(root, rect, "splitter");
                
                UI_COLUMN ID("%d", (int)root) {
                    label("Syntax Style");
                    UI_WIDTHFILL { if(button("Render as C")) present_style = 0;}
                    UI_WIDTHFILL { if(button("Render as Jai")) present_style = 1;}
                    UI_WIDTHFILL { if(button("Render as Python")) present_style = 2;}
                    UI_WIDTHFILL { if(button("Render as Pascal")) present_style = 3;}
                    local_persist v4f rect  = {};
                    local_persist String8 text_string  = make_string("cool beans");
                    yspacer(20);
                    
                    UI_WIDTHFILL{
                        fslider(0, 1, &rect.r, "R");
                        fslider(0, 1, &rect.g, "G");
                        fslider(0, 1, &rect.b, "B");
                        fslider(0, 1, &rect.a, "A");
                    }
                    
                    UI_WIDTHFILL {
                        text_box(&text_string);
                    }
                    
                    yspacer(20);
                    UI_ROW UI_WIDTHFILL {
                        button("Compile");
                        button("Run");
                    }
                }
            }
            
            
        }else if(root->type == PANEL_EDITOR) {
            UI_WINDOW(rect, "Code Editor#%d", (int)root) {
                ID("%d", (int)root) {
                    ui_panel_header(root, "Code Editor#%d", (int)root);
                    UI_CONTAINER("snippet"){
                        if(dropdown("Snippet#%d", (int)root)){
                            present(present_style);
                        }
                    }
                }
            }
        }else if(root->type == PANEL_STATUS){
            UI_WINDOW(rect, "Status#%d", (int)root) {
                UI_ROW {
                    label("mouse position:"); 
                    label("%.0f %.0f", platform->mouse_position.x, platform->mouse_position.y);
                }
            }
        }else if(root->type == PANEL_DEBUG){
            UI_WINDOW(rect, "Debug#%d", (int)root){
                ID("%d", (int)root) {
                    ui_panel_header(root, "Debug");
                    if(dropdown("test dropdown")){
                        UI_COLUMN {
                            button("this");
                            button("can");
                            button("be");
                            button("used");
                            button("to");
                            button("debug");
                        }
                    }
                    if(dropdown("secrets")){
                        
                    }
                }
            }
        }
        
    }
}