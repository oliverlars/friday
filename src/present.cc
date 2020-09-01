
struct Present_Node {
    Node* node;
    Present_Node* next = nullptr;
    Present_Node* prev = nullptr;
    
    Present_Node* left = nullptr;
    Present_Node* right = nullptr;
    Present_Node* up = nullptr;
    Present_Node* down = nullptr;
};

struct Presenter {
    
    Arena present_arena;
    Node* root;
    
    int x_start = 300;
    int x_offset = 0;
    int y_start = 480;
    int y_offset;
    int scroll_amount;
    int indent;
    
    String8* active_string;
    int cursor_index;
    
    Node* hover_node;
    
    Node* active_node;
    Node* current_node;
    
    Present_Node* node_list = nullptr;
    Present_Node** node_list_down_tail = nullptr;
    Present_Node** node_list_right_tail = nullptr;
    Present_Node** node_list_tail = nullptr;
    Present_Node* active_present_node;
    b32 should_edit;
    
    
    v4f cursor_rect;
    v4f cursor_target_rect;
    Colour cursor_colour;
    Colour cursor_target_colour;
    
    String8 token_list;
};


internal inline Present_Node*
allocate_present_node(Presenter* presenter){
    auto node = (Present_Node*)arena_allocate(&presenter->present_arena, sizeof(Present_Node));
    node->next = nullptr;
    node->prev = nullptr;
    node->left = nullptr;
    node->right = nullptr;
    node->up = nullptr;
    node->down = nullptr;
    return node;
}

internal inline void
set_current_node(Presenter* presenter, Node* node){
    if(!presenter->node_list){
        presenter->node_list = allocate_present_node(presenter);
        presenter->node_list->node = node;
        presenter->node_list->next = nullptr;
        presenter->node_list->prev = nullptr;
        presenter->node_list_tail = &presenter->node_list;
        if(!presenter->active_present_node){
            presenter->active_present_node = presenter->node_list;
        }
    }else{
        Present_Node* node_list = presenter->node_list;
        while(node_list->next){
            node_list = node_list->next;
        }
        node_list->next = allocate_present_node(presenter);
        node_list->next->node = node;
        node_list->next->next = nullptr;
        node_list->next->prev = node_list;
        presenter->node_list_tail = &node_list->next;
    }
    presenter->current_node = node;
}

internal inline void
set_current_right_node(Presenter* presenter, Node* node){
    if(!presenter->node_list){
        presenter->node_list = allocate_present_node(presenter);
        presenter->node_list->node = node;
        presenter->node_list_tail = &presenter->node_list;
        if(!presenter->active_present_node){
            presenter->active_present_node = presenter->node_list;
        }
    }else{
        Present_Node* node_tail = *presenter->node_list_tail;
        
        node_tail->right = allocate_present_node(presenter);
        node_tail->right->node = node;
        node_tail->right->left = node_tail;
        presenter->node_list_tail = &node_tail->right;
    }
}

internal inline void
set_current_down_node(Presenter* presenter, Node* node){
    if(!presenter->node_list){
        presenter->node_list = allocate_present_node(presenter);
        presenter->node_list->node = node;
        presenter->node_list_tail = &presenter->node_list;
        if(!presenter->active_present_node){
            presenter->active_present_node = presenter->node_list;
        }
    }else{
        Present_Node* node_tail = *presenter->node_list_tail;
        
        node_tail->down = allocate_present_node(presenter);
        node_tail->down->node = node;
        node_tail->down->up = node_tail;
        presenter->node_list_tail = &node_tail->down;
    }
    
}

internal b32
is_active_present_node(Presenter* presenter){
    return *presenter->node_list_tail == presenter->active_present_node;
}

internal inline int
get_presenter_x(Presenter* presenter) {
    return presenter->x_start + presenter->x_offset + presenter->indent;
}

internal inline int
get_presenter_y(Presenter* presenter) {
    return presenter->y_start + presenter->y_offset + platform.mouse_scroll_amount;
}

internal inline void
present_new_line(Presenter* presenter){
    presenter->y_offset -= renderer.fonts[0].line_height;
    presenter->x_offset = 0;
}
internal void
present_space(Presenter* presenter){
    presenter->x_offset += 10;
}

internal void
present_string(Presenter* presenter, char* string, u32 colour = 0xFFFFFFFF){
    push_string(get_presenter_x(presenter), get_presenter_y(presenter), string, colour);
    presenter->x_offset += get_text_width(string);
}

internal void
present_string(Presenter* presenter, String8 string, u32 colour = 0xFFFFFFFF){
    push_string8(get_presenter_x(presenter), get_presenter_y(presenter), string, colour);
    presenter->x_offset += get_text_width(string);
}

internal void
present_string(Presenter* presenter, f32 x, f32 y, String8 string, u32 colour = 0xFFFFFFFF){
    push_string8(x, y, string, colour);
    presenter->x_offset += get_text_width(string);
}

internal void
present_string(Presenter* presenter, f32 x, f32 y, char* string, u32 colour = 0xFFFFFFFF){
    push_string(x, y, string, colour);
    presenter->x_offset += get_text_width(string);
}

internal void
edit_string(Presenter* presenter, String8* string){
    if(platform.has_text_input){
        insert_in_string(string, platform.text_input, presenter->cursor_index);
        presenter->cursor_index += strlen(platform.text_input);
        platform.has_text_input = 0;
        OutputDebugStringA(platform.text_input);
        OutputDebugStringA("\n");
    }
    
    if(platform.keys_pressed[SDL_SCANCODE_LEFT]){
        presenter->cursor_index--;
        platform.keys_pressed[SDL_SCANCODE_LEFT] = 0;
    }
    
    if(platform.keys_pressed[SDL_SCANCODE_RIGHT]){
        presenter->cursor_index++;
        platform.keys_pressed[SDL_SCANCODE_RIGHT] = 0;
    }
    if(platform.keys_pressed[SDL_SCANCODE_BACKSPACE]){
        platform.keys_pressed[SDL_SCANCODE_BACKSPACE] = 0;
        pop_from_string(string, presenter->cursor_index);
        presenter->cursor_index--;
    }
}

internal void
present_highlighted_string(Presenter* presenter, String8 string, u32 colour = theme.panel.packed){
    
    f32 line_height = renderer.fonts[0].line_height;
    presenter->cursor_target_rect.x = get_presenter_x(presenter);
    presenter->cursor_target_rect.y = (get_presenter_y(presenter)-line_height/4);
    presenter->cursor_target_rect.z = get_text_width(string);
    presenter->cursor_target_rect.w = line_height;
    
    presenter->cursor_rect = add_rects(presenter->cursor_rect, 
                                       lerp_rects(presenter->cursor_rect, presenter->cursor_target_rect, 0.2f));
    v4f r = presenter->cursor_rect;
    v4f rt = presenter->cursor_target_rect;
    push_rectangle(r.x, r.y, r.z, r.w, 10, theme.button_highlight.packed);
    present_string(presenter, string, colour);
    
}

internal void
present_highlighted_string(Presenter* presenter, char* string, u32 colour = theme.panel.packed){
    
    f32 line_height = renderer.fonts[0].line_height;
    presenter->cursor_target_rect.x = get_presenter_x(presenter);
    presenter->cursor_target_rect.y = (get_presenter_y(presenter)-line_height/4);
    presenter->cursor_target_rect.z = get_text_width(string);
    presenter->cursor_target_rect.w = line_height;
    
    presenter->cursor_rect = add_rects(presenter->cursor_rect, 
                                       lerp_rects(presenter->cursor_rect, presenter->cursor_target_rect, 0.2f));
    v4f r = presenter->cursor_rect;
    v4f rt = presenter->cursor_target_rect;
    push_rectangle(r.x, r.y, r.z, r.w, 10, theme.button_highlight.packed);
    present_string(presenter, string, colour);
    
}


internal void
present_editable_string(Presenter* presenter, String8* string, u32 colour = theme.text.packed){
    
    auto id = gen_id(*string);
    auto widget = ui_push_widget(get_presenter_x(presenter),
                                 get_presenter_y(presenter),
                                 get_text_width(*string),
                                 renderer.fonts[0].line_height, 
                                 id, {});
    
    
    if(id == ui_state.clicked_id || 
       (is_active_present_node(presenter) && presenter->should_edit)){
        presenter->active_string = string;
        presenter->cursor_index = string->length;
        f32 text_width = get_text_width(*string);
        f32 offset = 5.0f;
        f32 width = text_width + offset;
        f32 line_height = renderer.fonts[0].line_height;
        f32 height = line_height;
        f32 x = get_presenter_x(presenter) - offset/2;
        f32 y = get_presenter_y(presenter) - line_height*0.25;
        
        if(presenter->active_string == string){
            f32 cursor_pos = get_text_width(*string);
            push_rectangle(3+x+cursor_pos, y, 3, height, 0.1, theme.cursor.packed);
            edit_string(presenter, string);
        }
    }else if(id == ui_state.hover_id){
    }
    
    if(is_active_present_node(presenter) && !presenter->should_edit){
        present_highlighted_string(presenter, *string, colour);
    }else{
        present_string(presenter, *string, colour);
    }
}

internal void
present_editable_string(Presenter* presenter, Node* node, u32 colour = theme.text.packed){
    
    auto id = gen_id(node->name);
    auto widget = ui_push_widget(get_presenter_x(presenter),
                                 get_presenter_y(presenter),
                                 get_text_width(node->name),
                                 renderer.fonts[0].line_height, 
                                 id, {});
    
    
    if(id == ui_state.clicked_id || 
       (is_active_present_node(presenter) && presenter->should_edit)){
        presenter->active_string = &node->name;
        presenter->cursor_index = node->name.length;
        f32 text_width = get_text_width(node->name);
        f32 offset = 5.0f;
        f32 width = text_width + offset;
        f32 line_height = renderer.fonts[0].line_height;
        f32 height = line_height;
        f32 x = get_presenter_x(presenter) - offset/2;
        f32 y = get_presenter_y(presenter) - line_height*0.25;
        
        if(presenter->active_string == &node->name){
            f32 cursor_pos = get_text_width(node->name);
            push_rectangle(3+x+cursor_pos, y, 3, height, 0.1, theme.cursor.packed);
            edit_string(presenter, &node->name);
        }
    }else if(id == ui_state.hover_id){
    }
    
    if(is_active_present_node(presenter) && !presenter->should_edit){
        present_highlighted_string(presenter, node->name, colour);
    }else{
        present_string(presenter, node->name, colour);
    }
}

internal void
present_selectable_string(Presenter* presenter, char* string, u32 colour = theme.text.packed){
    
    auto id = gen_id(string);
    auto widget = ui_push_widget(get_presenter_x(presenter),
                                 get_presenter_y(presenter),
                                 get_text_width(string),
                                 renderer.fonts[0].line_height, 
                                 id, {});
    
    
    if(id == ui_state.clicked_id || is_active_present_node(presenter)){
        
    }else if(id == ui_state.hover_id){
    }
    
    if(is_active_present_node(presenter)){
        present_highlighted_string(presenter, string, colour);
    }else{
        present_string(presenter, string, colour);
    }
}


#define INSERTABLE_WIDTH 20
internal void
present_x_insertable(Presenter* presenter,  Closure closure, char* label, ...){
    
    UI_ID id;
    {
        va_list args;
        va_start(args, label);
        char fmt_label[256];
        int size = vsnprintf(fmt_label, 256, label, args);
        fmt_label[size] = '\n';
        fmt_label[size+1] = 0;
        id = gen_unique_id(fmt_label);
    }
    
    auto widget = ui_push_widget(get_presenter_x(presenter)-INSERTABLE_WIDTH/2,
                                 get_presenter_y(presenter),
                                 INSERTABLE_WIDTH,
                                 renderer.fonts[0].line_height, 
                                 id, closure);
    
    auto anim_state = get_animation_state(id);
    if(!anim_state){
        anim_state = init_animation_state(id);
        anim_state->target_rect.x = 40.0f;
    }
    if(ui_state.hover_id == id){
        animate(anim_state);
    }else{
        unanimate(anim_state);
    }
    
    presenter->x_offset += anim_state->rect.x;
}

internal void
present_x_insertable(Presenter* presenter, Closure closure, String8 label){
    
    auto id = gen_id(label);
    auto widget = ui_push_widget(get_presenter_x(presenter)-INSERTABLE_WIDTH/2,
                                 get_presenter_y(presenter),
                                 INSERTABLE_WIDTH,
                                 renderer.fonts[0].line_height, 
                                 id, closure);
    
    auto anim_state = get_animation_state(id);
    if(!anim_state){
        anim_state = init_animation_state(id);
        anim_state->target_rect.x = 40.0f;
    }
    if(ui_state.hover_id == id){
        animate(anim_state);
    }else{
        unanimate(anim_state);
    }
    
    draw_view_buttons();
    
    presenter->x_offset += anim_state->rect.x;
}

#define INSERTABLE_Y_WIDTH 200

internal void
present_y_insertable(Presenter* presenter,  Closure closure, char* label, ...){
    
    UI_ID id;
    {
        va_list args;
        va_start(args, label);
        char fmt_label[256];
        int size = vsnprintf(fmt_label, 256, label, args);
        fmt_label[size] = '\n';
        fmt_label[size+1] = 0;
        id = gen_unique_id(fmt_label);
    }
    
    auto widget = ui_push_widget(get_presenter_x(presenter)-INSERTABLE_Y_WIDTH/2,
                                 get_presenter_y(presenter),
                                 INSERTABLE_Y_WIDTH,
                                 renderer.fonts[0].line_height, 
                                 id, closure);
    
    auto anim_state = get_animation_state(id);
    if(!anim_state){
        anim_state = init_animation_state(id);
        anim_state->target_rect.y = renderer.fonts[0].line_height;
    }
    if(ui_state.hover_id == id){
        animate(anim_state);
    }else{
        unanimate(anim_state);
    }
    
    presenter->y_offset -= anim_state->rect.y;
}

internal void
present_y_insertable(Presenter* presenter, Closure closure, String8 label){
    
    auto id = gen_id(label);
    auto widget = ui_push_widget(get_presenter_x(presenter)-INSERTABLE_Y_WIDTH/2,
                                 get_presenter_y(presenter),
                                 INSERTABLE_Y_WIDTH,
                                 renderer.fonts[0].line_height, 
                                 id, closure);
    
    auto anim_state = get_animation_state(id);
    if(!anim_state){
        anim_state = init_animation_state(id);
        anim_state->target_rect.y = renderer.fonts[0].line_height;
    }
    if(ui_state.hover_id == id){
        animate(anim_state);
    }else{
        unanimate(anim_state);
    }
    
    presenter->y_offset -= anim_state->rect.y;
}

internal void
present_pop_indent(Presenter* presenter){
    presenter->indent -= 40;
}

internal void
present_push_indent(Presenter* presenter){
    presenter->indent += 40;
}

#define present_indent(presenter) defer_loop(present_push_indent(presenter), present_pop_indent(presenter))

internal void
present_misc(Presenter* presenter, char* string, u32 colour = theme.text_misc.packed){
    present_string(presenter, string, colour);
}

internal void
present_misc(Presenter* presenter, String8 string, u32 colour = theme.text_misc.packed){
    present_string(presenter, string, colour);
}

internal void present_graph(Presenter* presenter, Node* root);

internal void
present_binary_literal(Presenter* presenter, Node* node){
    //set_current_node(presenter, node);
    auto literal = &node->literal;
    Arena* arena = &renderer.temp_string_arena;
    char* string = (char*)arena_allocate(arena, 256);
    snprintf(string, 256, "%d", literal->_int);
    present_string(presenter, string, theme.text_literal.packed);
}


internal void
present_literal_node(Presenter* presenter, Node* node){
    auto literal = &node->literal;
    set_current_right_node(presenter, node);
    char buffer[256];
    int length = snprintf(buffer, 256, "%d", literal->_int);
    buffer[length+1] = 0;
    present_selectable_string(presenter, buffer, theme.text_literal.packed);
    
}

internal void
present_binary_node(Presenter* presenter, Node* node){
    //set_current_node(presenter, node);
    auto binary = &node->binary;
    present_literal_node(presenter, binary->left);
    
    switch(binary->op_type){
        case OP_PLUS: present_misc(presenter, " + ");break;
        case OP_MINUS: present_misc(presenter, " - ");break;
        case OP_DIVIDE: present_misc(presenter, " / ");break;
        case OP_MULTIPLY: present_misc(presenter, " * ");break;
        case OP_LT: present_misc(presenter, " < ");break;
        case OP_EQ: present_misc(presenter, " == ");break;
        case OP_GT: present_misc(presenter, " > ");break;
        case OP_LTE: present_misc(presenter, " <= ");break;
        case OP_NEQ: present_misc(presenter, " != ");break;
        case OP_GTE: present_misc(presenter, " >= ");break;
    }
    
    present_literal_node(presenter, binary->right);
}

internal void
insert_parameters_for_function(u8* parameters){
    return;
    auto params = get_arg(parameters, Node*);
    
    while(params->next){
        params = params->next;
    }
    params->next = make_node(&friday.node_pool, NODE_DECLARATION, "arg");
    params->next->declaration.type_usage = _u16;
    
}


internal void present_type_usage_node(Presenter* presenter, Node* node);

internal void
present_function_node(Presenter* presenter, Node* node){
    //set_current_node(presenter, node);
    
    auto function = &node->function;
    present_editable_string(presenter, node, theme.text_function.packed);
    
    present_misc(presenter, " :: (");
    Closure closure = make_closure(insert_parameters_for_function, 1, arg(function->parameters));
    for(Node* param = function->parameters; param; param = param->next){
        if(param->type != NODE_DUMMY){
            set_current_right_node(presenter, param);
            present_editable_string(presenter, &param->name);
            present_misc(presenter, ":");
            present_space(presenter);
            present_type_usage_node(presenter, param->declaration.type_usage);
            if(param->next){
                present_misc(presenter, ",");
                present_space(presenter);
            }
        }
    }
    //present_x_insertable(presenter, closure, node->name);
    present_x_insertable(presenter, closure, "arg%d", (u64)node);
    
    present_misc(presenter, ")");
    if(function->return_type){
        present_space(presenter);
        present_misc(presenter, "->");
        present_space(presenter);
        present_graph(presenter, function->return_type);
    }
    present_space(presenter);
    present_misc(presenter, "{");
    
    present_new_line(presenter);
    
    present_indent(presenter){
        present_graph(presenter, function->scope);
    }
    
    
    present_misc(presenter, "}");
    present_new_line(presenter);
    
}

internal void
present_type_usage_node(Presenter* presenter, Node* node){
    set_current_right_node(presenter, node);
    present_space(presenter);
    for(int i = 0; i < node->type_usage.number_of_pointers; i++){
        present_misc(presenter, "*", theme.text.packed);
    }
    present_editable_string(presenter, &node->type_usage.type_reference->name,
                            theme.text_type.packed);
    present_space(presenter);
}

internal void
insert_type_for_declaration(u8* parameters){
    auto root = get_arg(parameters, Node*);
    auto decl = &root->declaration;
    decl->type_usage = make_node(&friday.node_pool, NODE_TYPE_USAGE);
    Node* node_list[10];
    find_node_types(node_list, 10, NODE_STRUCT);
    
    decl->type_usage->type_usage.type_reference = node_list[0];
}

internal void
present_editable_token_list(Presenter* presenter, Node* node){
    
    auto token = node;
    
#if 0
    auto id = gen_id(node->name);
    auto widget = ui_push_widget(get_presenter_x(presenter),
                                 get_presenter_y(presenter),
                                 get_text_width(node->name),
                                 renderer.fonts[0].line_height, 
                                 id, {});
    
    
    if(id == ui_state.clicked_id || 
       (is_active_present_node(presenter) && presenter->should_edit)){
        presenter->active_string = &node->name;
        presenter->cursor_index = node->name.length;
        f32 text_width = get_text_width(node->name);
        f32 offset = 5.0f;
        f32 width = text_width + offset;
        f32 line_height = renderer.fonts[0].line_height;
        f32 height = line_height;
        f32 x = get_presenter_x(presenter) - offset/2;
        f32 y = get_presenter_y(presenter) - line_height*0.25;
        
        if(presenter->active_string == &node->name){
            f32 cursor_pos = get_text_width(node->name);
            push_rectangle(3+x+cursor_pos, y, 3, height, 0.1, theme.cursor.packed);
            edit_string(presenter, &node->name);
        }
    }else if(id == ui_state.hover_id){
    }
    
    if(is_active_present_node(presenter) && !presenter->should_edit){
        present_highlighted_string(presenter, node->name, colour);
    }else{
        present_string(presenter, node->name, colour);
    }
#endif
}

internal void
present_editable_token_list(Presenter* presenter, String8 string, u32 colour){
    
    if(is_active_present_node(presenter)){
        present_highlighted_string(presenter, string, colour);
    }else {
        present_misc(presenter, string, colour);
    }
    
}

internal void
present_token_node(Presenter* presenter, Node* node){
    auto token = node;
    
    for(;token; token = token->next){
        Colour colour = theme.text_misc;
        switch(token->token.token_type){
            case TOKEN_LITERAL:{
                colour = theme.text_literal;
            }break;
            case TOKEN_REFERENCE:{
                colour = theme.text;
            }break;
        }
        set_current_right_node(presenter, token);
        present_editable_token_list(presenter, token->name, colour.packed);
        present_space(presenter);
    }
}

internal void
present_declaration_node(Presenter* presenter, Node* node){
    
    auto decl = &node->declaration;
    present_editable_string(presenter, &node->name);
    present_misc(presenter, " :");
    
    Closure closure = make_closure(insert_type_for_declaration, 1, arg(node));
    present_x_insertable(presenter, closure, "test%d", (u64)node);
    
    present_graph(presenter, decl->type_usage);
    present_misc(presenter, "= ");
    if(decl->is_initialised){
        present_graph(presenter, decl->expression);
    }else {
        present_misc(presenter, "void");
    }
    present_new_line(presenter);
}


internal void
insert_members_for_struct(u8* parameters){
    auto members = get_arg(parameters, Node*);
    while(members->next){
        members = members->next;
    }
    members->next = make_declaration_node(&friday.node_pool,"untitled");
}

internal void
present_struct_node(Presenter* presenter, Node* node){
    //set_current_node(presenter, node);
    auto _struct = &node->_struct;
    present_editable_string(presenter, &node->name, theme.text_type.packed);
    present_misc(presenter, " :: struct {");
    present_new_line(presenter);
    
    present_indent(presenter){
        for(Node* stmt = _struct->members; stmt; stmt = stmt->next){
            if(stmt->type != NODE_DUMMY){
                present_declaration_node(presenter, stmt);
            }
        }
    }
    Closure closure = make_closure(insert_members_for_struct, 1, arg(_struct->members));
    present_y_insertable(presenter, closure, "member%d", get_presenter_y(presenter));
    present_misc(presenter, "}");
}


internal void
present_scope_node(Presenter* presenter, Node* node){
    //set_current_node(presenter, node);
    auto scope = &node->scope;
    Node* stmt = scope->statements;
    Node* prev = stmt;
    for(; stmt; stmt = stmt->next){
        if(stmt->type != NODE_DUMMY){
            set_current_down_node(presenter, stmt);
        }
        if(prev->type != stmt->type){
            present_new_line(presenter);
        }
        present_graph(presenter, stmt);
        prev = stmt;
        
    }
}

internal void
present_loop_node(Presenter* presenter, Node* node){
    auto loop = &node->loop;
    present_selectable_string(presenter, "loop", theme.text.packed);
    present_space(presenter);
    
    present_graph(presenter, loop->min);
    present_space(presenter);
    present_misc(presenter, "to", theme.text.packed);
    present_space(presenter);
    present_graph(presenter, loop->max);
    present_space(presenter);
    present_misc(presenter, "{");
    present_new_line(presenter);
    present_indent(presenter){
        present_graph(presenter, loop->scope);
    }
    present_misc(presenter, "}");
    present_new_line(presenter);
}

internal void
present_conditional_node(Presenter* presenter, Node* node){
    auto cond = &node->conditional;
    present_selectable_string(presenter, "if", theme.text.packed);
    present_space(presenter);
    
    present_graph(presenter, cond->condition);
    
    present_space(presenter);
    present_misc(presenter, "{");
    present_new_line(presenter);
    present_indent(presenter){
        present_graph(presenter, cond->scope);
    }
    present_misc(presenter, "}");
    present_new_line(presenter);
}

internal void
reset_presenter(Presenter* presenter){
    if(!presenter) return;
    presenter->x_offset = 0;
    presenter->y_offset = 0;
    presenter->indent = 0;
    arena_reset(&presenter->present_arena);
    
}

internal void
present_graph(Presenter* presenter, Node* root){
    if(!root) return;
    switch(root->type){
        case NODE_BINARY:{
            present_binary_node(presenter, root);
        }break;
        case NODE_LITERAL: {
            present_literal_node(presenter, root);
        }break;
        case NODE_UNARY:{
            
        }break;
        case NODE_STRUCT: {
            present_struct_node(presenter, root);
        }break;
        case NODE_ENUM: {
        }break;
        case NODE_UNION: {
        }break;
        case NODE_SCOPE: {
            present_scope_node(presenter, root);
        }break;
        case NODE_TYPE_USAGE: {
            present_type_usage_node(presenter, root);
        }break;
        case NODE_DECLARATION: {
            present_declaration_node(presenter, root);
        }break;
        case NODE_IDENTIFIER: {
        }break;
        case NODE_FUNCTION: {
            present_function_node(presenter, root);
        }break;
        case NODE_CONDITIONAL: {
            present_conditional_node(presenter, root);
        }break;
        case NODE_LOOP: {
            present_loop_node(presenter, root);
        }break;
        case NODE_CALL: {
        }break;
        case NODE_TOKEN:{
            present_token_node(presenter, root);
        }break;
    }
    
}

internal void
present(Presenter* presenter){
    if(!presenter) return;
    present_graph(presenter, presenter->root);
    
}


enum Navigation_Mode {
    NV_COMMAND,
    NV_MAKE,
    NV_EDIT,
    NV_TEXT_EDIT,
    NV_DELETE,
};
struct {
    Navigation_Mode mode = NV_COMMAND;
} navigator;

internal void
draw_status_bar(Presenter* active_presenter){
    int size = 40;
    int x = 0;
    int width = platform.width-10;
    push_rectangle(x, -size, width, size*2, 10, theme.panel.packed);
    
    char* file = " active node: "; 
    push_string(x, size/2-renderer.fonts[0].line_height/4, 
                file, theme.text_misc.packed);
    x += get_text_width(file);
    switch(active_presenter->active_present_node->node->type){
        case NODE_DECLARATION:{
            char* str = "declaration";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
        case NODE_TYPE_USAGE:{
            char* str = "type usage";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
        case NODE_FUNCTION:{
            char* str = "function";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
        case NODE_LOOP:{
            char* str = "loop";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
        case NODE_CONDITIONAL:{
            char* str = "conditional";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
        case NODE_LITERAL:{
            char* str = "literal";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
        case NODE_BINARY:{
            char* str = "binary";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
        case NODE_STRUCT:{
            char* str = "struct";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
    }
    
    x = width/3;
    char* mode = "active mode: ";
    
    push_string(x, size/2-renderer.fonts[0].line_height/4, 
                mode, theme.text_misc.packed);
    x += get_text_width(file);
    
    switch(navigator.mode){
        case NV_COMMAND:{
            char* str = "command";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
        case NV_MAKE:{
            char* str = "make";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
        case NV_EDIT:{
            char* str = "edit";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
        case NV_TEXT_EDIT:{
            char* str = "text edit";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
        case NV_DELETE:{
            char* str = "delete";
            push_string(x, size/2-renderer.fonts[0].line_height/4, 
                        str, theme.text.packed);
        }break;
    }
}
