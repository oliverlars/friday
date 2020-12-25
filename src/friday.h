
// TODO(Oliver): maybe improve this?
// needed because all globals are wiped on hot load

struct Editor_State {
    Pool ast_pool;
    
    Ast_Node* program;
};

Editor_State* editor;

struct Friday_Globals {
    Renderer_State* renderer;
    UI_State* ui;
    Editor_State* editor;
};
