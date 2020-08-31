// NOTE(Oliver): stuff!

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
navigate_graph(Presenter* presenter){
    Present_Node* node_list = presenter->node_list;
    
    if(navigator.mode == NV_COMMAND){
        if(platform.keys_pressed[SDL_SCANCODE_D]){
            auto node = presenter->active_present_node;
            auto node_to_delete = node->node;
            remove_node_at(node_to_delete);
            friday.node_pool.clear(node_to_delete);
        }
        
        if(platform.keys_pressed[SDL_SCANCODE_J]){
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
        
        if(platform.keys_pressed[SDL_SCANCODE_K]){
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
        
        if(platform.keys_pressed[SDL_SCANCODE_L]){
            auto node = presenter->active_present_node;
            if(node->right){
                presenter->active_present_node = presenter->active_present_node->right;
            }
        }
        
        if(platform.keys_pressed[SDL_SCANCODE_H]){
            auto node = presenter->active_present_node;
            if(node->left){
                presenter->active_present_node = presenter->active_present_node->left;
            }
        }
        
        if(platform.keys_pressed[SDL_SCANCODE_I]){
            navigator.mode = NV_TEXT_EDIT;
            presenter->should_edit = true;
        }
        
        if(platform.keys_pressed[SDL_SCANCODE_M]){
            navigator.mode = NV_MAKE;
        }
    }
    
    if(navigator.mode == NV_TEXT_EDIT){
        if(platform.keys_pressed[SDL_SCANCODE_LCTRL] &&
           platform.keys_pressed[SDL_SCANCODE_LEFTBRACKET]){
            navigator.mode = NV_COMMAND;
            
            presenter->should_edit = false;
        }
    }
    if(navigator.mode == NV_MAKE){
        
        if(platform.keys_pressed[SDL_SCANCODE_B]){
            auto node = presenter->active_present_node;
            auto decl = make_declaration_node(&friday.node_pool, "untitled");
            node->node->function.scope->scope.statements->next = decl;
            navigator.mode = NV_COMMAND;
        }
        if(platform.keys_pressed[SDL_SCANCODE_D]){
            auto node = presenter->active_present_node;
            auto decl = make_declaration_node(&friday.node_pool, "untitled");
            insert_node_at(decl, node->node);
            navigator.mode = NV_COMMAND;
        }
        
        if(platform.keys_pressed[SDL_SCANCODE_C]){
            auto node = presenter->active_present_node;
            auto decl = make_conditional_node(&friday.node_pool, "untitled");
            insert_node_at(decl, node->node);
            navigator.mode = NV_COMMAND;
        }
        
        if(platform.keys_pressed[SDL_SCANCODE_L]){
            auto node = presenter->active_present_node;
            auto decl = make_loop_node(&friday.node_pool, "untitled");
            insert_node_at(decl, node->node);
            navigator.mode = NV_COMMAND;
        }
        if(platform.keys_pressed[SDL_SCANCODE_F]){
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
    if(platform.keys_pressed[SDL_SCANCODE_LCTRL] &&
       platform.keys_pressed[SDL_SCANCODE_LEFTBRACKET]){
        navigator.mode = NV_COMMAND;
        
        presenter->should_edit = false;
        
    }
    
}