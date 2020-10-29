const u64 default_memory_block_size = 4096; 

struct Arena_Block {
    u8* memory;
    u64 size;
    u64 used;
    Arena_Block* next;
};

struct Arena {
    
    Arena_Block* first = nullptr;
    Arena_Block* active = nullptr;
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
