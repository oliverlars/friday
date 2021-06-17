
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
ui_popup(v4f rect, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    push_widget_popup(rect, string);
    
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

internal int
menu_list(String8* list, s32 count, char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    int clicked_which = -1;
    UI_COLUMN {
        for(int i = 0; i < count; i++){
            
            Widget* widget = push_widget(list[i]);
            
            widget_set_property(widget, WP_CLICKABLE);
            widget_set_property(widget, WP_RENDER_TEXT);
            widget_set_property(widget, WP_RENDER_BORDER);
            widget_set_property(widget, WP_RENDER_BACKGROUND);
            widget_set_property(widget, WP_SPACING);
            widget_set_property(widget, WP_LERP_COLOURS);
            widget_set_property(widget, WP_GHOST_LAYOUT);
            
            auto result = update_widget(widget);
            if(result.clicked) { clicked_which = i; break; }
            
        }
    }
    return clicked_which;
}

internal void
ui_panel_header(Panel* panel, char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    
    b32 dropdown = false;
    UI_ROW {
        UI_WIDTHFILL{
            xspacer();
            ID("change type"){
                
                auto widget = push_widget(string);
                widget_set_property(widget, WP_CLICKABLE);
                widget_set_property(widget, WP_FIXED_SIZE);
                widget_set_property(widget, WP_SPACING);
                widget_set_property(widget, WP_FIRST_TRANSITION);
                widget_set_property(widget, WP_RENDER_HOOK);
                widget_set_property(widget, WP_HOVER_INFLATE);
                widget_set_property(widget, WP_RENDER_BORDER);
                
                auto render_hook = [](Widget* widget) {
                    auto bbox = v4f2(widget->pos, widget->min);
                    f32 padded_size = widget->min.height;
                    v2f tpos = bbox.pos;
                    tpos.y -= padded_size/2.0;
                    tpos.x += padded_size/2.0;
                    push_triangle(tpos, padded_size, 0.25 + lerp(0.0, 0.25, widget->active_transition), ui->theme.text);
                    bbox.pos.x += padded_size/2.0;
                    widget_render_text(bbox.pos , widget, ui->theme.text);
                };
                
                widget->render_hook = render_hook;
                auto result = update_widget(widget);
                widget->min = get_text_size(widget->string, widget->style.font_scale);
                widget->min.width += 2.0*widget->min.height;
                
                if(result.clicked){
                    widget->checked = !widget->checked;
                    ui->popup = widget->checked;
                }
                
                if(widget->checked){
                    ui->popup_rect = v4f2(widget->pos, v2f(200,125));
                    ui->popup_rect.pos.x -= (ui->popup_rect.width - widget->min.width);
                    ui->popup_rect.pos.y -= widget->min.height;
                    ui->popup_rect.pos.y -= 20;
                    ui->popup_rect.pos.y -= PADDING;
                    ui->popup_panel = panel;
                }
            }
        }
    }
}

internal void
filebar_dropdown(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    b32 dropdown = false;
    
    dropdown = arrow_dropdown3("%.*s", string.length, string.text);
    auto widget = ui->current_widget;
    if(dropdown){
        widget->checked = !widget->checked;
        ui->popup = widget->checked;
    }
    
    if(widget->checked){
        ui->popup_rect = v4f2(widget->pos, v2f(200,125));
        ui->popup_rect.pos.x -= (ui->popup_rect.width - widget->min.width);
        ui->popup_rect.pos.y -= widget->min.height;
        ui->popup_rect.pos.y -= 20;
        ui->popup_rect.pos.y -= PADDING;
    }
    
}

internal void
label(char* fmt, ...){
    
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    push_default_style();
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
    widget_set_property(widget, WP_FIXED_SIZE);
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_HOVER_INFLATE);
    widget_set_property(widget, WP_RENDER_BORDER);
    
    push_default_style();
    
    auto render_hook = [](Widget* widget) {
        auto bbox = v4f2(widget->pos, widget->min);
        f32 padded_size = widget->min.height;
        v2f tpos = bbox.pos;
        tpos.y -= padded_size/2.0;
        tpos.x += padded_size/2.0;
        push_triangle(tpos, padded_size, 0.25 + lerp(0.0, 0.25, widget->active_transition), ui->theme.text);
        bbox.pos.x += padded_size/2.0;
        widget_render_text(bbox.pos , widget, ui->theme.text);
    };
    
    widget->render_hook = render_hook;
    auto result = update_widget(widget);
    widget->min = get_text_size(widget->string, widget->style.font_scale);
    widget->min.width += 2.0*widget->min.height;
    
    
    if(result.clicked){
        widget->checked = !widget->checked;
    }
    else if(result.was_active && widget->checked){
        widget->checked = 0;
    }
    
    return widget->checked;
    
}

internal b32
arrow_dropdown2(char* fmt, ...){
    
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    auto widget = push_widget(string);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_RENDER_BACKGROUND);
    widget_set_property(widget, WP_HOVER_RENDER_BORDER);
    widget_set_property(widget, WP_RENDER_SHADOW);
    
    Widget_Style style = {};
    style.text_colour = v4f_from_colour(ui->theme.text);
    style.border_colour = v4f_from_colour(ui->theme.text);
    style.background_colour = v4f_from_colour(ui->theme.darker_background);
    style.font_scale = 0.7f;
    style.rounded_corner_amount = 5.0f;
    push_style(style);
    
    auto render_hook = [](Widget* widget) {
        auto bbox = v4f2(widget->pos, widget->min);
        f32 padded_size = widget->min.height;
        v2f tpos = bbox.pos;
        tpos.y -= padded_size/2.0;
        tpos.x += padded_size/2.0;
        push_triangle(tpos, padded_size, 0.25 + lerp(0.0, 0.25, widget->active_transition), ui->theme.text);
        bbox.pos.x += padded_size/2.0;
        widget_render_text(bbox.pos , widget, ui->theme.text);
    };
    
    widget->render_hook = render_hook;
    auto result = update_widget(widget);
    widget->min = get_text_size(widget->string, widget->style.font_scale);
    widget->min.width += 2.0*widget->min.height;
    
    if(result.clicked){
        widget->checked = !widget->checked;
    }
    else if(result.was_active && widget->checked){
        widget->checked = 0;
    }
    
    return widget->checked;
    
}

internal b32
arrow_dropdown3(char* fmt, ...){
    
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    auto widget = push_widget(string);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_FIRST_TRANSITION);
    widget_set_property(widget, WP_RENDER_TEXT);
    widget_set_property(widget, WP_HOVER_RENDER_BACKGROUND);
    widget_set_property(widget, WP_HOVER_INFLATE);
    
    Widget_Style style = {};
    style.text_colour = v4f_from_colour(ui->theme.text);
    style.border_colour = v4f_from_colour(ui->theme.text);
    style.background_colour = v4f_from_colour(ui->theme.darker_background);
    style.font_scale = 0.7f;
    style.rounded_corner_amount = 5.0f;
    push_style(style);
    
    auto result = update_widget(widget);
    widget->min = get_text_size(widget->string, widget->style.font_scale);
    widget->min.width *= 1.2f;
    
    if(result.clicked){
        widget->checked = !widget->checked;
    }
    else if(result.was_active && widget->checked){
        widget->checked = 0;
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
    push_default_style();
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_RENDER_TEXT);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_RENDER_BACKGROUND);
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_LERP_COLOURS);
    widget_set_property(widget, WP_HOVER_INFLATE);
    //widget_set_property(widget, WP_LERP_POSITION);
    auto result = update_widget(widget);
    return result.clicked;
    
}

internal b32
icon_button(Bitmap bitmap, char* fmt, ...){
    
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    
    Widget* widget = push_widget(string);
    
    Widget_Style style = {};
    style.text_colour = v4f_from_colour(ui->theme.text);
    style.border_colour = v4f_from_colour(ui->theme.text);
    style.background_colour = v4f_from_colour(ui->theme.darker_background);
    style.font_scale = 0.7f;
    style.rounded_corner_amount = 5.0f;
    push_style(style);
    
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_LERP_COLOURS);
    widget_set_property(widget, WP_HOVER_INFLATE);
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_HOVER_RENDER_BACKGROUND);
    //widget_set_property(widget, WP_LERP_POSITION);
    widget->bitmap = bitmap;
    auto render_hook = [](Widget* widget) {
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        push_rectangle_textured(v4f2(pos, widget->min), 3, widget->bitmap);
    };
    
    widget->render_hook = render_hook;
    
    auto result = update_widget(widget);
    
    widget->min = v2f(26, 26);
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
    push_default_style();
    widget_set_property(widget, WP_SPACING);
    widget_set_property(widget, WP_DRAGGABLE);
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_HOVER_INFLATE);
    //widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_LERP_COLOURS);
    
    auto render_hook = [](Widget* widget) {
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        v4f bbox = v4f2(pos, widget->min);
        
        v4f pc = bbox;
        pc.width *= (widget->value);
        push_rectangle(pc, 1, ui->theme.sub_colour);
        
        pos.x += 1;
        pos.y -= 1;
        
        f32 centre = pos.x + widget->min.x/2.0f;
        String8 string = make_stringf(&platform->frame_arena, "%.2f", widget->value);
        f32 text_centre = get_text_width(string, widget->style.font_scale)/2.0f;
        f32 text_x = centre - text_centre;
        push_string(v2f(text_x, bbox.y), string, ui->theme.text, widget->style.font_scale);
        
    };
    
    widget->render_hook = render_hook;
    widget->value = (*value - min)/(max - min);
    auto result = update_widget(widget);
    widget->min.height = get_font_line_height(widget->style.font_scale);
    if(ui->dragging && ui->active == widget->id){
        f32 x = (platform->mouse_position.x - result.pos.x)/result.size.width;
        *value = x*(max-min);
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
            ID("change type"){
                result = arrow_dropdown("%.*s", string.length, string.text);
            }
        }
        
    }
    
    return result;
}

internal void
text_box(String8* string){
    
    auto widget_string = make_stringf(&platform->frame_arena, "edit%d", (int)string);
    auto widget = push_widget(widget_string);
    
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_HOVER_RENDER_BORDER);
    widget_set_property(widget, WP_RENDER_BORDER);
    widget_set_property(widget, WP_HOVER_RENDER_BACKGROUND);
    widget_set_property(widget, WP_TEXT_EDIT);
    
    widget->text_edit_string = string;
    
    push_default_style();
    
    auto render_hook = [](Widget* widget) {
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        v4f bbox = v4f2(pos, widget->min);
        
        Colour colour = colour_from_v4f(widget->style.text_colour);
        
        f32 centre = pos.x + widget->min.x/2.0f;
        f32 text_centre = get_text_width(*widget->text_edit_string, widget->style.font_scale)/2.0f;
        f32 text_x = centre - text_centre;
        RENDER_CLIP(v4f2(pos, widget->min)){
            push_string(v2f(text_x, bbox.y), *widget->text_edit_string, colour, widget->style.font_scale);
        }
        
        if(ui->active == widget->id && ui->text_edit == widget->id){
            
            v2f next = {};
            next.x = text_x + get_text_width_n(*widget->text_edit_string, ui->cursor_pos, widget->style.font_scale);
            next.y = bbox.y;
            v2f cursor_size = v2f(1.5, widget->min.height*0.9f);
            
            lerp(&ui->v0.x, next.x, 0.1f);
            lerp(&ui->v0.y, next.y, 0.1f);
            
            lerp(&ui->v1.x, next.x, 0.2f);
            lerp(&ui->v1.y, next.y+cursor_size.height/2.0, 0.3f);
            
            lerp(&ui->v2.x, next.x, 0.1f);
            lerp(&ui->v2.y, next.y+cursor_size.height, 0.3f);
            
            push_bezier(ui->v0, ui->v1, ui->v2, 2, ui->theme.cursor);
            
        }
    };
    
    widget->render_hook = render_hook;
    
    auto result = update_widget(widget);
    widget->min = get_text_size(*string, widget->style.font_scale);
    if(result.clicked){
        ui->cursor_pos = string->length;
        ui->text_edit = widget->id;
    }
}
