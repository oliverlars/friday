
#define internal static
#define global static
#define assert assert

#include <inttypes.h>
#include <float.h>
#include <assert.h>
#include <string.h>
#include <memory.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s32 b32;
typedef s32 b32x;

typedef float f32;
typedef double f64;

const u32 U32Max_ = ((u32)-1);
const f32 F32Max_ = FLT_MAX;
const f32 INF_ = FLT_MAX;
const f32 F32Min_ = -FLT_MAX;
const f32 Pi32_ = 3.1415926535897f;
const f32 Tau32_ = 6.283185307179f;

#define U32Max U32Max_
#define F32Max F32Max_
#define INF INF_
#define F32Min F32Min_
#define Pi32 Pi32_
#define Tau32 Tau32_

#define auto auto

#define Kilobytes(x) (1024*x)
#define Megabytes(x) (1024*Kilobytes(x))
#define Gigabytes(x) (1024*Megabytes(x))

// NOTE(Oliver): this array will remain contiguous and stable for upto whatever
// the reserved amount is, which will be more than reasonable for most
// uses, but if stable pointers are required then a chunked list is better
template<typename T>
struct Array {
    T* data = nullptr;
    
    union {
        // NOTE(Oliver): the spec says this will be initialised to 0
        // should just be able to rely on constructor, but it's helpful
        // i guess?
        u64 used;
        u64 size;
    };
    u64 capacity = 0;
    
    // TODO(Oliver): maybe a megabyte is unnecessary? probs more than i'll need it for
    void reserve(u64 amount_to_reserve){
        
        // NOTE(Oliver): reserve  a megabyte of address space should give us a cap
        // of 250 million arrays or something
        capacity = amount_to_reserve/sizeof(T);
        data = (T*)VirtualAlloc(0, amount_to_reserve, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    }
    
    Array() { used = 0;}
    
    ~Array(){
        VirtualFree(data, 0, MEM_RELEASE);
    }
    
    inline void reset(){
        used = 0;
    }
    
    void insert(T item){
        OPTICK_EVENT();
        if(!data){
            // NOTE(Oliver): reserve  a megabyte of address space should give us a cap
            // of 250 million arrays or something
            u64 amount_to_reserve = Megabytes(1);
            capacity = amount_to_reserve/sizeof(T);
            data = (T*)VirtualAlloc(0, amount_to_reserve, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        }else if(used + 1 >= capacity){
            T* temp_data = (T*)VirtualAlloc(0, capacity*2*sizeof(T), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            for(int i = 0; i < used; i++){
                temp_data[i] = data[i];
            }
            VirtualFree(data, 0, MEM_RELEASE);
            data = temp_data;
            capacity = capacity*2;
            // NOTE(Oliver): we don't really want to end up in here
            // we lose pointer stability and it's just a pain
        }
        data[used++] = item;
    }
    
    T &operator[] (u64 index) {
        assert(index >= 0 && index < used);
        return data[index];
    }
    
    inline T* first() {
        return &data[0];
    }
    
    inline T* last() {
        return &data[used];
    }
    
    inline T pop(){
        T last_element = last();
        used--;
        return last_element;
    }
    
    inline T bytes_used() {
        return used*sizeof(T);
    }
};

bool is_power_of_two(u64 value) {
	return (value & (value-1)) == 0;
}

struct Memory_Pool {
    
    struct Chunk {
        void* data = nullptr;
        s64 used = 0;
        s64 capacity = 0;
    };
    
    const u64 DEFAULT_CHUNK_SIZE = 4096 * 4;
    Array<Chunk> chunks;
    
    ~Memory_Pool() {
        reset();
    }
    
    void* allocate(u64 amount) {
        OPTICK_EVENT();
        
        // TODO(Oliver): we should align this
        
        for (int i = 0; i < chunks.size; i++) {
            Chunk* chunk = &chunks[i];
            auto available = chunk->capacity - chunk->used;
            assert(available >= 0);
            
            if (available >= amount) {
                void* result = (char*)chunk->data + chunk->used;
                chunk->used += amount;
                
                return result;
            }
        }
        
        // All chunks exhauted or amount simply doesnt fit..
        auto ammount_to_allocate = DEFAULT_CHUNK_SIZE;
        if (ammount_to_allocate < amount) ammount_to_allocate = amount;
        OutputDebugStringA("ALLOC\n");
        
        Chunk chunk;
        chunk.data = malloc(ammount_to_allocate);
        chunk.used = amount;
        chunk.capacity = ammount_to_allocate;
        chunks.insert(chunk);
        
        return chunk.data;
    }
    
    void reset() {
        OPTICK_EVENT();
        Chunk* chunk = chunks.data;
        for (int i = 0; i < chunks.size; i++) {
            chunk->used = 0;
            chunk++;
        }
        
    }
    
};

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
    
    Arena() {}
    
    ~Arena() {
        
    }
    
    void clear(){
        for(auto block = first; block; block = block->next){
            free(block);
        }
        first = nullptr;
        active = nullptr;
    }
    // NOTE(Oliver): this is to reuse allocations
    void reset(){
        for(auto block = first; block; block = block->next){
            block->used = 0;
        }
        active = first;
    }
    
    
};

internal void* 
arena_allocate(Arena* arena, u64 size){
    if(!arena->active  ||
       arena->active->used + size > arena->active->size){
        
        u64 bytes_required = default_memory_block_size;
        
        if(size > bytes_required){
            bytes_required = size;
        }
        
        Arena_Block* new_block = 0;
        
        new_block = (Arena_Block*)malloc(sizeof(Arena_Block) + bytes_required);
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
reset_arena(Arena* arena){
    
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
    
    Pool(u64 size=64) {chunk_size = size;}
    u64 chunk_size;
    
    void* allocate(){
        
        if(!active || !free_head){
            
            u64 bytes_required = default_pool_block_size;
            
            if(chunk_size > bytes_required){
                bytes_required = chunk_size;
            }
            
            Pool_Block* new_block = 0;
            
            new_block = (Pool_Block*)malloc(sizeof(Pool_Block) + bytes_required);
            assert(new_block);
            new_block->memory = (u8*)new_block + sizeof(Pool_Block);
            new_block->size = bytes_required;
            new_block->next = nullptr;
            
            if(active){
                active->next = new_block;
                active = new_block;
            }else{
                first = new_block;
                active = new_block;
            }
            
            //active->clear(chunk_size);
            for(int i = 0; i < active->size/chunk_size; i++){
                void* pointer = &active->memory[i * chunk_size];
                Pool_Node* node = reinterpret_cast<Pool_Node*>(pointer);
                node->next = free_head;
                free_head = node;
            }
        }
        
        void* memory = free_head;
        free_head = free_head->next;
        
        return memory;
    }
    
    void clear(void* pointer){
        bool is_valid_pointer = false;
        for(auto block = first; block; block = block->next){
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
        node->next = free_head;
        free_head = node;
    }
    
    void reset(){
        for(auto block = first; block; block = block->next){
            void* start = block->memory;
            void* end = &block->memory[block->size-1];
            
            //active->clear(chunk_size);
            for(int i = 0; i < active->size/chunk_size; i++){
                void* pointer = &block->memory[i * chunk_size];
                Pool_Node* node = reinterpret_cast<Pool_Node*>(pointer);
                node->next = free_head;
                free_head = node;
            }
            
        }
        
    }
    
    void clear_all(){
        for(auto block = first; block; block = block->next){
            free(block);
        }
    }
    
};



struct {
    
    u32 width;
    u32 height;
    
    Arena permanent_arena;
    Arena temporary_arena;
} platform;
