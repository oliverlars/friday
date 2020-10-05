// NOTE(Oliver): stuff!


internal void
navigate_graph(Presenter* presenter){
    Present_Node* node_list = presenter->node_list;
    
    if(navigator.mode == NV_COMMAND){
        if(platform.keys_pressed[KEY_D]){
            auto node = presenter->active_present_node;
            auto node_to_delete = node->node;
            remove_node_at(node_to_delete);
            friday.node_pool.clear(node_to_delete);
        }
        
        if(platform.keys_pressed[KEY_P]){
            auto node = presenter->active_present_node;
            if(node->node->type == NODE_TYPE_USAGE){
                node->node->type_usage.number_of_pointers++;
            }
        }
        if(platform.keys_pressed[KEY_9]){
            auto node = presenter->active_present_node;
            if(node->node->type == NODE_TOKEN){
                auto token = make_token_misc_node(&friday.node_pool, "(");
                insert_node_at(token, node->node);
            }
        }
        if(platform.keys_pressed[KEY_0]){
            auto node = presenter->active_present_node;
            if(node->node->type == NODE_TOKEN){
                auto token = make_token_misc_node(&friday.node_pool, ")");
                insert_node_at(token, node->node);
            }
        }
        if(was_pressed(input.navigate_down)){
            auto node = presenter->active_present_node;
            if(node->down){
                presenter->active_present_node = presenter->active_present_node->down;
            }else if(node->left){
                auto left = node->left;
                while(left->left){
                    left = left->left;
                }
                if(left->down){
                    presenter->active_present_node = left->down;
                }
            }else if(node->right){
                auto right = node->right;
                while(right->right){
                    right = right->right;
                }
                if(right->down){
                    presenter->active_present_node = right->down;
                }
            }
        }
        
        if(was_pressed(input.navigate_up)){
            auto node = presenter->active_present_node;
            if(node->up){
                auto node_up = node->up;
                while(node_up->left){
                    node_up = node_up->left;
                }
                presenter->active_present_node = node_up;
            }else if(node->left){
                auto left = node->left;
                while(left->left){
                    left = left->left;
                }
                if(left->up){
                    auto left_up = left->up;
                    while(left_up->left){
                        left_up = left_up->left;
                    }
                    presenter->active_present_node = left_up;
                }
            }else if(node->right){
                auto right = node->right;
                while(right->right){
                    right = right->right;
                }
                if(right->up){
                    presenter->active_present_node = right->up;
                }
            }
        }
        
        if(was_pressed(input.navigate_right)){
            auto node = presenter->active_present_node;
            if(node->right){
                presenter->active_present_node = presenter->active_present_node->right;
            }
        }
        
        if(was_pressed(input.navigate_left)){
            auto node = presenter->active_present_node;
            if(node->left){
                presenter->active_present_node = presenter->active_present_node->left;
            }
        }
        
        if(was_pressed(input.enter_text_edit_mode)){
            navigator.mode = NV_TEXT_EDIT;
            presenter->should_edit = true;
        }
        
        if(was_pressed(input.enter_make_mode)){
            navigator.mode = NV_MAKE;
        }
    }
    
    if(navigator.mode == NV_TEXT_EDIT){
        if(platform.keys_pressed[KEY_CTRL] &&
           platform.keys_pressed[KEY_LBRACKET]){
            navigator.mode = NV_COMMAND;
            
            presenter->should_edit = false;
        }
    }
    if(navigator.mode == NV_MAKE){
        
        if(platform.keys_pressed[KEY_B]){
            auto node = presenter->active_present_node;
            auto decl = make_declaration_node(&friday.node_pool, "untitled");
            switch(node->node->type){
                case NODE_FUNCTION:{
                    node->node->function.scope->scope.statements->next = decl;
                }break;
                case NODE_LOOP:{
                    node->node->loop.scope->scope.statements->next = decl;
                }break;
                case NODE_CONDITIONAL:{
                    node->node->conditional.scope->scope.statements->next = decl;
                }break;
            }
            navigator.mode = NV_COMMAND;
        }
        if(was_pressed(input.make_decl)){
            auto node = presenter->active_present_node;
            auto decl = make_declaration_node(&friday.node_pool, "untitled");
            insert_node_at(decl, node->node);
            navigator.mode = NV_COMMAND;
        }
        
        if(platform.keys_pressed[KEY_E]){
            auto node = presenter->active_present_node;
            auto np = &friday.node_pool;
            auto token_list = make_token_misc_node(np, "(");
            token_list->prev = nullptr;
            token_list->next = make_token_literal_node(np, "1");
            token_list->next->prev = token_list;
            token_list->next->next = make_token_misc_node(np, "+");
            token_list->next->next->prev = token_list->next;
            
            token_list->next->next->next = make_token_literal_node(np, "2");
            token_list->next->next->next->prev = token_list->next->next;
            
            token_list->next->next->next->next = make_token_misc_node(np, ")");
            token_list->next->next->next->next->prev = token_list->next->next->next;
            
            token_list->next->next->next->next->next = make_token_misc_node(np, "*");
            token_list->next->next->next->next->next->prev = token_list->next->next->next->next;
            
            token_list->next->next->next->next->next->next = make_token_literal_node(np, "3");
            token_list->next->next->next->next->next->next->prev = token_list->next->next->next->next->next;
            
            node->node->declaration.expression = token_list;
            node->node->declaration.is_initialised = true;
            navigator.mode = NV_COMMAND;
        }
        
        if(platform.keys_pressed[KEY_C]){
            auto node = presenter->active_present_node;
            auto decl = make_conditional_node(&friday.node_pool, "untitled");
            insert_node_at(decl, node->node);
            navigator.mode = NV_COMMAND;
        }
        
        if(platform.keys_pressed[KEY_L]){
            auto node = presenter->active_present_node;
            auto decl = make_loop_node(&friday.node_pool, "untitled");
            insert_node_at(decl, node->node);
            navigator.mode = NV_COMMAND;
        }
        
        if(platform.keys_pressed[KEY_F]){
            auto node = presenter->active_present_node;
            auto func = make_function_node(&friday.node_pool, "untitled");
            insert_node_at(func, node->node);
            navigator.mode = NV_COMMAND;
        }
        
        if(platform.keys_pressed[SDL_SCANCODE_A]){
            auto node = presenter->active_present_node;
            if(node->node->type == NODE_FUNCTION){
                auto param = make_declaration_node(&friday.node_pool, "arg");
                auto params = node->node->function.parameters;
                while(params->next){
                    params = params->next;
                }
                params->next = param;
            }
            navigator.mode = NV_COMMAND;
            
        }
    }
    if(was_pressed(input.enter_command_mode)){
        navigator.mode = NV_COMMAND;
        
        presenter->should_edit = false;
        
    }
    
}