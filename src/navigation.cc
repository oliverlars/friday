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
            platform.keys_pressed[SDL_SCANCODE_J] = 0;
        }
        if(platform.keys_pressed[SDL_SCANCODE_K]){
            auto node = presenter->active_present_node;
            if(node->up){
                presenter->active_present_node = presenter->active_present_node->up;
            }else if(node->left){
                auto left = node->left;
                while(left->left){
                    left = left->left;
                }
                if(left->up){
                    presenter->active_present_node = left->up;
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
            platform.keys_pressed[SDL_SCANCODE_K] = 0;
        }
        if(platform.keys_pressed[SDL_SCANCODE_L]){
            auto node = presenter->active_present_node;
            if(node->right){
                presenter->active_present_node = presenter->active_present_node->right;
            }
            debug_print("%d\n", (int)presenter->active_present_node);
            platform.keys_pressed[SDL_SCANCODE_L] = 0;
        }
        if(platform.keys_pressed[SDL_SCANCODE_H]){
            auto node = presenter->active_present_node;
            if(node->left){
                presenter->active_present_node = presenter->active_present_node->left;
            }
            platform.keys_pressed[SDL_SCANCODE_H] = 0;
        }
        if(platform.keys_pressed[SDL_SCANCODE_I]){
            navigator.mode = NV_TEXT_EDIT;
            presenter->should_edit = true;
            platform.keys_pressed[SDL_SCANCODE_I] = 0;
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
            platform.keys_pressed[SDL_SCANCODE_LCTRL] = 0;
            platform.keys_pressed[SDL_SCANCODE_LEFTBRACKET] = 0;
            
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
            if(node->node->next){
                auto temp = node->node->next;
                node->node->next = decl;
                node->node->next->next = temp;
            }else {
                node->node->next = decl;
            }
            
            navigator.mode = NV_COMMAND;
        }
        if(platform.keys_pressed[SDL_SCANCODE_F]){
            auto node = presenter->active_present_node;
            auto func = make_function_node(&friday.node_pool, "untitled");
            if(node->node->next){
                auto temp = node->node->next;
                node->node->next = func;
                node->node->next->next = temp;
            }else {
                node->node->next = func;
            }
            navigator.mode = NV_COMMAND;
        }
        if(platform.keys_pressed[SDL_SCANCODE_P]){
            auto node = presenter->active_present_node;
            auto param = make_declaration_node(&friday.node_pool, "arg");
            auto params = node->node->function.parameters;
            while(params->next){
                params = params->next;
            }
            params->next = param;
            params->next->declaration.type_usage = _u16;
            navigator.mode = NV_COMMAND;
            platform.keys_pressed[SDL_SCANCODE_P] = 0;
            
        }
    }
    
}