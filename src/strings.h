struct String8 {
    u64 length;
    char* text;
};

struct String8_Node {
    String8_Node* next;
    String8 string;
};

struct String8_List {
    String8_Node* first;
    String8_Node* last;
};

struct Lexer {
    char* pos;
};
