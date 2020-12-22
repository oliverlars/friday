
internal b32
platform_is_event_mouse(Platform_Event* event){
    return event->type > PLATFORM_EVENT_MOUSE_BEGIN &&
        event->type < PLATFORM_EVENT_MOUSE_END;
}

internal b32
platform_compare_events(Platform_Event a, Platform_Event b){
    return (a.type == b.type &&
            a.key == b.key &&
            a.mouse_button == b.mouse_button &&
            a.character == b.character &&
            a.modifiers == b.modifiers);
    
}

internal Platform_Event
platform_key_press(Key key, Key_Modifiers modifiers){
    Platform_Event event;
    event.type = PLATFORM_EVENT_KEY_PRESS;
    event.key = key;
    event.modifiers = modifiers;
    return event;
}

internal Platform_Event
platform_key_release(Key key, Key_Modifiers modifiers){
    Platform_Event event;
    event.type = PLATFORM_EVENT_KEY_RELEASE;
    event.key = key;
    event.modifiers = modifiers;
    return event;
}

internal Platform_Event
platform_character_input(u64 character){
    Platform_Event event;
    event.type = PLATFORM_EVENT_CHARACTER_INPUT;
    event.character = character;
    return event;
}

internal Platform_Event
platform_mouse_move(v2f position, v2f delta){
    Platform_Event event;
    event.type = PLATFORM_EVENT_MOUSE_MOVE;
    event.position = position;
    event.delta = delta;
    return event;
}

internal Platform_Event
platform_mouse_drag(v2f position, v2f delta){
    Platform_Event event;
    event.type = PLATFORM_EVENT_MOUSE_DRAG;
    event.position = position;
    event.delta = delta;
    return event;
}

internal Platform_Event
platform_mouse_press(Mouse_Button button, v2f position){
    Platform_Event event;
    event.type = PLATFORM_EVENT_MOUSE_PRESS;
    event.mouse_button = button;
    event.position = position;
    return event;
}

internal Platform_Event
platform_mouse_release(Mouse_Button button, v2f position){
    Platform_Event event;
    event.type = PLATFORM_EVENT_MOUSE_RELEASE;
    event.mouse_button = button;
    event.position = position;
    return event;
}

internal Platform_Event
platform_mouse_scroll(v2f delta, Key_Modifiers modifiers){
    Platform_Event event;
    event.type = PLATFORM_EVENT_MOUSE_SCROLL;
    event.scroll = delta;
    event.modifiers = modifiers;
    return event;
}

internal b32
platform_get_next_event(Platform_Event** event){
    b32 result = 0;
    u32 start_index = 0;
    Platform_Event* new_event = 0;
    if(*event){
        start_index = (*event - platform->events) + 1;
    }
    for(int i = start_index; i < platform->event_count; i++){
        if(platform->events[i].type != PLATFORM_EVENT_INVALID){
            new_event = platform->events + i;
            break;
        }
    }
    *event = new_event;
    result = new_event != 0;
    return result;
}

internal void
platform_consume_event(Platform_Event* event){
    event->type  = PLATFORM_EVENT_INVALID;
}

internal void
platform_begin_frame(){
    platform->pump_events = 0;
}

internal void
platform_end_frame(){
    platform->current_time += 1.0f / platform->target_fps;
    
    Platform_Event *event = 0;
    for (;platform_get_next_event(&event);){
        platform_consume_event(event);
    }
}

internal void
platform_push_event(Platform_Event event){
    hard_assert(platform != 0);
    if(platform->event_count < ArrayCount(platform->events)){
        platform->events[platform->event_count++] = event;
    }
}