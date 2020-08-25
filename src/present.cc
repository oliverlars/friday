
struct Present_Node {
    
    Node* node;
    v4f rect;
    Animation_State* anim;
    char name[256];
    Present_Node* next;
    Present_Node* prev;
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
};


internal inline void
set_current_node(Presenter* presenter, Node* node){
    presenter->current_node = node;
}

internal inline int
get_presenter_x(Presenter* presenter) {
    return presenter->x_start + presenter->x_offset + presenter->indent;
}

internal inline int
get_presenter_y(Presenter* presenter) {
    return presenter->y_start + presenter->y_offset;
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
present_highlighted_string(Presenter* presenter, String8 string){
    f32 line_height = renderer.fonts[0].line_height;
    push_rectangle(get_presenter_x(presenter), get_presenter_y(presenter) - line_height/4, 
                   get_text_width(string), line_height, 10, theme.cursor.packed);
    present_string(presenter, string, theme.panel.packed);
}

internal void
present_highlighted_string(Presenter* presenter, char* string){
    
    f32 line_height = renderer.fonts[0].line_height;
    push_rectangle(get_presenter_x(presenter), get_presenter_y(presenter)-line_height/4, 
                   get_text_width(string), line_height, 10, theme.cursor.packed);
    present_string(presenter, string, theme.panel.packed);
}

internal void
present_editable_string(Presenter* presenter, String8* string, u32 colour = theme.text.packed){
    
    auto id = gen_id(*string);
    auto widget = ui_push_widget(get_presenter_x(presenter),
                                 get_presenter_y(presenter),
                                 get_text_width(*string),
                                 renderer.fonts[0].line_height, 
                                 id, {});
    
    
    if(id == ui_state.clicked_id){
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
    }else{
    }
    if(presenter->current_node == presenter->active_node){
        present_highlighted_string(presenter, *string);
    }else{
        present_string(presenter, *string, colour);
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
    set_current_node(presenter, node);
    auto literal = &node->literal;
    Arena* arena = &renderer.temp_string_arena;
    char* string = (char*)arena_allocate(arena, 256);
    snprintf(string, 256, "%d", literal->_int);
    present_string(presenter, string, theme.text_literal.packed);
}

internal void
present_binary_node(Presenter* presenter, Node* node){
    set_current_node(presenter, node);
    auto binary = &node->binary;
    present_binary_literal(presenter, binary->left);
    
    switch(binary->op_type){
        case OP_PLUS: present_misc(presenter, " + ");break;
        case OP_MINUS: present_misc(presenter, " - ");break;
        case OP_DIVIDE: present_misc(presenter, " / ");break;
        case OP_MULTIPLY: present_misc(presenter, " * ");break;
    }
    
    present_binary_literal(presenter, binary->right);
}

internal void
insert_parameters_for_function(u8* parameters){
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
    set_current_node(presenter, node);
    
    auto function = &node->function;
    present_editable_string(presenter, &node->name, theme.text_function.packed);
    
    present_misc(presenter, " :: (");
    Closure closure = make_closure(insert_parameters_for_function, 1, arg(function->parameters));
    for(Node* param = function->parameters; param; param = param->next){
        set_current_node(presenter, param);
        if(param->type != NODE_DUMMY){
            present_editable_string(presenter, &param->name);
            present_misc(presenter, ":");
            present_space(presenter);
            present_string(presenter, param->declaration.type_usage->name, theme.text_type.packed);
            
            if(param->next){
                present_misc(presenter, ",");
                present_space(presenter);
            }
        }
    }
    //present_x_insertable(presenter, closure, node->name);
    present_x_insertable(presenter, closure, "arg%d", (int)node);
    
    present_misc(presenter, ") {");
    
    present_new_line(presenter);
    
    present_indent(presenter){
        present_graph(presenter, function->scope);
    }
    
    present_new_line(presenter);
    
    present_misc(presenter, "}");
}

internal void
present_type_usage_node(Presenter* presenter, Node* node){
    set_current_node(presenter, node);
    
    present_space(presenter);
    present_editable_string(presenter, &node->name,
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
present_declaration_node(Presenter* presenter, Node* node){
    set_current_node(presenter, node);
    
    auto decl = &node->declaration;
    present_editable_string(presenter, &node->name);
    present_misc(presenter, " :");
    
    Closure closure = make_closure(insert_type_for_declaration, 1, arg(node));
    present_x_insertable(presenter, closure, "test%d", (int)node);
    present_graph(presenter, decl->type_usage);
    present_misc(presenter, "= ");
    if(decl->is_initialised){
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
    set_current_node(presenter, node);
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
    set_current_node(presenter, node);
    auto scope = &node->scope;
    Node* stmt = scope->statements;
    for(; stmt; stmt = stmt->next){
        present_graph(presenter, stmt);
    }
}

internal void
reset_presenter(Presenter* presenter){
    if(!presenter) return;
    presenter->x_offset = 0;
    presenter->y_offset = 0;
    presenter->indent = 0;
}

internal void
present_graph(Presenter* presenter, Node* root){
    if(!root) return;
    switch(root->type){
        case NODE_BINARY:{
            present_binary_node(presenter, root);
        }break;
        case NODE_UNARY:{
            
        }break;
        case NODE_STRUCT: {
            present_struct_node(presenter, root);
            present_new_line(presenter);
            present_new_line(presenter);
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
        }break;
        case NODE_LOOP: {
        }break;
        case NODE_CALL: {
        }break;
        
    }
    
}

internal void
present(Presenter* presenter){
    if(!presenter) return;
    present_graph(presenter, presenter->root);
    
}