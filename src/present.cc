
internal void
present_cursor(){
    auto widget = push_widget(make_string("cursor"));
    widget_set_property(widget, WP_RENDER_HOOK);
    
    auto render_hook = [](Widget* widget){
        push_rectangle(v4f2(widget->pos, widget->min), 1, ui->theme.cursor);
    };
    widget->render_hook = render_hook;
    v2f size = get_text_size(widget->string);
    widget->min = v2f(1, size.height);
    update_widget(widget);
}

internal void
present_string(Widget* widget, Colour colour){
    f32 offset = widget->hot_transition*10.0f;
    v4f bbox = v4f2(widget->pos, widget->min);
    bbox = inflate_rect(bbox, offset);
    f32 scale = text_scale_from_pixels(widget->string, widget->hot_transition*20.0f);
    v2f delta;
    if(ui->hot == widget->id && has_left_clicked()){
        ui->active = widget->id;
    }
    if(0 && widget->id == ui->active){ //NOTE(Oliver): this'll be fun later!
        widget->pos.x = platform->mouse_position.x;
        widget->pos.y = platform->mouse_position.y;
        widget->pos.y -= widget->min.height;
        widget->pos.x -= widget->min.width/2.0f;
    }
    for(int i = 0; i < widget->string.length; i++){
        v2f mp = platform->mouse_position;
        v2f pos = v2f(widget->pos.x, widget->pos.y);
        push_stringi(pos, widget->string, colour, i);
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


internal void
present_as_c(){
    UI_ROW {
        xspacer(20);
        present_keyword("int");
        xspacer();
        present_id("test variable");
        xspacer();
        present_misc("=");
        xspacer();
        present_literal("1024");
        xspacer();
        present_misc("+");
        xspacer();
        present_literal("2048");
    }
    
}

internal void
present_as_jai(){
    UI_ROW {
        xspacer(20);
        
        present_id("test variable");
        present_misc(":");
        xspacer();
        present_keyword("int");
        xspacer();
        present_misc("=");
        xspacer();
        present_literal("1024");
        xspacer();
        present_misc("+");
        xspacer();
        present_literal("2048");
    }
    
}

internal void
present_as_python(){
    UI_ROW {
        xspacer(20);
        
        present_id("test variable");
        xspacer();
        present_misc("=");
        xspacer();
        present_literal("1024");
        xspacer();
        present_misc("+");
        xspacer();
        present_literal("2048");
    }
    
}
internal void
present(int present_style){
    if(present_style == 0){
        present_as_jai();
    }else if(present_style == 1){
        present_as_c();
    }else if(present_style == 2){
        present_as_python();
    }
}