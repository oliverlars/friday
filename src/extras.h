#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <inttypes.h>
#include <float.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>


#define global static
#define internal static
#define local_persist static
#define ArrayCount(x) (sizeof(x) / sizeof((x)[0]))

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

u32 random_state = 0;

internal u32
random() {
    u32 x = random_state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return random_state = x;
}

internal void
seed_from_time(){
    random_state = time(0);
}

internal void
seed(u32 value){
    random_state = value;
}

internal f32
randomf(f32 low, f32 high){
    return low + (high - low) * ((rand() % 10000) / 10000.0f);
}

#undef Assert
#define AssertStatement HardAssert
#define Assert HardAssert
#define HardAssert(b) do { if(!(b)) { _AssertFailure(#b, __LINE__, __FILE__, 1); } } while(0)
#define SoftAssert(b) do { if(!(b)) { _AssertFailure(#b, __LINE__, __FILE__, 0); } } while(0)
#define BreakDebugger() _DebugBreak_Internal_()
#define Log(...)         _DebugLog(0,           __FILE__, __LINE__, __VA_ARGS__)
#define LogWarning(...)  _DebugLog(Log_Warning, __FILE__, __LINE__, __VA_ARGS__)
#define LogError(...)    _DebugLog(Log_Error,   __FILE__, __LINE__, __VA_ARGS__)

#define Log_Warning (1<<0)
#define Log_Error   (1<<1)

void _AssertFailure(char *expression, int line, char *file, int crash);
void _DebugLog(s32 flags, char *file, int line, char *format, ...);
void _DebugBreak_Internal_(void);
void _BeginTimer(char *file, int line, char *format, ...);
void _EndTimer(void);