
struct Present_Node {
    
    Node* node;
    v4f rect;
    Animation_State* anim;
    char name[256];
    Present_Node* next;
    Present_Node* prev;
};

struct {
    
    Arena present_arena;
    Present_Node* present_nodes_head;
    
    
    int x_start = 480;
    int x_offset = 0;
    int y_start = 480;
    int y_offset;
    int scroll_amount;
    int indent;
    
} presenter;



internal inline int
get_presenter_x() {
    return presenter.x_start + presenter.x_offset;
}

internal inline int
get_presenter_y() {
    return presenter.y_start + presenter.y_offset;
}

internal inline void
present_new_line(){
    presenter.y_offset -= renderer.fonts[0].line_height;
    presenter.x_offset = 0;
    
}

internal void
present_pop_indent(){
    presenter.indent -= 40;
}

internal void
present_push_indent(){
    presenter.indent += 40;
}

internal void
present_string(char* string, u32 colour = 0xFFFFFFFF){
    push_string(get_presenter_x(), get_presenter_y(), string, colour);
    presenter.x_offset += get_text_width(string);
}

internal void
present_string(String8 string, u32 colour = 0xFFFFFFFF){
    push_string8(get_presenter_x(), get_presenter_y(), string, colour);
    presenter.x_offset += get_text_width(string);
}

internal void
present_misc(char* string, u32 colour = theme.text.packed){
    present_string(string, colour);
}

internal void
present_misc(String8 string, u32 colour = theme.text.packed){
    present_string(string, colour);
}

internal void
present_binary_literal(Node* node){
    auto literal = &node->literal;
    
    Arena* arena = &renderer.temp_string_arena;
    char* string = (char*)arena_allocate(arena, 256);
    snprintf(string, 256, "%d", literal->_int);
    present_string(string, theme.text_literal.packed);
}

internal void
present_binary_node(Node* node){
    auto binary = &node->binary;
    present_binary_literal(binary->left);
    
    String8 op = {};
    Arena* arena = &renderer.temp_string_arena;
    switch(binary->op_type){
        case OP_PLUS: op = make_string(arena, " + ");break;
        case OP_MINUS: op = make_string(arena, " - ");break;
        case OP_DIVIDE: op = make_string(arena, " / ");break;
        case OP_MULTIPLY: op = make_string(arena, " * ");break;
    }
    present_misc(op);
    
    present_binary_literal(binary->right);
}

internal void
present_function_node(Node* node){
    auto function = &node->function;
    present_string(node->name, theme.text_function.packed);
    present_misc(" :: () {");
    
    present_new_line();
    present_new_line();
    
    present_misc("}");
}

internal void
reset_presenter(){
    presenter.x_offset = 0;
    presenter.y_offset = 0;
}

internal void
present_graph(Node* root){
    if(!root) return;
    switch(root->type){
        case NODE_BINARY:{
            present_binary_node(root);
        }break;
        case NODE_UNARY:{
        }break;
        case NODE_STRUCT: {
            
        }break;
        case NODE_ENUM: {
        }break;
        case NODE_UNION: {
        }break;
        case NODE_SCOPE: {
        }break;
        case NODE_TYPE_USAGE: {
        }break;
        case NODE_DECLARATION: {
        }break;
        case NODE_IDENTIFIER: {
        }break;
        case NODE_FUNCTION: {
            present_function_node(root);
        }break;
        case NODE_CONDITIONAL: {
        }break;
        case NODE_LOOP: {
        }break;
        case NODE_CALL: {
        }break;
        
    }
    reset_presenter();
    
}