
// TODO(Oliver): maybe improve this?
// needed because all globals are wiped on hot load

struct Serial_Node {
    int marker;
    Arc_Node node;
    char string[256];
};

struct Editor_State {
    Pool arc_pool;
    
    Arc_Node* root;
    
    Arc_Node* builtins;
    
    Pool string_pool;
    
    String8 views[256];
    int view_count;
    
    int serial_index;
    Serial_Node serialise[1024];
    
    int deserial_index;
    Arc_Node* deserialise[1024];
    
};

Editor_State* editor;

struct Friday_Globals {
    Renderer_State* renderer;
    UI_State* ui;
    Editor_State* editor;
    Presenter_State* presenter;
};
