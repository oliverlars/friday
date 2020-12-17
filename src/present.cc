
internal void
present_keyword(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_HOOK);
    
    auto render_hook = [](Widget* widget){
        widget_render_text(widget, ui->theme.text_type);
        
    };
    widget->render_hook = render_hook;
    v4f bbox = get_text_bbox({}, widget->string);
    widget->min = bbox.size;
    update_widget(widget);
}
internal void
present_literal(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_HOOK);
    
    auto render_hook = [](Widget* widget){
        widget_render_text(widget, ui->theme.text_literal);
        
    };
    widget->render_hook = render_hook;
    v4f bbox = get_text_bbox({}, widget->string);
    widget->min = bbox.size;
    update_widget(widget);
}

internal void
present_function(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_HOOK);
    
    auto render_hook = [](Widget* widget){
        widget_render_text(widget, ui->theme.text_function);
        
    };
    widget->render_hook = render_hook;
    v4f bbox = get_text_bbox({}, widget->string);
    widget->min = bbox.size;
    update_widget(widget);
}

internal void
present_id(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_HOOK);
    
    auto render_hook = [](Widget* widget){
        widget_render_text(widget, ui->theme.text);
        
    };
    widget->render_hook = render_hook;
    v4f bbox = get_text_bbox({}, widget->string);
    widget->min = bbox.size;
    update_widget(widget);
}

internal void
present_misc(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_HOOK);
    
    auto render_hook = [](Widget* widget){
        widget_render_text(widget, ui->theme.text_misc);
        
    };
    widget->render_hook = render_hook;
    v4f bbox = get_text_bbox({}, widget->string);
    widget->min = bbox.size;
    update_widget(widget);
}
