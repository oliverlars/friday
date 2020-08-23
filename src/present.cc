
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
    
    int x_start = 480;
    int x_offset = 0;
    int y_start = 480;
    int y_offset;
    int scroll_amount;
    int indent;
    
    String8* active_string;
    int cursor_index;
    
    Node* active_node;
};


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
present_editable_string(Presenter* presenter, String8* string, u32 colour = theme.text.packed){
    
    Closure closure = make_closure(nullptr, 0);
    auto id = gen_id(*string);
    auto widget = ui_push_widget(get_presenter_x(presenter),
                                 get_presenter_y(presenter),
                                 get_text_width(*string),
                                 renderer.fonts[0].line_height, 
                                 id, closure);
    
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
    }
    present_string(presenter, *string, colour);
}


#define INSERTABLE_WIDTH 40
internal void
present_insertable(Presenter* presenter, char* label, Closure closure){
    
    auto callback = [](u8* parameters){};
    closure = make_closure(callback, 0);
    auto id = gen_id(label);
    auto widget = ui_push_widget(get_presenter_x(presenter)-INSERTABLE_WIDTH/2,
                                 get_presenter_y(presenter),
                                 INSERTABLE_WIDTH,
                                 renderer.fonts[0].line_height, 
                                 id, closure);
    
    auto anim_state = get_animation_state(id);
    if(ui_state.hover_id == id){
        animate(anim_state);
    }else{
        unanimate(anim_state);
    }
    
    presenter->x_offset += anim_state->rect.x;
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
    auto literal = &node->literal;
    
    Arena* arena = &renderer.temp_string_arena;
    char* string = (char*)arena_allocate(arena, 256);
    snprintf(string, 256, "%d", literal->_int);
    present_string(presenter, string, theme.text_literal.packed);
}

internal void
present_binary_node(Presenter* presenter, Node* node){
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
present_function_node(Presenter* presenter, Node* node){
    auto function = &node->function;
    present_editable_string(presenter, &node->name, theme.text_function.packed);
    
    present_misc(presenter, " :: () {");
    
    present_new_line(presenter);
    
    present_indent(presenter){
        present_graph(presenter, function->scope);
    }
    
    present_new_line(presenter);
    
    present_misc(presenter, "}");
}

internal void
present_type_usage_node(Presenter* presenter, Node* node){
    auto type_usage = &node->type_usage;
    present_string(presenter, node->name);
}

internal void
present_declaration_node(Presenter* presenter, Node* node){
    auto decl = &node->declaration;
    present_editable_string(presenter, &node->name);
    present_misc(presenter, " :");
    if(is_mouse_in_rect(get_presenter_x(presenter),
                        get_presenter_y(presenter),
                        INSERTABLE_WIDTH,
                        renderer.fonts[0].line_height)){
        
        //present_insertable(presenter, "type_inserter", {});
    }
    present_insertable(presenter, "type_inserter", {});
    
    present_misc(presenter, "= ");
    if(decl->expression){
    }else {
        present_misc(presenter, "void");
    }
    present_graph(presenter, decl->type_usage);
}

internal void
present_struct_node(Presenter* presenter, Node* node){
    auto _struct = &node->_struct;
    present_string(presenter, node->name, theme.text_type.packed);
    present_misc(presenter, " :: struct {");
    present_new_line(presenter);
    for(Node* stmt = _struct->members; stmt; stmt = stmt->next){
        present_declaration_node(presenter, stmt);
        present_new_line(presenter);
        
    }
    present_misc(presenter, "}");
}


internal void
present_scope_node(Presenter* presenter, Node* node){
    auto scope = &node->scope;
    
    for(Node* stmt = scope->statements; stmt; stmt = stmt->next){
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