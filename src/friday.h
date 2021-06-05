
// TODO(Oliver): maybe improve this?
// needed because all globals are wiped on hot load

struct Editor_State {
    Pool arc_pool;
    
    Arc_Node* root;
    
    Arc_Node* builtins;
    
    Pool string_pool;
    
    String8 views[256];
    int view_count;
};

Editor_State* editor;

struct Friday_Globals {
    Renderer_State* renderer;
    UI_State* ui;
    Editor_State* editor;
    Presenter_State* presenter;
};
