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
    if(platform.keys_pressed[SDL_SCANCODE_J]){
        Present_Node* node_list = presenter->node_list;
        while(node_list){
            if(node_list->node == presenter->active_node){
                presenter->active_node = node_list->next->node;
                break;
            }
            node_list = node_list->next;
        }
        platform.keys_pressed[SDL_SCANCODE_J] = 0;
    }
    if(platform.keys_pressed[SDL_SCANCODE_K]){
        Present_Node* node_list = presenter->node_list;
        while(node_list){
            if(node_list->node == presenter->active_node){
                presenter->active_node = node_list->prev->node;
                break;
            }
            node_list = node_list->next;
        }
        platform.keys_pressed[SDL_SCANCODE_K] = 0;
    }
}