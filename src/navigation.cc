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
    
    for(;node_list; node_list = node_list->next){
        debug_print("%d %d\n", (int)node_list, (int)presenter->active_present_node);
        
        if(node_list == presenter->active_present_node){
            if(navigator.mode == NV_COMMAND){
                if(platform.keys_pressed[SDL_SCANCODE_J]){
                    if(node_list->next) {
                        presenter->active_present_node = node_list->next;
                        debug_print("%d\n", (int)presenter->active_present_node);
                    }
                    platform.keys_pressed[SDL_SCANCODE_J] = 0;
                }
                if(platform.keys_pressed[SDL_SCANCODE_K]){
                    debug_print("%d\n", (int)presenter->active_present_node);
                    presenter->active_present_node = node_list->prev;
                    platform.keys_pressed[SDL_SCANCODE_K] = 0;
                }
                if(platform.keys_pressed[SDL_SCANCODE_I]){
                    navigator.mode = NV_TEXT_EDIT;
                    presenter->should_edit = true;
                    platform.keys_pressed[SDL_SCANCODE_I] = 0;
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
        }
    }
    
}