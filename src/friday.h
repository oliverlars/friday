
// TODO(Oliver): maybe improve this?
// needed because all globals are wiped on hot load

struct Editor_State {
    Pool ast_pool;
    
    Pool arc_pool;
    
    Pool block_pool;
    
    Ast_Node* program;
    Arc_Node* root;
    
    Pool string_pool;
    
    Block* block_start;
};

Editor_State* editor;

struct Friday_Globals {
    Renderer_State* renderer;
    UI_State* ui;
    Editor_State* editor;
    Presenter_State* presenter;
};
