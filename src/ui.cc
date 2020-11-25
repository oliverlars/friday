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
gen_unique_id(char* label){
    int size = strlen(label);
    UI_ID result = 0;
    hash32(&result, label, size);
    return result;
}

internal UI_ID
gen_unique_id(String8 label){
    int size = label.length;
    UI_ID result = 0;
    hash32(&result, label.text, size);
    return result;
}

internal UI_ID
gen_id(char* label){
    
    u64 id = (u64)(void*)label;
    return (UI_ID)id;
}

internal UI_ID
gen_id(String8 label){
    
    u64 id = (u64)(void*)label.text;
    return (UI_ID)id;
}

internal UI_ID
gen_unique_id(void* data){
    u64 id = (u64)data;
    return (UI_ID)id;
}

internal UI_ID
gen_unique_id(void* data, int num_bytes){
    UI_ID result = 0;
    hash32(&result, (char*)data, num_bytes);
    return (UI_ID)result;
}

internal Widget*
make_widget(String8 string){
    
}

internal Widget*
push_widget(String8 string){
    
}
