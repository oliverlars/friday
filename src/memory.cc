
internal void* 
arena_allocate(Arena* arena, u64 size){
    if(!arena->active  || 
       arena->active->used + size > arena->active->size){
        if(arena->active && arena->active->next){
            assert(true);
            // NOTE(Oliver): we must have already allocated some
            // blocks, lets reuse those!!!
            arena->active = arena->active->next;
            goto end; // NOTE(Oliver): lazy...
        }
        
        u64 bytes_required = default_memory_block_size;
        
        if(size > bytes_required){
            bytes_required = size;
        }
        
        Arena_Block* new_block = 0;
        
        new_block = (Arena_Block*)calloc(1, sizeof(Arena_Block) + bytes_required);
        assert(new_block);
        new_block->memory = (u8*)new_block + sizeof(Arena_Block);
        new_block->size = bytes_required;
        new_block->next = nullptr;
        new_block->used = 0;
        
        if(arena->active){
            arena->active->next = new_block;
            arena->active = new_block;
        }else{
            arena->first = new_block;
            arena->active = new_block;
        }
    }
    end: 
    void* memory = arena->active->memory + arena->active->used;
    arena->active->used += size;
    
    return memory;
}

internal Arena
make_arena(u64 size, void* backing){
    Arena result;
    result.first = (Arena_Block*)backing;
    result.first->memory = (u8*)backing + sizeof(Arena_Block);
    result.first->size = size - sizeof(Arena_Block);
    result.first->next = nullptr;
    result.first->used = 0;
    result.active = result.first;
    return result;
}

internal void
arena_reset(Arena* arena){
    
    for(auto block = arena->first; block; block = block->next){
        block->used = 0;
    }
    arena->active = arena->first;
    
}

internal Arena
subdivide_arena(Arena* arena, u64 size){
    Arena result = {};
    result.first = (Arena_Block*)arena_allocate(arena, size + sizeof(Arena_Block));
    result.first->size = size + sizeof(Arena_Block);
    result.first->memory = (u8*)result.first + sizeof(Arena_Block);
    result.first->next = nullptr;
    result.first->used = 0;
    result.active = result.first;
    return result;
}

void pool_clear(Pool* pool, void* pointer){
    bool is_valid_pointer = false;
    for(auto block = pool->first; block; block = block->next){
        void* start = block->memory;
        void* end = &block->memory[block->size];
        if(!pointer){
            return;
        }
        
        if(start <= pointer && pointer < end){
            is_valid_pointer = true;
        }
    }
    if(!is_valid_pointer) return;
    
    Pool_Node* node;
    
    node = (Pool_Node*)pointer;
    node->next = pool->free_head;
    pool->free_head = node;
}

void pool_reset(Pool* pool){
    for(auto block = pool->first; block; block = block->next){
        void* start = block->memory;
        void* end = &block->memory[block->size-1];
        
        //active->clear(chunk_size);
        for(int i = 0; i < pool->active->size/pool->chunk_size; i++){
            void* pointer = &block->memory[i * pool->chunk_size];
            Pool_Node* node = reinterpret_cast<Pool_Node*>(pointer);
            node->next = pool->free_head;
            pool->free_head = node;
        }
        
    }
    
}

internal void* 
pool_allocate(Pool* pool){
    
    if(!pool->active || !pool->free_head){
        
        u64 bytes_required = default_pool_block_size;
        
        if(pool->chunk_size > bytes_required){
            bytes_required = pool->chunk_size;
        }
        
        Pool_Block* new_block = 0;
        
        new_block = (Pool_Block*)calloc(1, sizeof(Pool_Block) + bytes_required);
        assert(new_block);
        new_block->memory = (u8*)new_block + sizeof(Pool_Block);
        new_block->size = bytes_required;
        new_block->next = nullptr;
        
        if(pool->active){
            pool->active->next = new_block;
            pool->active = new_block;
        }else{
            pool->first = new_block;
            pool->active = new_block;
        }
        
        //active->clear(chunk_size);
        for(int i = 0; i < pool->active->size/pool->chunk_size; i++){
            void* pointer = &pool->active->memory[i * pool->chunk_size];
            Pool_Node* node = reinterpret_cast<Pool_Node*>(pointer);
            node->next = pool->free_head;
            pool->free_head = node;
        }
    }
    
    void* memory = pool->free_head;
    pool->free_head = pool->free_head->next;
    
    return memory;
}

void pool_free(Pool* pool){
    for(auto block = pool->first; block; block = block->next){
        free(block);
    }
}

internal Pool
make_pool(u64 size) {
    Pool result;
    result.chunk_size = size;
    result.first = nullptr;
    result.active = nullptr;
    result.free_head = nullptr;
    
    return result;
}


