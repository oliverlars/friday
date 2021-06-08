
internal void
xspacer(f32 space){
    auto widget = push_widget();
    widget->min = v2f(space, 0);
    widget_set_property(widget, WP_SPACING);
    update_widget(widget);
}

internal void
yspacer(f32 space){
    
    auto widget = push_widget();
    widget->min = v2f(0, space);
    widget_set_property(widget, WP_SPACING);
    update_widget(widget);
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
                if(button_fixed("console")){
                    panel->type = PANEL_CONSOLE;
                }
            }
        }
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
    //widget_set_property(widget, WP_LERP_POSITION);
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
    //widget_set_property(widget, WP_LERP_POSITION);
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
    //widget_set_property(widget, WP_LERP_POSITION);
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
            push_rectangle_outline(bbox, 1, 3, colour_from_v4f(border_colour));
            
            pos.x += 1;
            pos.y -= 1;
        }
        f32 centre = pos.x + widget->min.x/2.0f;
        String8 string = make_stringf(&platform->frame_arena, "%.2f", widget->value);
        f32 text_centre = get_text_width(string, widget->style.font_scale)/2.0f;
        f32 text_x = centre - text_centre;
        push_string(v2f(text_x, bbox.y), string, ui->theme.text, widget->style.font_scale);
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

internal b32
dropdown(char* fmt, ...){
    b32 result = 0;
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    UI_ROW {
        label("%.*s", string.length, string.text);
        UI_WIDTHFILL{
            xspacer(20);
            result= arrow_dropdown("change type%.*s", string.length, string.text);
        }
        
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
            f32 text_centre = get_text_width(s, widget->style.font_scale)/2.0f;
            f32 text_x = centre - text_centre;
            push_string(v2f(text_x, bbox.y), s, ui->theme.text);
            
            v2f cursor = {};
            cursor.x = text_x + get_text_width_n(s, ui->cursor_pos, widget->style.font_scale);
            cursor.y = bbox.y;
            push_rectangle(v4f2(cursor, v2f(2, widget->min.height)), 1, ui->theme.cursor);
        }else {
            f32 centre = pos.x + widget->min.x/2.0f;
            f32 text_centre = get_text_width(widget->string, widget->style.font_scale)/2.0f;
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
