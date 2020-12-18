
internal void
present_string(Widget* widget, Colour colour){
    if(ui->hot == widget->id){
        push_string(widget->pos, widget->string, colour, 1.0f + 0.5*sinf(3.14*widget->hot_transition));
    }else {
        push_string(widget->pos, widget->string, colour);
    }
}

internal void
present_keyword(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_HOOK);
    
    auto render_hook = [](Widget* widget){
        present_string(widget, ui->theme.text_type);
    };
    widget->render_hook = render_hook;
    v2f size = get_text_size(widget->string);
    widget->min = size;
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
        present_string(widget, ui->theme.text_literal);
    };
    widget->render_hook = render_hook;
    v2f size = get_text_size(widget->string);
    widget->min = size;
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
        present_string(widget, ui->theme.text_function);
    };
    widget->render_hook = render_hook;
    v2f size = get_text_size(widget->string);
    widget->min = size;
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
        present_string(widget, ui->theme.text);
    };
    widget->render_hook = render_hook;
    v2f size = get_text_size(widget->string);
    widget->min = size;
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
        present_string(widget, ui->theme.text_misc);
    };
    widget->render_hook = render_hook;
    v2f size = get_text_size(widget->string);
    widget->min = size;
    update_widget(widget);
}
