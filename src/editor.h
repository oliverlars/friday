
enum Editor_Mode {
    E_INVALID,
    E_EDIT,
    E_CREATE,
};

struct Editor_State {
    Pool arc_pool;
    
    Arc_Node* root;
    
    Arc_Node* builtins;
    Arc_Node* stdlib;
    
    Pool string_pool;
    
    String8 views[256];
    int view_count;
    
    b32 should_reload;
    
    Editor_Mode mode;
    
    String8 file_location;
};

Editor_State* editor;
