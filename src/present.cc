
internal void
present_cursor(){
    auto widget = push_widget(make_string("cursor"));
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_LERP_POSITION);
    
    auto render_hook = [](Widget* widget){
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        push_rectangle(v4f2(pos, widget->min), 1, ui->theme.cursor);
    };
    
    widget->render_hook = render_hook;
    v2f size = get_text_size(widget->string);
    widget->min = v2f(2, size.height);
    update_widget(widget);
}

internal void
present_string(Widget* widget, Colour colour){
    f32 offset = widget->hot_transition*10.0f;
    v2f pos = widget->pos;
    pos.y -= widget->min.height;
    v4f bbox = v4f2(pos, widget->min);
    bbox = inflate_rect(bbox, offset);
    f32 scale = text_scale_from_pixels(widget->string, widget->hot_transition*20.0f);
    v2f delta;
    
    if(widget_has_property(widget, WP_LERP_COLOURS)){
        lerp_rects(&widget->style.text_colour, v4f_from_colour(colour), 0.1f);
    }
    push_string(pos, widget->string, colour_from_v4f(widget->style.text_colour), widget->style.font_scale + 1.0f);
}

internal void
present_keyword(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 string = make_stringfv(&platform->frame_arena, fmt, args);
    va_end(args);
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_LERP_COLOURS);
    widget_set_property(widget, WP_CLICKABLE);
    if(ui->active == widget->id){
        present_cursor();
    }
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
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_TEXT_EDIT);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_LERP_COLOURS);
    widget_set_property(widget, WP_CLICKABLE);
    if(ui->active == widget->id){
        present_cursor();
    }
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
    widget_set_property(widget, WP_TEXT_EDIT);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_LERP_COLOURS);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_LERP_POSITION);
    if(ui->active == widget->id){
        present_cursor();
    }
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
    widget_set_property(widget, WP_TEXT_EDIT);
    widget_set_property(widget, WP_CLICKABLE);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_LERP_COLOURS);
    widget_set_property(widget, WP_CLICKABLE);
    
    if(ui->active == widget->id){
        present_cursor();
    }
    
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
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_LERP_COLOURS);
    widget_set_property(widget, WP_CLICKABLE);
    
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
    UI_COLUMN{
        UI_ROW {
            present_keyword("void");
            xspacer();
            present_id("big func");
            present_misc("()");
            xspacer();
            present_misc("{");
        }
        UI_ROW {
            xspacer(40);
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
        UI_ROW {
            present_misc("}");
        }
    }
}

internal void
present_as_jai(){
    UI_COLUMN{
        UI_ROW {
            present_id("big func");
            xspacer();
            present_misc("::");
            xspacer();
            present_misc("()");
            xspacer();
            present_misc("{");
        }
        UI_ROW {
            xspacer(40);
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
        UI_ROW {
            present_misc("}");
        }
    }
}

internal void
present_as_python(){
    UI_COLUMN{
        UI_ROW {
            present_keyword("def");
            xspacer();
            present_id("big func");
            xspacer();
            present_misc("()");
            present_misc(":");
        }
        UI_ROW {
            xspacer(40);
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
}

internal void
present_as_rust() {
    UI_COLUMN{
        UI_ROW {
            present_keyword("fn");
            xspacer();
            present_id("big func");
            present_misc("()");
            xspacer();
            present_misc("{");
        }
        UI_ROW {
            xspacer(40);
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
        UI_ROW {
            present_misc("}");
        }
    }
}

internal void
present_as_go() {
    UI_COLUMN{
        UI_ROW {
            present_keyword("func");
            xspacer();
            present_id("big func");
            present_misc("()");
            xspacer();
            present_misc("{");
        }
        UI_ROW {
            xspacer(40);
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
        UI_ROW {
            present_misc("}");
        }
    }
}

internal void
present_as_pascal() {
    UI_COLUMN{
        UI_ROW {
            present_keyword("procedure");
            xspacer();
            present_id("big func");
            present_misc("()");
        }
        UI_ROW {
            present_keyword("begin");
        }
        UI_ROW {
            xspacer(40);
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
            present_literal("2048");}
        UI_ROW {
            present_keyword("end");
        }
    }
}

internal void
present(int present_style){
    switch(present_style){
        case 0: present_as_c();break;
        case 1: present_as_jai();break;
        case 2: present_as_python();break;
        case 3: present_as_pascal();break;
    }
    
}