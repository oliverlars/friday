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
        if (event->type == PLATFORM_EVENT_KEY_PRESS && event->key == key){
            *event_out = event;
            result = 1;
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
has_mouse_dragged(Platform_Event **event_out){
    b32 result = 0;
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        if (event->type == PLATFORM_EVENT_MOUSE_MOVE && event->mouse_button == MOUSE_BUTTON_LEFT){
            *event_out = event;
            result = 1;
        }
    }
    return(result);
}

internal b32
has_mouse_dragged(v2f* delta = 0) {
    Platform_Event* event = 0;
    b32 result = has_mouse_dragged(&event);
    if(result){
        platform_consume_event(event);
        if(delta){
            *delta = event->delta;
        }
    }
    return result;
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

#define HASH_INITIAL 2166136261

internal void
hash32(u64* hash, char* data, int size){
    
    while(size--){
        *hash = (*hash ^ *data++) * 16777619;
    }
}

internal UI_ID
generate_id(String8 label){
    int size = label.length;
    UI_ID lower = 0;
    hash32(&lower, label.text, size);
    auto layout = ui->layout_stack;
    UI_ID result = 0;
    result = (layout->widget->id << 32) ^ lower;
    
    return result;
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
    auto id = generate_id(string);
    auto hash = id & (MAX_WIDGETS-1);
    auto widget = ui->widget_table[hash];
    if(!widget){
        widget = push_type_zero(&platform->permanent_arena, Widget);
        ui->widget_table[hash] = widget;
        return widget;
    }
    
    do {
        if(id == widget->id){
            break;
        }
        if(!widget->next_hash){
            widget = push_type_zero(&platform->permanent_arena, Widget);
            widget->next_hash = widget;
            break;
        }
        
        widget = widget->next_hash;
    }while(widget);
    
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
    widget->string = string;
    widget->id = generate_id(string);
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

internal Widget_Update
update_widget(Widget* widget){
    Widget_Update result = {};
    
    Widget* last_widget = get_widget(widget->string);
    if(last_widget){
        v4f bbox = v4f2(last_widget->pos, last_widget->min);
        if(is_in_rect(platform->mouse_position, bbox) && has_left_clicked()){
            result.clicked = true;
        }
    }
    
    if(widget_has_property(widget, WP_CLICKABLE)){
        
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
push_widget_widthfill(){
    auto widget = push_widget();
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_WIDTHFILL);
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
    widget->string = string;
    auto layout = push_layout(widget);
    widget_set_property(widget, WP_WINDOW);
    widget_set_property(widget, WP_CLIP);
    widget_set_property(widget, WP_RENDER_BACKGROUND);
    
    widget->min = rect.size;
    widget->pos = rect.pos;
    update_widget(widget);
    push_widget_padding(v2f(10, 10));
    
}

internal void
pop_widget_window(){
    while(ui->layout_stack) pop_layout();
    
}

internal v2f layout_widgets(Widget* widget, v2f pos);

#define PADDING 10
#define V2PADDING v2f(PADDING, PADDING)

internal v2f
layout_row(Widget* widget, v2f pos){
    
    v2f size = {};
    v2f start_pos = pos;
    ForEachWidgetSibling(widget){
        if(widget_has_property(it, WP_WIDTHFILL)){
            it->min.x = it->parent->min.x - (pos.x - start_pos.x);
        }
        v2f next_pos = layout_widgets(it, pos);
        if(widget_has_property(it, WP_SPACING)){
            pos.x += PADDING;
        }
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
        if(widget_has_property(it, WP_SPACING)){
            pos.y -= PADDING;
            size.height += PADDING;
        }
        
        size.height += next_pos.height;
        
        size.width = max(size.width, next_pos.width);
    }
    size.height -= PADDING;
    
    return size;
}

internal v2f
layout_widthfill(Widget* widget, v2f pos){
    
    v2f size = {};
    
    f32 total_width = 0;
    int number_of_children = 0;
    
    ForEachWidgetSibling(widget) {
        //v2f next_pos = layout_widgets(it, pos);
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
        v2f next_pos = layout_widgets(it, pos);
        if(widget_has_property(widget, WP_SPACING)){
            pos.x += PADDING;
        }
        
        pos.x += next_pos.width;
        
        size.width += next_pos.width;
        size.height = max(size.height, next_pos.height);
    }
    
    return size;
}

internal v2f
layout_widgets(Widget* widget, v2f pos = v2f(0,0)){
    if(!widget) return {};
    
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
        layout_widgets(widget->first_child, pos);
        
    }
    widget->pos = pos;
    
    if(widget_has_property(widget, WP_COLUMN)){
        return layout_column(widget->first_child, pos);
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
    v4f bbox = get_text_bbox(widget->pos, widget->string);
    bbox = v4f2(widget->pos, widget->min);
    if(widget_has_property(widget, WP_RENDER_BACKGROUND)){
        push_rectangle(bbox, 1, ui->theme.sub_colour);
    }
    if(widget_has_property(widget, WP_RENDER_BORDER)){
        
        bbox = inflate_rect(bbox, widget->hot_transition*2.5f);
        push_rectangle_outline(bbox, 1, 3, ui->theme.border);
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
        push_rectangle_outline(bbox, 1, 3, ui->theme.text);
        
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
    widget->pos.y -= widget->min.height;
    
    if(ui->hot == widget->id || is_in_rect(platform->mouse_position, v4f2(widget->pos, widget->min))){
        lerp(&widget->hot_transition, 1.0f, 0.1f);
        ui->hot = widget->id; 
    }else {
        lerp(&widget->hot_transition, 0, 0.1f);
    }
    
    if(widget_has_property(widget, WP_RENDER_TEXT)){
        widget_render_text(widget, ui->theme.text);
    }
    if(widget_has_property(widget, WP_RENDER_TRIANGLE)){
        v4f bbox = v4f2(widget->pos, widget->min);
        bbox = inflate_rect(bbox, widget->hot_transition*2.5f);
        if(widget_has_property(widget, WP_RENDER_BORDER)){
            push_rectangle_outline(bbox, 1, 3, ui->theme.text);
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
    update_widget(widget);
}

internal b32
arrow_dropdown(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_TRIANGLE);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_FIXED_SIZE);
    widget_set_property(widget, WP_SPACING);
    auto result = update_widget(widget);
    return result.clicked;
}

internal b32
button(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    Widget* widget;
    widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_TEXT);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_SPACING);
    auto result = update_widget(widget);
    return result.clicked;
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
    widget->pos = v2f(0,0);
    widget_set_property(widget, WP_RENDER_TEXT);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_FIXED_SIZE);
    widget_set_property(widget, WP_SPACING);
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
fslider(f32 min, f32 max, f32* value){
    
    auto widget = push_widget();
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_CUSTOM_DATA);
    widget_set_property(widget, WP_RENDER_HOOK);
    
    
    auto render_hook = [](Widget* widget) {
        v4f bbox = get_text_bbox(widget->pos, widget->string);
        bbox = v4f2(widget->pos, widget->min);
        push_rectangle(bbox, 1, ui->theme.sub_colour);
        push_rectangle(bbox, 1, ui->theme.text_function);
        if(widget_has_property(widget, WP_RENDER_BORDER)){
            push_rectangle_outline(bbox, 1, 3, ui->theme.border);
            widget->pos.x += 1;
            widget->pos.y -= 1;
        }
        f32 centre = widget->pos.x + widget->min.x/2.0f;
        f32 text_centre = get_text_width(widget->string)/2.0f;
        f32 text_x = centre - text_centre;
        push_string(v2f(text_x, bbox.y), widget->string, ui->theme.text);
    };
    
    widget->data = value;
    widget->render_hook = render_hook;
    widget->string = make_stringf(&platform->frame_arena, "%f", *value);
    update_widget(widget);
}

internal void
ui_window(v4f rect, bool title_bar, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    push_widget_window(rect, string);
    
    push_widget_column();
    if(title_bar){
        UI_ROW {
            label("%.*s", string.length, string.text);
            UI_WIDTHFILL{
                xspacer(20);
                //button_fixed("V");
                arrow_dropdown("change type%.*s", string.length, string.text);
            }
        }
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

internal void
update_panel_split(Panel* parent, f32 split_ratio){
    parent->first->split_ratio = split_ratio;
    parent->second->split_ratio = 1.0f - split_ratio;
}

internal void present_keyword(char* fmt, ...);
internal void present_literal(char* fmt, ...);
internal void present_function(char* fmt, ...);
internal void present_id(char* fmt, ...);
internal void present_misc(char* fmt, ...);
internal void present_cursor();

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
            UI_WINDOW(rect, true, "Properties") {
                char* names[] = {"test", "dog", "piano"};
                UI_COLUMN {
                    
                    UI_WIDTHFILL button("Render as C");
                    UI_WIDTHFILL button("Render as Jai");
                    
                    yspacer(20);
                    UI_ROW UI_WIDTHFILL {
                        static b32 checked = false;
                        if(check(&checked, "Compile")){
                            button("test");
                        }
                        button("Run");
                    }
                }
                
            }
            
        }else if(root->type == PANEL_EDITOR) {
            UI_WINDOW(rect, true, "Code Editor") {
                
                UI_ROW {
                    xspacer(100);
                    present_function("entry point");
                    xspacer();
                    present_misc("::");
                    xspacer();
                    present_misc("(");
                    
                    present_id("arg count");
                    present_misc(":");
                    xspacer();
                    present_keyword("s32");
                    
                    present_misc(",");
                    present_cursor();
                    xspacer();
                    present_id("args");
                    present_misc(":");
                    xspacer();
                    present_keyword("s8*");
                    
                    present_misc(")");
                    xspacer();
                    present_misc("{");
                    
                }
                
                UI_ROW{
                    xspacer(100);
                    xspacer(40);
                    present_id("test variable");
                    
                    present_misc(":");
                    
                    xspacer();
                    
                    present_keyword("s32");
                    
                    xspacer();
                    present_misc("=");
                    xspacer();
                    
                    present_literal("1024");
                    
                    
                    xspacer();
                    
                    present_misc("+");
                    
                    xspacer();
                    
                    present_literal("1024");
                    
                    
                }
                UI_ROW{
                    xspacer(100);
                    present_misc("}");
                }
            }
            
        }else if(root->type == PANEL_STATUS){
            UI_WINDOW(rect, false, "Status") {
                UI_ROW {
                    label("mouse position:"); 
                    label("%.0f %.0f", platform->mouse_position.x, platform->mouse_position.y);
                }
            }
        }
        
    }
}