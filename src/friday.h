
#define internal static
#define global static
#define local_persist static
#define assert assert

#include <inttypes.h>
#include <float.h>
#include <assert.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <stdarg.h>

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

const u64 U64Max_ = ((u64)-1);
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
#define U64Max U64Max_

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
    
    T& operator[] (u64 index) {
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
};

struct Temp_Arena {
    Temp_Arena(Arena _arena) { arena = _arena; };
    ~Temp_Arena() {}
    Arena arena;
};

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

struct String8 {
    char* text;
    u64 length;
    
    // NOTE(Oliver): we may want to append to strings
    u64 capacity;
    
    char operator[](u64 index){
        assert(index >= 0 && index < length);
        return text[index];
    }
};

internal String8
make_string(Arena* arena, char* string, u64 capacity = 256){
    
    char* pointer = string;
    while(pointer && *pointer){
        pointer++;
    }
    u64 length = pointer - string;
    
    char* text = (char*)arena_allocate(arena, capacity);
    for(int i = 0; i < length; i++){
        text[i] = string[i];
    }
    String8 result;
    result.text = text;
    result.length = length;
    result.capacity = capacity;
    return result;
}

internal String8
make_string(u8* backing, char* string, u64 capacity){
    
    char* pointer = string;
    while(pointer && *pointer){
        pointer++;
    }
    u64 length = pointer - string;
    
    char* text = (char*)backing;
    for(int i = 0; i < length; i++){
        text[i] = string[i];
    }
    String8 result;
    result.text = text;
    result.length = length;
    result.capacity = capacity;
    return result;
}

internal String8
copy_string(Arena* arena, String8 to_copy){
    char* buffer = (char*)arena_allocate(arena, to_copy.length);
    String8 result;
    result.length = to_copy.length;
    result.text = buffer;
    for(int i = 0; i < to_copy.length; i++){
        result.text[i] = to_copy.text[i];
    }
    return result;
}

internal char*
to_cstring(Arena* arena, String8 string){
    char* result = (char*)arena_allocate(arena, string.length+1);
    for(int i = 0; i < string.length; i++){
        result[i] = string.text[i];
    }
    result[string.length] = 0;
    return result;
}

internal String8
append_to_string(Arena* arena, String8 string, char* appendee){
    String8 result = make_string(arena, "result");
    for(int i = 0; i < string.length; i++){
        result.text[i] = string.text[i];
    }
    for(int i = 0; i < strlen(appendee); i++){
        result.text[i+string.length] = appendee[i];
    }
    result.length = string.length + strlen(appendee);
    return result;
}

internal String8
prepend_to_string(Arena* arena, char* prependee, String8 string){
    String8 result = make_string(arena, "result");
    int prependee_length = strlen(prependee);
    for(int i = 0; i < string.length; i++){
        result.text[i+prependee_length] = string.text[i];
    }
    for(int i = 0; i < prependee_length; i++){
        result.text[i] = prependee[i];
    }
    result.length = prependee_length + string.length;
    return result;
}

internal bool
string_eq(String8 a, char* b){
    if(!a.text || !a.length) return false;
    if(!b || !(*b)) return false;
    int b_length = strlen(b);
    int min_length = a.length > b_length ? b_length : a.length;
    
    for(int i = 0; i < min_length; i++){
        if(a[i] != b[i]){
            return false;
        }
    }
    
    return true;
}

internal int
string_to_int(String8 string){
    
    int result = 0;
    bool is_negative = string.text[0] == '-';
    for(int i = is_negative; i < string.length; i++){
        result = result*10 + (string.text[i] - '0');
    }
    
    if(is_negative){
        result *= -1;
    }
    return result;
}

internal void
insert_in_string(String8* string, char* insertable, u64 index){
    if(!insertable) return;
    int length = strlen(insertable);
    for(int i = string->capacity-1; i > index; i--){
        string->text[i] = string->text[i-length];
    }
    for(int i = index; i < index+length; i++){
        string->text[index] = *insertable++;
    }
    string->length += length;
}

internal void
pop_from_string(String8* string, u64 index){
    if(string->length == 0) return;
    for(int i = index; i < string->length; i++){
        string->text[i-1] = string->text[i];
    }
    string->length--;
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
    
    void clear_all(){
        for(auto block = first; block; block = block->next){
            free(block);
        }
    }
    
};

internal Pool
make_pool(u64 size) {
    Pool result;
    result.chunk_size = size;
    
    result.first = nullptr;
    result.active = nullptr;
    result.free_head = nullptr;
    
    return result;
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

struct {
    
    u32 width;
    u32 height;
    
    Arena permanent_arena;
    Arena temporary_arena;
    
    f32 mouse_x, mouse_y;
    
    bool mouse_left_clicked;
    
    bool mouse_left_up;
    bool mouse_left_down;
    
    f32 mouse_scroll_delta;
    f32 mouse_scroll_source;
    f32 mouse_scroll_target;
    
    bool mouse_middle_up;
    bool mouse_middle_down;
    
    bool mouse_middle_clicked;
    bool mouse_right_clicked;
    bool mouse_left_double_clicked;
    
    bool mouse_drag;
    f32 mouse_drag_x;
    f32 mouse_drag_y;
    
    bool keys_pressed[4096];
    bool keys_down[4096];
    
    bool has_text_input;
    char* text_input;
    
    
    u64 tick;
    
    f32 delta_time;
    
} platform;


enum Key {
#define Key(name, str) KEY_##name,
#include "key_list.h"
#undef Key
    KEY_MAX
};

internal char *
get_key_name(s32 key) {
    static char* key_names[KEY_MAX] = {
#define Key(name, str) str,
#include "key_list.h"
#undef Key
    };
    char *key_name = "(Invalid Key)";
    if(key >= 0 && key < KEY_MAX)
    {
        key_name = key_names[key];
    }
    return key_name;
}

internal f32
clampf(f32 value, f32 min, f32 max){
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

internal void
debug_print(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    char output[256];
    int size = vsnprintf(output, 256, fmt, args);
    output[size] = '\n';
    output[size+1] = 0;
    OutputDebugStringA(output);
}

struct Key_State {
    bool was_down;
    int half_transition_count;
};

#define INPUT_COUNT 64
union Input {
    Key_State actions[INPUT_COUNT];
    struct{
        Key_State navigate_left;
        Key_State navigate_right;
        Key_State navigate_up;
        Key_State navigate_down;
        
        Key_State enter_text_edit_mode;
        Key_State enter_command_mode;
        Key_State enter_make_mode;
        
        Key_State editor_zoom;
        
        Key_State backspace;
        Key_State add_pointers;
        Key_State make_arg;
        Key_State make_decl;
        Key_State make_func;
        Key_State make_scope;
        Key_State make_loop;
        Key_State make_cond;
    };
} input;


internal b32
was_pressed(Key_State state){
    bool result = ((state.half_transition_count > 1) || 
                   ((state.half_transition_count == 1) && (state.was_down)));
    
    return result;
}

internal b32
is_pressed(Key_State state){
    bool result = state.was_down;
    return result;
}

internal void
process_keyboard_event(Key_State* state, bool is_down){
    if(state->was_down != is_down){
        state->was_down = is_down;
        state->half_transition_count += 1;
    }
}

internal void
debug_print(String8 string){
    char output[256];
    output[0] = '%';
    output[1] = '.';
    output[0] = '*';
    output[0] = 's';
    int size = snprintf(output, string.length, string.text);
    output[size-1] = '\n';
    output[size] = 0;
    OutputDebugStringA(output);
}

#define defer_loop(begin, end) for(int _i_ = (begin, 0); !_i_; ++_i_, end)