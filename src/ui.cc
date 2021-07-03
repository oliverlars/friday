global UI_State* ui = 0;

internal b32
has_mouse_released(Platform_Event **event_out, Mouse_Button button){
    b32 result = 0;
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_MOUSE_RELEASE && event->mouse_button == button){
            *event_out = event;
            result = 1;
        }
    }
    return(result);
}

internal b32
has_left_released(){
    Platform_Event* event = 0;
    b32 result = has_mouse_released(&event, MOUSE_BUTTON_LEFT);
    if (result){
        platform_consume_event(event);
    }
    return(result);
}

internal b32
has_middle_released(){
    Platform_Event* event = 0;
    b32 result = has_mouse_released(&event, MOUSE_BUTTON_MIDDLE);
    if (result){
        platform_consume_event(event);
    }
    return(result);
}

internal b32
has_left_released_dont_consume(){
    Platform_Event* event = 0;
    b32 result = has_mouse_released(&event, MOUSE_BUTTON_LEFT);
    
    return(result);
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
is_left_down(){
    return platform->is_left_down;
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
is_middle_down(Platform_Event **event_out){
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
is_middle_down(){
    Platform_Event* event = 0;
    b32 result = is_middle_down(&event);
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
has_mouse_scrolled(Platform_Event **event_out){
    b32 result = 0;
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_MOUSE_SCROLL){
            *event_out = event;
            result = 1;
        }
    }
    return(result);
}

internal b32
has_mouse_scrolled(f32* amount = 0) {
    Platform_Event* event = 0;
    b32 result = has_mouse_scrolled(&event);
    if(result){
        platform_consume_event(event);
        if(amount){
            *amount = event->scroll.y;
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
has_left_dragged(v2f* delta = 0){
    b32 result = 0;
    
    Platform_Event* move_event = 0;
    result = has_mouse_moved(&move_event);
    result = result & is_left_down();
    
    if(result){
        platform_consume_event(move_event);
        if(delta) *delta = move_event->delta;
    }
    return result;
}


internal b32
has_middle_dragged(v2f* delta = 0){
    b32 result = 0;
    Platform_Event* middle_event = 0;
    Platform_Event* move_event = 0;
    result = has_mouse_moved(&move_event);
    result = result & is_middle_down(&middle_event);
    if(result){
        platform_consume_event(middle_event);
        platform_consume_event(move_event);
        if(delta) *delta = move_event->delta;
    }
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
            if(event_out){
                *event_out = event;
            }
            result = 1;
        }
    }
    
    return(result);
}


internal f32
animate(f32 source, f32 target, f32 value){
    return lerp(source, target, value);
    value = value/2.0f;
    return (target - source)*(1.0f - exp(-value*platform->dt));
}

internal void
animate(f32* source, f32 target, f32 value){
    return lerp(source, target, value);
    value = value/2.0f;
    *source += (target - *source)*(1.0f - exp(-value*platform->dt));
}

internal void
animate(int* source, int target, f32 value){
    return lerp(source, target, value);
    value = value/2.0f;
    *source += (target - *source)*(1.0f - exp(-value*platform->dt));
}


internal void
load_theme_ayu(){
    
#if 0    
    ui->theme.background.packed = 0x151826ff;
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
    
    ui->theme.background.packed = 0x1C1F33ff;
    ui->theme.darker_background.packed = 0x303851ff;
    ui->theme.text.packed = 0xE7E7E7ff;
    
    ui->theme.sub_colour.packed = 0x676e8aff;
    ui->theme.border.packed = 0xE7E7E7ff;
    
    ui->theme.select.packed = 0x85C3E5ff;
    
    ui->theme.text_comment.packed = 0xffc2d94d;
    ui->theme.text_literal.packed = 0x54CB8Fff;
    ui->theme.text_function.packed = 0x47C7F3ff;
    ui->theme.text_type.packed = 0xFED35Eff;
    ui->theme.text_misc.packed = 0x676E88ff;
    
    ui->theme.cursor.packed = 0xe08c17ff;
    ui->theme.error.packed = 0xffcc3333;
    
    
}

internal void
load_theme_matrix(){
    
    ui->theme.background.packed = 0x051812ff;
    ui->theme.text.packed = 0x169B48ff;
    
    ui->theme.sub_colour.packed = 0x676e8aff;
    ui->theme.border.packed = 0x35F192ff;
    
    ui->theme.select.packed = 0x85C3E5ff;
    
    ui->theme.text_comment.packed = 0xffc2d94d;
    ui->theme.text_literal.packed = 0x54CB8Fff;
    ui->theme.text_function.packed = 0x47C7F3ff;
    ui->theme.text_type.packed = 0xFED35Eff;
    ui->theme.text_misc.packed = 0x676E88ff;
    
    ui->theme.cursor.packed = 0xe08c17ff;
    ui->theme.error.packed = 0xffcc3333;
    
    
}

internal void
load_theme_dedouze(){
    ui->theme.background.packed = 0x1B5CCFff;
    ui->theme.text.packed = 0xFFFFC6ff;
    
    
    ui->theme.sub_colour.packed = 0x676e8aff;
    ui->theme.border.packed = 0xFFFFC6ff;
    
    
    ui->theme.text_comment.packed = 0x71ABFAff;
    ui->theme.text_literal.packed = 0x71ABFAff;
    ui->theme.text_function.packed = 0xF8BBF9ff;
    ui->theme.text_type.packed = 0x9DFFF6ff;
    ui->theme.text_misc.packed = 0x59337Dff;
    
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
    ;
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

internal void
push_style(Widget_Style style){
    
    auto next_style = push_type_zero(&platform->frame_arena, Style_Node);
    
    if(ui->style_stack){
        next_style->prev = ui->style_stack;
        ui->style_stack = next_style;
    }else {
        ui->style_stack = next_style;
    }
    
    next_style->style = style;
}


internal void
push_default_style(){
    
    Widget_Style style = {};
    style.text_colour = v4f_from_colour(ui->theme.text);
    style.border_colour = v4f_from_colour(ui->theme.text);
    style.background_colour = v4f_from_colour(ui->theme.background);
    style.font_scale = 0.75f;
    style.rounded_corner_amount = 3.0f;
    push_style(style);
    
}


internal void
pop_style(){
    if(ui->style_stack){
        ui->style_stack = ui->style_stack->prev;
    }
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
get_last_widget(UI_ID id, String8 string){
    auto hash = id & (MAX_TABLE_WIDGETS-1);
    auto last_table = ui->last_widget_table;
    
    if(last_table){
        auto widget = last_table[hash];
        if(widget){
            do {
                if(string_eq(string, widget->string)) return widget;
                widget = widget->next_hash;
            }while(widget);
        }
    }
    return nullptr;
}

internal Widget*
get_widget(String8 string){
    
    if(string.length == 0) return nullptr;
    
    auto id = generate_id(string);
    auto hash = id & (MAX_TABLE_WIDGETS-1);
    Widget* widget = ui->widget_table[hash];
    
    if(!widget){
        widget = push_type_zero(&platform->frame_arena, Widget);
        number_of_widgets++;
        ui->widget_table[hash] = widget;
    }else{
        
        do {
            if(string_eq(string, widget->string)){
                break;
            }
            if(!widget->next_hash){
                auto next_widget = push_type_zero(&platform->frame_arena, Widget);
                widget->next_hash = next_widget;
                widget = next_widget;
                break;
            }
            
            widget = widget->next_hash;
        }while(widget);
    }
    
    if(ui->last_widget_table){
        auto last_widget = ui->last_widget_table[hash];
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

internal inline Widget*
get_current_window() {
    return ui->windows[ui->current_window];
}

internal b32
is_there_an_overlapping_window_after(){
    if(ui->previous_window_count){
        
        for(int i = 0; i < ui->previous_window_count; i++){
            if(i == ui->current_window) continue;
            auto window = ui->previous_windows[i];
            auto bounds = v4f2(window->pos, window->min);
            bounds.y -= bounds.height;
            if(widget_has_property(window, WP_ALWAYS_ON_TOP) &&
               is_in_rect(platform->mouse_position, bounds)){
                return true;
            }
        }
    }
    return false;
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

internal b32
widget_properties_are_equal(Widget* a, Widget* b){
    if(!a || !b) return false;
    for(int i = 0; i < PROPERTIES_MAX/64 +1; i++){
        if(a->properties[i] != b->properties[i]){
            return false;
        }
    }
    return true;
}

internal Layout*
push_layout(Widget* widget){
    auto layout = push_type_zero(&platform->frame_arena, Layout);
    if(!ui->layout_stack){
        ui->layout_stack = layout;
    }else {
        if(widget_properties_are_equal(widget, ui->layout_stack->widget)){
            return ui->layout_stack;
        }
        layout->prev = ui->layout_stack;
        ui->layout_stack = layout;
    }
    layout->widget = widget;
    
    return layout;
}

internal Widget*
push_widget(){
    auto widget = make_widget();
    ui->current_widget = widget;
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
    ui->current_widget = widget;
    
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

internal void
ui_edit_text(Widget* widget){
    clampi(&ui->cursor_pos, 0, widget->text_edit_string->length);
    auto string = widget->text_edit_string;
    
    if(has_pressed_key_modified(KEY_LEFT, KEY_MOD_CTRL)){
        if(string->text[ui->cursor_pos-1] == ' '){
            while(string->text[ui->cursor_pos-1] == ' '){
                ui->cursor_pos--;
            }
        }else {
            while(string->text[ui->cursor_pos-1] != ' ' &&
                  ui->cursor_pos >= 0){
                ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
            }
        }
    }
    
    if(has_pressed_key_modified(KEY_RIGHT, KEY_MOD_CTRL)){
        if(string->text[ui->cursor_pos] == ' '){
            while(string->text[ui->cursor_pos] == ' '){
                ui->cursor_pos++;
            }
        }else{
            while(string->text[ui->cursor_pos] != ' ' &&
                  ui->cursor_pos <= string->length){
                ui->cursor_pos++;
            }
        }
    }
    
    if(has_pressed_key(KEY_LEFT)){
        ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
    }
    
    if(has_pressed_key(KEY_RIGHT)){
        ui->cursor_pos = ui->cursor_pos < string->length ? ui->cursor_pos +1: string->length;
    }
    
    if(has_pressed_key_modified(KEY_BACKSPACE, KEY_MOD_CTRL)){
        if(string->text[ui->cursor_pos-1] == ' '){
            pop_from_string(string, ui->cursor_pos);
            ui->cursor_pos--;
        }else {
            while(string->text[ui->cursor_pos-1] != ' ' &&
                  ui->cursor_pos >= 0){
                pop_from_string(string, ui->cursor_pos);
                ui->cursor_pos = ui->cursor_pos >= 0 ? ui->cursor_pos -1: 0;
            }
        }
    }
    
    if(has_pressed_key(KEY_BACKSPACE)){
        if(ui->cursor_pos){
            pop_from_string(string, ui->cursor_pos);
            ui->cursor_pos--;
        }
    }
    if(has_pressed_key(KEY_END)){
        ui->cursor_pos = string->length;
    }
    
    if(has_pressed_key(KEY_HOME)){
        ui->cursor_pos = 0;
    }
    
    Platform_Event* event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_CHARACTER_INPUT){
            insert_in_string(string,
                             event->character,
                             ui->cursor_pos++);
        }
    }
    
    
}

internal Widget_Update
update_widget(Widget* widget){
    Widget_Update result = {};
    UI_ID last_active = ui->active;
    
    Widget* last_widget = get_widget(widget->string);
    if(is_there_an_overlapping_window_after()){
        
    }else if(last_widget){
        
        v4f bbox = v4f2(last_widget->pos, last_widget->min);
        bbox.y -= bbox.height; //we draw widgets from top left not bottom left
        
        if(!ui->dragging && is_in_rect(platform->mouse_position, bbox)){
            ui->hot = widget->id;
        }
        if(ui->active == widget->id){
            if(has_left_released()){
                if(ui->hot == widget->id && widget_has_property(widget, WP_CLICKABLE)) {
                    result.clicked = true;
                    result.clicked_position = platform->mouse_position;
                    result.pos = bbox.pos;
                    result.size = bbox.size;
                }
                if(ui->dragging){
                    ui->dragging = false;
                }
                if(!widget_has_property(widget, WP_TEXT_EDIT)){
                    ui->active = -1;
                }
            }else if(has_middle_released()){
                if(ui->hot == widget->id) {
                    result.clicked_position = platform->mouse_position;
                    result.pos = bbox.pos;
                    result.size = bbox.size;
                    ui->dragging = false;
                }
                if(ui->dragging){
                    ui->dragging = false;
                }
                ui->active = -1;
            }
        }else if(ui->hot == widget->id){
            if(widget_has_property(widget, WP_DRAGGABLE) && has_left_dragged()){
                ui->active = widget->id;
                ui->dragging = true;
            }else if(widget_has_property(widget, WP_DRAGGABLE) && has_middle_dragged()){
                ui->active = widget->id;
                ui->dragging = true;
            }else if(widget_has_property(widget, WP_CLICKABLE) && has_left_clicked()){
                ui->active = widget->id;
            }
        }
        result.pos = bbox.pos;
        result.size = bbox.size;
        
        result.was_active = (last_active == widget->id) && (last_active != ui->active);
        
    }
    
    widget->style = ui->style_stack->style;
    
    if(ui->active == widget->id && 
       ui->text_edit == widget->id && 
       widget_has_property(widget, WP_TEXT_EDIT)){
        ui_edit_text(widget);
    }
    
    if(widget_has_property(widget, WP_RENDER_HOOK)){
        widget->min = get_text_bbox({}, widget->string).size;
    }
    
    if(widget_has_property(widget, WP_RENDER_TEXT)){
        widget->min = get_text_bbox({}, widget->string, widget->style.font_scale).size;
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
    update_widget(widget);
}

internal void
push_widget_column(){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_COLUMN);
    widget_set_property(widget, WP_SPACING);
    update_widget(widget);
}

internal void
push_widget_ghost(){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    widget->min = widget->parent->min;
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_GHOST_LAYOUT);
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
    
    auto widget = push_widget(string);
    
    auto layout = push_layout(widget);
    
    // NOTE(Oliver): i must be able to just call
    // push_widget(string)???????????
    
    ui->current_window = ui->window_count;
    ui->windows[ui->window_count++] = widget;
    
    push_default_style();
    
    widget_set_property(widget, WP_OVERLAP);
    widget_set_property(widget, WP_CLIP);
    widget_set_property(widget, WP_RENDER_BACKGROUND);
    //widget_set_property(widget, WP_RENDER_BORDER);
    
    auto result = update_widget(widget);
    
    widget->min = rect.size;
    widget->pos = rect.pos;
    
    push_widget_padding(v2f(10, 10));
}

internal void
push_widget_popup(v4f rect, String8 string){
    
    auto widget = make_widget(string);
    
    if(!ui->layout_stack && !ui->root){
        ui->root = widget;
    }else {
        
        Widget* last = ui->root;
        for(; last->next_sibling; last = last->next_sibling);
        last->next_sibling = widget;
        widget->prev_sibling = last;
        
    }
    
    auto layout = push_layout(widget);
    
    ui->current_window = ui->window_count;
    ui->windows[ui->window_count++] = widget;
    
    Widget_Style style = {};
    style.text_colour = v4f_from_colour(ui->theme.text);
    style.border_colour = v4f_from_colour(ui->theme.text);
    
    style.background_colour = v4f_from_colour(ui->theme.darker_background);
    style.background_colour.r /= 3.0;
    style.background_colour.g /= 3.0;
    style.background_colour.b /= 3.0;
    
    style.font_scale = 0.7f;
    style.rounded_corner_amount = 5.0f;
    push_style(style);
    
    widget_set_property(widget, WP_OVERLAP);
    widget_set_property(widget, WP_RENDER_BACKGROUND);
    widget_set_property(widget, WP_FIT_TO_CHILDREN);
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_CLIP);
    widget_set_property(widget, WP_ALWAYS_ON_TOP);
    widget_set_property(widget, WP_KEEP_INSIDE_WINDOW);
    
    auto render_hook = [](Widget* widget){
        //push_triangle(widget->pos + v2f(widget->min.width-50,0),150, 0, colour_from_v4f(widget->style.background_colour));
    };
    widget->render_hook = render_hook;
    auto result = update_widget(widget);
    widget->min = rect.size;
    widget->pos = rect.pos;
    push_widget_padding(v2f(10, 10));
}

internal void
pop_widget_window(){
    pop_layout();
    pop_layout();
    pop_layout();
    
    pop_style();
    ui->current_window--;
}

internal void
push_widget_container(String8 string){
    auto widget = push_widget(string);
    auto layout = push_layout(widget);
    
    //widget_set_property(widget, WP_DRAGGABLE);
    widget_set_property(widget, WP_FIT_TO_CHILDREN);
    widget_set_property(widget, WP_OVERLAP);
    widget_set_property(widget, WP_MANUAL_LAYOUT);
    
    auto result = update_widget(widget);
    widget->min = v2f(400, 200);
    push_widget_padding(v2f(10, 10));
    
    if(0 && ui->dragging && ui->active == widget->id){
        widget->pos.x = widget->pos.x + platform->mouse_delta.x;
        widget->pos.y = widget->pos.y + platform->mouse_delta.y;
    }
    
}

internal void
pop_widget_container(){
    pop_layout();
    pop_layout();
    pop_layout();
}

internal v2f layout_widgets(Widget* widget, v2f pos =  v2f(0,0), b32 dont_lerp_children = false);

#define PADDING 5
#define V2PADDING v2f(PADDING, PADDING)

internal v2f
layout_row(Widget* widget, v2f pos, b32 dont_lerp_children){
    v2f size = {};
    v2f start_pos = pos;
    v2f next_size = {};
    v2f temp_pos = pos;
    ForEachWidgetSibling(widget){
        
        v2f next_size = layout_widgets(it, temp_pos, dont_lerp_children);
        
        if(widget_has_property(it, WP_SPACING)){
            temp_pos.x += PADDING;
            size.width += PADDING;
        }
        temp_pos.x += next_size.width;
        
        size.width += next_size.width;
        size.height = max(size.height, next_size.height);
        
    }
    
    ForEachWidgetSibling(widget){
        
        v2f next_size = layout_widgets(it, pos, dont_lerp_children);
        layout_widgets(it, pos + v2f(0, next_size.height/2.0 - size.height/2.0), dont_lerp_children);
        if(widget_has_property(it, WP_SPACING)){
            pos.x += PADDING;
        }
        
        pos.x += next_size.width;
    }
    
    return size;
}

internal v2f
layout_wrap(Widget* widget, v2f pos, b32 dont_lerp_children){
    
    v2f size = {};
    v2f start_pos = pos;
    v2f next_size = {};
    f32 width = widget->parent->min.width;
    f32 end = start_pos.x + width;
    ForEachWidgetSibling(widget){
        if(widget_has_property(it, WP_WIDTHFILL)){
            it->min.x = it->parent->min.x - (pos.x - start_pos.x);
        }
        
        v2f next_size = layout_widgets(it, pos, dont_lerp_children);
        if(pos.x + next_size.width >= end){
            if(widget_has_property(it, WP_SPACING)){
                pos.y -= PADDING;
                size.height += PADDING;
            }
            pos.y -= next_size.height;
            pos.x = start_pos.x;
            next_size = layout_widgets(it, pos, dont_lerp_children);
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
layout_column(Widget* widget, v2f pos, b32 dont_lerp_children){
    v2f size = {};
    
    ForEachWidgetSibling(widget){
        v2f next_size = layout_widgets(it, pos, dont_lerp_children);
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
layout_widthfill(Widget* widget, v2f pos, b32 dont_lerp_children){
    
    v2f size = {};
    f32 min_space_required = 0;
    s32 number_of_children = 0;
    // NOTE(Oliver): layout as if it's a normal row
    {
        v2f temp_pos = pos;
        v2f start_pos = pos;
        v2f next_size = {};
        ForEachWidgetSibling(widget){
            if(widget_has_property(it, WP_WIDTHFILL)){
                it->min.x = it->parent->min.x - (temp_pos.x - start_pos.x);
            }
            v2f next_size = layout_widgets(it, temp_pos, dont_lerp_children);
            
            if(widget_has_property(it, WP_SPACING)){
                temp_pos.x += PADDING;
                size.width += PADDING;
            }
            temp_pos.x += next_size.width;
            
            size.width += next_size.width;
            size.height = max(size.height, next_size.height);
            if(widget_has_property(it, WP_FIXED_SIZE)){
                min_space_required += next_size.width;
                min_space_required += PADDING;
            }else {
                number_of_children++;
            }
        }
        
    }
    f32 available_space = widget->parent->min.width;
    f32 space_left = available_space - min_space_required;
    f32 space_per_widget = space_left/number_of_children;
    
    ForEachWidgetSibling(widget){
        if(widget_has_property(it, WP_FIXED_SIZE)){
        }else {
            it->min.width = space_per_widget;
        }
    }
    
    size = {};
    {
        v2f start_pos = pos;
        v2f next_size = {};
        ForEachWidgetSibling(widget){
            if(widget_has_property(it, WP_WIDTHFILL)){
                it->min.x = it->parent->min.x - (pos.x - start_pos.x);
            }
            v2f next_size = layout_widgets(it, pos, dont_lerp_children);
            
            if(widget_has_property(it, WP_SPACING)){
                pos.x += PADDING;
                size.width += PADDING;
            }
            pos.x += next_size.width;
            
            size.width += next_size.width;
            size.height = max(size.height, next_size.height);
            number_of_children++;
        }
    }
    return size;
}

internal v2f
_layout_widthfill(Widget* widget, v2f pos, b32 dont_lerp_children){
    
    v2f size = {};
    
    f32 total_width = 0;
    int number_of_children = 0;
    v2f start_pos = pos;
    ForEachWidgetSibling(widget) {
        //v2f next_size = layout_widgets(it, pos);
        if(widget_has_property(it, WP_SPACING)){
            total_width += PADDING;
        }
        
        
        if(!widget_has_property(it, WP_FIXED_SIZE)){
            number_of_children++;
        }
        
    }
    
    f32 available_space =  widget->parent->min.width;
    f32 width = 0;
    if(number_of_children == 1){
        width = (available_space - total_width + PADDING)/(f32)number_of_children - PADDING;
    }else {
        width = (available_space - total_width + PADDING)/(f32)number_of_children;
    }
    
    
    width = width >= 0 ? width : 0;
    
    ForEachWidgetSibling(widget) {
        //log("first: %f", it->min.width);
        if(!widget_has_property(it, WP_FIXED_SIZE)){
            it->min.width += width;
        }
        v2f next_size = layout_widgets(it, pos, dont_lerp_children);
        
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

internal void
layout_container(Widget* widget,  v2f pos, b32 dont_lerp_children){
    
    if(widget->pos.x == 0 && widget->pos.y == 0){
        widget->pos = pos;
    }
    
    pos = widget->pos;
    
    v2f size = layout_widgets(widget->first_child, pos);
    
}

internal void
layout_window(Widget* widget,  v2f pos){
    
}

internal v2f
layout_widgets(Widget* widget, v2f pos, b32 dont_lerp_children){
    
    if(!widget) return {};
    
    if(widget_has_property(widget, WP_MANUAL_LAYOUT)){
        if(widget->pos.x == 0 && widget->pos.y == 0){
            widget->pos = pos;
        }
        
    }
    
    v2f size = {};
    if(widget_has_property(widget, WP_OVERLAP)){
        
        pos = widget->pos;
        size = layout_widgets(widget->first_child, pos);
        
        //layout_window(widget, pos);
    }
    if(widget_has_property(widget, WP_FIT_TO_CHILDREN)){
        widget->min = size;
        widget->min.x -= PADDING;
    }
    
    if(widget_has_property(widget, WP_PADDING)){
        v2f size = widget->parent->min;
        v2f padding = widget->min;
        pos.x += (size.width - padding.width)/2.0f;
        pos.y -= (size.height - padding.height)/2.0f;
        widget->pos = pos;
        widget->min = layout_widgets(widget->first_child, pos, dont_lerp_children);
        widget->min.width += (size.width - padding.width); 
        widget->min.height += (size.height - padding.height); 
        
    }
    
    if(widget->pos.x && widget->pos.y && widget_has_property(widget, WP_LERP_POSITION)){
        animate(&widget->pos.x, pos.x, 0.2f);
        animate(&widget->pos.y, pos.y, 0.2f);
    }else if(!widget_has_property(widget, WP_CONTAINER)){
        widget->pos = pos;
    }
    if(dont_lerp_children)  widget->pos = pos;
    
    
    if(widget_has_property(widget, WP_GHOST_LAYOUT)){
        layout_widgets(widget->first_child, pos, dont_lerp_children);
        return {};
    }
    
    if(widget_has_property(widget, WP_COLUMN)){
        return layout_column(widget->first_child, pos, dont_lerp_children);
    }
    
    if(widget_has_property(widget, WP_WRAP)){
        return layout_wrap(widget->first_child, pos, dont_lerp_children);
    }
    
    if(widget_has_property(widget, WP_ROW)){
        return layout_row(widget->first_child, pos, dont_lerp_children);
    }
    
    if(widget_has_property(widget, WP_WIDTHFILL)){
        return layout_widthfill(widget->first_child, pos, dont_lerp_children);
    }
    
    return widget->min;
}

internal void
widget_render_text(v2f pos, Widget* widget, Colour colour){
    pos.y -= widget->min.height;
    v4f text_bbox = get_text_bbox(pos, widget->string, widget->style.font_scale);
    v4f bbox = v4f2(pos, widget->min);
    
    f32 centre = pos.x + widget->min.x/2.0f;
    f32 text_centre = get_text_width(widget->string, widget->style.font_scale)/2.0f;
    f32 text_x = centre - text_centre;
    
    v4f shadow_colour = v4f_from_colour(colour);
    shadow_colour.r /= 8;
    shadow_colour.g /= 8;
    shadow_colour.b /= 8;
    shadow_colour.a = 1;
    
    v2f text_size = get_text_size(widget->string, widget->style.font_scale);
    push_string(v2f(text_x+1.5, text_bbox.y-1.5), widget->string, colour_from_v4f(shadow_colour), widget->style.font_scale);
    push_string(v2f(text_x, bbox.y), widget->string, colour, widget->style.font_scale);
    //push_rectangle_outline(v4f(text_x, bbox.y, text_size.width, text_size.height), 0.2, 3, ui->theme.text);
}

internal void
widget_render_text(Widget* widget, Colour colour){
    widget_render_text(widget->pos, widget, colour);
}

internal void
render_widgets(Widget* widget){
    if(!widget) return;
    v2f pos = widget->pos; 
    
    pos.y -= widget->min.height;
    
    { 
        
        if(widget_has_property(widget, WP_PADDING)){
            v4f bbox = v4f2(pos, widget->min);
        }
        
        
        if(ui->hot == widget->id){
            animate(&widget->hot_transition, 1.0f, 0.1f);
        }else {
            animate(&widget->hot_transition, 0, 0.1f);
        }
        
        if(ui->active == widget->id || widget->checked){
            animate(&widget->active_transition, 1.0f, 0.1f);
        }else if(!widget->checked) {
            animate(&widget->active_transition, 0, 0.1f);
        }
        
        
        if((widget_has_property(widget, WP_HOVER_RENDER_BACKGROUND) &&
            ui->hot == widget->id) || 
           widget_has_property(widget, WP_RENDER_BACKGROUND)){
            
            
            v4f bbox = v4f2(pos, widget->min);
            if(widget_has_property(widget, WP_HOVER_INFLATE)){
                bbox = inflate_rect(bbox, widget->hot_transition*2.0);
            }
            if(widget_has_property(widget, WP_RENDER_SHADOW)){
                v4f shadow_rect = bbox;
                shadow_rect.pos += v2f(3, -3);
                Colour shadow = colour_from_v4f(widget->style.background_colour);
                shadow.r /= 3.0f;
                shadow.g /= 3.0f;
                shadow.b /= 3.0f;
                push_rectangle(shadow_rect, widget->style.rounded_corner_amount, shadow);
            }
            //push_rectangle(bbox, 1, colour_from_v4f(widget->style.background_colour));
            
            push_rectangle(bbox, widget->style.rounded_corner_amount, 
                           colour_from_v4f(widget->style.background_colour));
            
        }
        
        if(widget_has_property(widget, WP_RENDER_TEXT)){
            widget_render_text(widget,(ui->theme.text));
        }
        
        
        if(widget_has_property(widget, WP_RENDER_BORDER) ||
           (widget_has_property(widget, WP_HOVER_RENDER_BORDER) && 
            ui->hot == widget->id)){
            
            v4f bbox = v4f2(pos, widget->min);
            if(widget_has_property(widget, WP_HOVER_INFLATE)){
                bbox = inflate_rect(bbox, widget->hot_transition*2.0);
            }
            v4f border_colour = {};
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
            
            push_rectangle_outline(bbox, 0.2, widget->style.rounded_corner_amount, 
                                   colour_from_v4f(border_colour));
        }
        
        if(widget_has_property(widget, WP_RENDER_HOOK)){
            widget->render_hook(widget);
        }
        
    }
    
    if(widget_has_property(widget, WP_CLIP)){
        v4f bbox = v4f2(pos, widget->min);
        RENDER_CLIP(bbox){
            ForEachWidgetChild(widget){
                render_widgets(it);
            }
        }
    }else {
        ForEachWidgetChild(widget){
            render_widgets(it);
        }
    }
    
}

internal void 
split_panel(Panel* panel, f32 split_ratio, Panel_Split_Type split_type, Panel_Type type, b32 draggable = true){
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
    
    panel->first->draggable = draggable; 
    panel->second->draggable = draggable; 
    
}

static int present_style = 0;
internal void present_arc(Arc_Node* node);
static f32 font_scale = 0.85f;
static f32 next_font_scale = font_scale;

internal void
render_panels(Panel* root, v4f rect){
    
    if(!root) return;
    
    if(root->is_dragging){
        v2f delta = {};
        if(has_left_released()){
            root->is_dragging = false;
        }
        // NOTE(Oliver): credit to Max Jordan for giving me the algorithm
        // to resize panels
        switch(root->first->split_type){
            case PANEL_SPLIT_HORIZONTAL:{
                root->first->split_ratio = 1.0 -platform->mouse_position.y/platform->window_size.height;
                root->second->split_ratio =platform->mouse_position.y/platform->window_size.height;
            }break;
            case PANEL_SPLIT_VERTICAL:{
                root->first->split_ratio = platform->mouse_position.x/platform->window_size.width;
                root->second->split_ratio = 1.0 - platform->mouse_position.x/platform->window_size.width;
            }break;
        }
    }
    
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
        if(root->no_pad){
        }else {
            rect.x += PADDING;
            rect.y -= PADDING;
            rect.width -= PADDING*2;
            rect.height -= PADDING*2;
        }
        auto harea = v4f(rect.x-PADDING*2, rect.y-rect.height, PADDING*4, rect.height);
        auto varea = v4f(rect.x, rect.y-rect.height-PADDING*2, rect.width, PADDING*4);
        
        
        v2f delta = {};
        if(root->draggable &&
           !root->parent->is_dragging && is_in_rect(platform->mouse_position, harea)){
            if(!ui->dragging && has_left_dragged()){
                root->parent->is_dragging = true;
            }
            platform->set_cursor_to_horizontal_resize();
        } else if(root->draggable &&
                  !root->parent->is_dragging && is_in_rect(platform->mouse_position, varea)){
            if(!ui->dragging && has_left_dragged()){
                root->parent->is_dragging = true;
            }
        }
        
        if((root->parent->is_dragging && root->split_type == PANEL_SPLIT_VERTICAL) 
           || is_in_rect(platform->mouse_position, harea)){
            platform->set_cursor_to_horizontal_resize();
        }
        if((root->parent->is_dragging && root->split_type == PANEL_SPLIT_HORIZONTAL) 
           || is_in_rect(platform->mouse_position, varea)){
            platform->set_cursor_to_vertical_resize();
        }
        
        if(root->type == PANEL_PROPERTIES){
            UI_WINDOW(rect, "Properties#%d", (int)root) {
                
                ID("%d", (int)root){
                    ui_panel_header(root, "Properties");
                    yspacer(10);
                    b32 result = false;
                    
                    UI_COLUMN ID("%d", (int)root) {
                        UI_WIDTHFILL {
                            result = arrow_dropdown2("Syntax Style");
                        }
                        
                        if(result){
                            
                            yspacer(10);
                            UI_WIDTHFILL { if(button("Render as default")) present_style = 0;}
                            UI_WIDTHFILL { if(button("Render as C")) present_style = 1;}
                            UI_WIDTHFILL { if(button("Render as Pascal")) present_style = 2;}
                            UI_WIDTHFILL { if(button("Render as Python")) present_style = 3;}
                            local_persist v4f rect  = {};
                            yspacer(10);
                        }else {
                            yspacer(1);
                        }
                        
                        UI_WIDTHFILL {
                            result = arrow_dropdown2("Size and spacing");
                        }
                        if(result){
                            yspacer(10);
                            UI_WIDTHFILL{
                                label("Font"); 
                                fslider(0, 2, &next_font_scale, "font scale");
                            }
                            
                            UI_WIDTHFILL{
                                label("Indent"); 
                                fslider(0, 50, &presenter->indent_level, "indent");
                            }
                            yspacer(10);
                        }else {
                            yspacer(1);
                            
                        }
                        
                        UI_WIDTHFILL {
                            result = arrow_dropdown2("Code generation");
                        }
                        if(result){
                            yspacer(10);
                            
                            UI_ROW UI_WIDTHFILL {
                                if(button("Compile")){
                                    compile_c(editor->root);
                                }
                                button("Run");
                            }
                            yspacer(1);
                            UI_ROW UI_WIDTHFILL {
                                label("File");
                                widget_set_property(ui->current_widget, WP_FIXED_SIZE);
                                text_box(&editor->file_location);
                                
                            }
                            yspacer(1);
                            
                            UI_ROW UI_WIDTHFILL {
                                if(button("Serialise")){
                                    platform->delete_file(editor->file_location);
                                    serialise(editor->file_location, editor->root);
                                }
                                if(button("Deserialise")){
                                    editor->should_reload = true;
                                }
                                
                            }
                            yspacer(10);
                            
                        }else {
                            yspacer(1);
                        }
                        
                        UI_WIDTHFILL {
                            result = arrow_dropdown2("Frame graph");
                        }
                        if(result){
                            yspacer(20);
                            label("frame graph"); 
                            
                            UI_ROW UI_WIDTHFILL{
                                frame_graph();
                            }
                            
                            UI_WIDTHFILL {
                                local_persist String8 string = make_string("test");
                                text_box(&string);
                            }
                        }else {
                            yspacer(1);
                        }
                        
                    }
                }
                
            }
            
        }else if(root->type == PANEL_EDITOR) {
            ID("%d", (int)root) {
                UI_WINDOW(rect, "Editor") {
                    
                    ui_panel_header(root, "Code Editor");
                    switch(present_style){
                        case 0: {
                            label("Default style");
                        }break;
                        case 1: {
                            label("C style");
                            
                        }break;
                        case 2: {
                            label("Pascal style");
                            
                        }break;
                        case 3: {
                            label("Python style");
                        }break;
                    }
#if 0
                    UI_ROW {
                        label("view");
                        for(int i = 0; i < editor->view_count; i++){
                            button("%.*s", editor->views[i].length, editor->views[i].text);
                            xspacer(10);
                        }
                        xspacer(30);
                        if(button("+")){
                            editor->views[editor->view_count] = make_stringf(&editor->string_pool, "view %d", editor->view_count);
                            editor->view_count++;
                        }
                    }
#endif
                    
                    UI_CONTAINER("snippet"){
                        present_arc(editor->root);
                    }
                    
                }
            }
        }else if(root->type == PANEL_STATUS){
            UI_WINDOW(rect, "Status") {
                UI_ROW {
                    label("delta time:"); 
                    label("%.1f", platform->dt);
                }
            }
        }else if(root->type == PANEL_DEBUG){
            UI_WINDOW(rect, "Debug"){
                ID("%d", (int)root) {
                    ui_panel_header(root, "Debug");
                    yspacer();
                    b32 result = 0;
                    UI_WIDTHFILL {
                        result = arrow_dropdown2("navigation");
                    }
                    
                    if(result){
                        UI_COLUMN {
                            label("current line: %d", presenter->cursor.line_index);
                            label("current pos: %d", presenter->cursor.buffer_index);
                            label("max line: %d", presenter->line_pos);
                            label("max pos: %d", presenter->buffer_pos);
                            for(int j = 0; j < presenter->line_pos; j++){
                                UI_ROW{
                                    for(int i = presenter->lines[j].start; i < presenter->lines[j].end; i++){
                                        ID("buffer%d", (int)presenter->buffer[i].node){
                                            if(presenter->buffer[i].node->string.length){
                                                label("%.*s", presenter->buffer[i].node->string.length,
                                                      presenter->buffer[i].node->string.text);
                                            }else {
                                                //label("empty %d", presenter->buffer[i].node->ast_type);
                                            }
                                        }
                                    }
                                    label("| line: %d %d", presenter->lines[j].start, presenter->lines[j].end);
                                }
                            }
                            
                        }
                    }
                    
                }
            }
        }else if(root->type == PANEL_CONSOLE){
            UI_WINDOW(rect, "Console"){
                ID("%d", (int)root) {
                    ui_panel_header(root, "Console");
                    label("%f", time_per_gui_update);
                }
            }
        }else if(root->type == PANEL_HEADER) {
            UI_WINDOW(rect, "Header#%d", (int)root){
                widget_remove_property(get_current_window(), WP_RENDER_BORDER);
                widget_remove_property(get_current_window(), WP_RENDER_BACKGROUND);
                ID("%d", (int)root) {
                    b32 result = false;
                    UI_ROW {
                        icon_button(ui->logo, "logo button");
                        xspacer();
                        result = arrow_dropdown3("File");
                        if(result){
                            v4f rect = get_left_dropdown_rect_from_current_widget();
                            UI_POPUP(rect, "file popup"){
                                
                                UI_ROW UI_WIDTHFILL{
                                    if(button("New")){
                                    }
                                }
                                
                                UI_ROW UI_WIDTHFILL{
                                    if(button("Open")){
                                    }
                                }
                                yspacer(5);
                                UI_ROW UI_WIDTHFILL{
                                    if(button("Save")){
                                    }
                                }
                                
                                UI_ROW UI_WIDTHFILL{
                                    if(button("Save As")){
                                    }
                                }
                                yspacer(5);
                                UI_ROW UI_WIDTHFILL{
                                    if(button("Import")){
                                    }
                                }
                                
                                UI_ROW UI_WIDTHFILL{
                                    if(button("Import as")){
                                    }
                                }
                                
                            }
                        }
                        xspacer();
                        result = arrow_dropdown3("Edit");
                        xspacer();
                        result = arrow_dropdown3("Compile");
                        xspacer();
                        result = arrow_dropdown3("Window");
                        xspacer();
                        result = arrow_dropdown3("Help");
                    }
                }
            }
        }
        
    }
    
}

internal v4f
get_right_dropdown_rect_from_current_widget() {
    
    Widget* widget = ui->current_widget;
    v4f rect = {};
    rect = v4f2(widget->pos, v2f(200,125));
    rect.pos.x -= (rect.width - widget->min.width);
    rect.pos.y -= widget->min.height;
    rect.pos.y -= PADDING;
    return rect;
}

internal v4f
get_left_dropdown_rect_from_current_widget() {
    
    Widget* widget = ui->current_widget;
    v4f rect = {};
    rect = v4f2(widget->pos, v2f(200,125));
    rect.pos.y -= widget->min.height;
    rect.pos.y -= PADDING;
    return rect;
}

internal void
panel_switch_popup(Panel* panel){
    auto rect = get_right_dropdown_rect_from_current_widget();
    UI_POPUP(rect, "popup"){
        ID("%d", (int)ui->popup_panel) {
            UI_ROW UI_WIDTHFILL{
                if(button("Properties")){
                    panel->type = PANEL_PROPERTIES;
                }
            }
            UI_ROW UI_WIDTHFILL{
                if(button("Code Editor")){
                    panel->type = PANEL_EDITOR;
                }
            }
            UI_ROW UI_WIDTHFILL{
                if(button("Debug Info")){
                    panel->type = PANEL_DEBUG;
                    
                }
            }
            UI_ROW UI_WIDTHFILL{
                if(button("Console")){
                    panel->type = PANEL_CONSOLE;
                }
            }
        }
    }
}

internal void
append_window_to(Widget** to, Widget* window) {
    assert(to);
    if(*to){
        Widget* last = *to;
        for(; last->next_sibling; last = last->next_sibling);
        last->next_sibling = window;
        window->prev_sibling = last;
    }else {
        *to = window;
    }
}

internal int
compare_windows(const void* a, const void* b){
    Widget* window_a = *(Widget**)a;
    Widget* window_b = *(Widget**)b;
    int x = widget_has_property(window_a, WP_ALWAYS_ON_TOP);
    int y = widget_has_property(window_b, WP_ALWAYS_ON_TOP);
    return x-y;
}

internal int
compare_windows(Widget* a, Widget* b){
    int x = widget_has_property(a, WP_ALWAYS_ON_TOP);
    int y = widget_has_property(b, WP_ALWAYS_ON_TOP);
    return x-y;
}

internal void
sort_previous_windows() {
    
    for(int i = 0; i < ui->previous_window_count -1; i++) {
        int min = i;
        for (int j = i + 1; j < ui->previous_window_count; j++)
            if (compare_windows(ui->previous_windows[min], ui->previous_windows[j]) > 0){
            min = j;
        }
        
        // Move minimum element at current i.
        Widget* key = ui->previous_windows[min];
        while (min > i)
        {
            ui->previous_windows[min] = ui->previous_windows[min - 1];
            min--;
        }
        ui->previous_windows[i] = key;
    }
}

internal void
sort_windows() {
    
    for(int i = 0; i < ui->window_count -1; i++) {
        int min = i;
        for (int j = i + 1; j < ui->window_count; j++)
            if (compare_windows(ui->windows[min], ui->windows[j]) > 0){
            min = j;
        }
        
        // Move minimum element at current i.
        Widget* key = ui->windows[min];
        while (min > i)
        {
            ui->windows[min] = ui->windows[min - 1];
            min--;
        }
        ui->windows[i] = key;
    }
}
