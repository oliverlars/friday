#define ARENA_MAX          Gigabytes(1)
#define ARENA_COMMIT_SIZE  Kilobytes(4)

internal Arena
make_arena() {
    Arena arena = {0};
    arena.size = ARENA_MAX;
    arena.base = platform->reserve(arena.size);
    assert(arena.base);
    arena.alloc_position = 0;
    arena.commit_position = 0;
    return arena;
}

#define push_type(arena, type) (type*)_arena_allocate(arena, sizeof(type))
#define push_type_zero(arena, type) (type*)_arena_allocate_zero(arena, sizeof(type))
#define push_size(arena, size) _arena_allocate(arena, size)
#define push_size_zero(arena, size) _arena_allocate_zero(arena, size)

internal void *
_arena_allocate(Arena *arena, u64 size)
{
    void *memory = 0;
    if(arena->alloc_position + size > arena->commit_position)
    {
        u64 commit_size = size;
        commit_size += ARENA_COMMIT_SIZE-1;
        commit_size -= commit_size % ARENA_COMMIT_SIZE;
        platform->commit((u8 *)arena->base + arena->commit_position, commit_size);
        arena->commit_position += commit_size;
    }
    memory = (u8 *)arena->base + arena->alloc_position;
    arena->alloc_position += size;
    
    return memory;
}

internal void *
_arena_allocate_zero(Arena *arena, u64 size)
{
    void *memory = push_size(arena, size);
    memset(memory, 0, size);
    return memory;
}

internal void
arena_pop(Arena *arena, u64 size)
{
    if(size > arena->alloc_position)
    {
        size = arena->alloc_position;
    }
    arena->alloc_position -= size;
}

internal void
arena_clear(Arena *arena)
{
    arena_pop(arena, arena->alloc_position);
}

internal void
arena_free(Arena* arena) {
    platform->release(arena->base);
}

internal Arena
subdivide_arena(Arena* arena, u64 size){
    Arena result = {};
    result.base = push_size_zero(arena, size);
    result.size = size;
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


