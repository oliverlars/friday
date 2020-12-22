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

struct String8_Cap {
    u64 capacity;
    union {
        String8 string;
        struct {
            u64 length;
            char* text;
        };
    };
};

struct String_Heap_Node {
    String_Heap_Node* next;
    
};

struct String_Heap {
    Arena arena;
    
};

struct Lexer {
    char* pos;
};
