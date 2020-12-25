
#define SPACE (font_scale*20)

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
present_string(Colour colour, String8 string){
    
    auto widget = push_widget(string);
    widget_set_property(widget, WP_RENDER_HOOK);
    widget_set_property(widget, WP_LERP_POSITION);
    widget_set_property(widget, WP_LERP_COLOURS);
    widget_set_property(widget, WP_CLICKABLE);
    widget->style.text_colour = v4f_from_colour(colour);
    auto render_hook = [](Widget* widget ){
        v2f pos = widget->pos;
        pos.y -= widget->min.height;
        
        push_string(pos, widget->string, colour_from_v4f(widget->style.text_colour), widget->style.font_scale);
    };
    
    widget->render_hook = render_hook;
    update_widget(widget);
    v2f size = get_text_size(widget->string, widget->style.font_scale);
    widget->min = size;
    widget->style.font_scale = font_scale;
    
}

internal void
present_scope(Ast_Node* node, int present_style){
    auto statement = node->scope.statements;
    UI_COLUMN {
        for(; statement; statement = statement->next){
            xspacer(SPACE);
            UI_ROW{
                present_graph(statement, present_style);
            }
        }
    }
}

internal void
present_function(Ast_Node* node, int present_style){
    auto function = &node->function;
    auto parameters = function->parameters;
    UI_COLUMN{
        switch(present_style){
            case 0: {
                ID("%d", (int)node) {
                    UI_ROW  {
                        present_graph(function->return_type, present_style);
                        present_string(ui->theme.text_function, node->name);
                        xspacer(SPACE);
                        present_string(ui->theme.text_misc, make_string("("));
                        for(;parameters; parameters = parameters->next){
                            present_graph(parameters, present_style);
                        }
                        present_string(ui->theme.text_misc, make_string(")"));
                        xspacer(SPACE);
                        present_string(ui->theme.text_misc, make_string("{"));
                    }
                    UI_ROW {
                        present_graph(function->scope, present_style);
                    }
                    UI_ROW {
                        present_string(ui->theme.text_misc, make_string("}"));
                    }
                    
                }
                
            }break;
            case 1: {
                
                ID("%d", (int)node) {
                    UI_ROW  {
                        present_string(ui->theme.text_function, node->name);
                        xspacer(SPACE);
                        present_string(ui->theme.text_misc, make_string("("));
                        for(;parameters; parameters = parameters->next){
                            present_graph(parameters, present_style);
                        }
                        present_string(ui->theme.text_misc, make_string(")"));
                        xspacer(SPACE);
                        present_string(ui->theme.text_misc, make_string("->"));
                        xspacer(SPACE);
                        present_graph(function->return_type, present_style);
                        present_string(ui->theme.text_misc, make_string("{"));
                    }
                    UI_ROW {
                        present_graph(function->scope, present_style);
                    }
                    UI_ROW {
                        present_string(ui->theme.text_misc, make_string("}"));
                    }
                    
                }
                
            }break;
            case 2: {
                
                ID("%d", (int)node) {
                    UI_ROW  {
                        present_string(ui->theme.text_type, make_string("def"));
                        xspacer(SPACE);
                        present_string(ui->theme.text_function, node->name);
                        xspacer(SPACE);
                        present_string(ui->theme.text_misc, make_string("("));
                        for(;parameters; parameters = parameters->next){
                            present_graph(parameters, present_style);
                        }
                        present_string(ui->theme.text_misc, make_string("):"));
                        present_graph(function->return_type, present_style);
                    }
                    UI_ROW {
                        present_graph(function->scope, present_style);
                    }
                    
                }
                
            }break;
            case 3: {
                
                ID("%d", (int)node) {
                    UI_ROW  {
                        present_string(ui->theme.text_type, make_string("procedure"));
                        xspacer(SPACE);
                        present_string(ui->theme.text_function, node->name);
                        xspacer(SPACE);
                        present_string(ui->theme.text_misc, make_string("("));
                        for(;parameters; parameters = parameters->next){
                            present_graph(parameters, present_style);
                        }
                        present_string(ui->theme.text_misc, make_string("):"));
                        present_graph(function->return_type, present_style);
                    }
                    UI_ROW {
                        present_string(ui->theme.text_type, make_string("begin"));
                    }
                    UI_ROW {
                        present_graph(function->scope, present_style);
                    }
                    UI_ROW {
                        present_string(ui->theme.text_type, make_string("end"));
                    }
                }
                
            }break;
        }
    }
}
internal void
present_declaration(Ast_Node* node, int present_style){
    auto decl = &node->declaration;
    
    switch(present_style){
        case 0:{
            UI_ROW {
            }
        }break;
        case 1:{
        }break;
        case 2:{
        }break;
        case 3:{
        }break;
    }
}

internal void
present_graph(Ast_Node* node, int present_style){
    
    if(!node) return;
    
    switch(node->type){
        case AST_INVALID:{
        }break;
        case AST_BINARY:{
        }break;
        case AST_UNARY:{
        }break;
        case AST_LITERAL:{
        }break;
        case AST_STRUCT:{
        }break;
        case AST_ENUM:{
        }break;
        case AST_UNION:{
        }break;
        case AST_SCOPE:{
            present_scope(node, present_style);
        }break;
        case AST_TYPE_USAGE:{
        }break;
        case AST_DECLARATION:{
        }break;
        case AST_IDENTIFIER:{
        }break;
        case AST_FUNCTION:{
            present_function(node, present_style);
        }break;
        case AST_CONDITIONAL:{
        }break;
        case AST_LOOP:{
        }break;
        case AST_CALL:{
        }break;
        case AST_TOKEN:{
        }break;
    }
}


