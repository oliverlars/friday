// NOTE(Oliver): stuff!

enum Navigation_Mode {
    NV_COMMAND,
    NV_MAKE,
    NV_EDIT,
    NV_DELETE,
};
struct {
    
    Navigation_Mode mode;
} navigator;

internal void
navigate_graph(Presenter* presenter){
    Present_Node* node_list = presenter->node_list;
    
    for(;node_list; node_list = node_list->next){
        if(node_list->node == presenter->active_node){
            if(platform.keys_pressed[SDL_SCANCODE_J]){
                if(node_list->next) {
                    presenter->active_node = node_list->next->node;
                }
                platform.keys_pressed[SDL_SCANCODE_J] = 0;
            }
            if(platform.keys_pressed[SDL_SCANCODE_K]){
                presenter->active_node = node_list->prev->node;
                platform.keys_pressed[SDL_SCANCODE_K] = 0;
            }
            if(platform.keys_pressed[SDL_SCANCODE_I]){
                presenter->should_edit = true;
                platform.keys_pressed[SDL_SCANCODE_I] = 0;
            }
            if(platform.keys_pressed[SDL_SCANCODE_LCTRL] &&
               platform.keys_pressed[SDL_SCANCODE_LEFTBRACKET]){
                presenter->should_edit = false;
                platform.keys_pressed[SDL_SCANCODE_LCTRL] = 0;
                platform.keys_pressed[SDL_SCANCODE_LEFTBRACKET] = 0;
                
            }
        }
    }
    
}