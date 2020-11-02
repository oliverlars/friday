
struct Arena {
    void* base;
    u64 size;
    u64 alloc_position;
    u64 commit_position;
};


struct Pool_Node {
    Pool_Node* next;
};

const int default_pool_block_size = 4096;

struct Pool_Block {
    u8* memory;
    u64 size;
    u64 used;
    Pool_Block* next;
};

struct Pool {
    
    Pool_Block* first = nullptr;
    Pool_Block* active = nullptr;
    Pool_Node* free_head = nullptr;
    
    u64 chunk_size;
    
};
