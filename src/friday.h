
// TODO(Oliver): maybe improve this?
// needed because all globals are wiped on hot load

struct Serial_Node {
    b32 marker;
    Arc_Node node;
    char string[256];
};

struct Arc_Format {
    s32 version_number;
    Serial_Node* nodes;
};

struct Friday_Globals {
    Renderer_State* renderer;
    UI_State* ui;
    Editor_State* editor;
    Presenter_State* presenter;
};

internal void
frame_graph();