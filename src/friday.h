
#define internal static
#define global static
#define assert assert

#include <inttypes.h>
#include <float.h>
#include <assert.h>
#include <string.h>

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
    u64 used = 0;
    u64 capacity = 0;
    
    // TODO(Oliver): maybe a megabyte is unnecessary? probs more than i'll need it for
    void reserve(u64 amount_to_reserve){
        
        // NOTE(Oliver): reserve  a megabyte of address space should give us a cap
        // of 250 million arrays or something
        capacity = amount_to_reserve/sizeof(T);
        data = (T*)VirtualAlloc(0, amount_to_reserve, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        
    }
    Array() {}
    ~Array(){
        VirtualFree(data, 0, MEM_RELEASE);
    }
    
    void insert(T item){
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
        return &data[count];
    }
    
    inline T pop(){
        T last_element = last();
        count--;
        return last_element;
    }
};

template<typename T>
struct Chunk {
    T data;
};

struct Chunked_List {
    Chunked_List(u64 the_chunk_size){
        chunk_size = the_chunk_size;
    }
    
    u64 chunk_size;
    
    Chunked_List* next = nullptr;
    Chunked_List* previous = nullptr;
    Chunked_List** end = nullptr;
};