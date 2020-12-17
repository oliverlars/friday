
enum Key_Enum {
#define Key(name, str) KEY_##name,
#include "key_list.h"
#undef Key
    KEY_MAX
};
typedef u64 Key;


enum Key_Modifiers_Enum {
    KEY_MOD_CTRL = (1 << 0),
    KEY_MOD_SHIFT = (1 << 1),
    KEY_MOD_ALT = (1 << 2),
};
typedef u64 Key_Modifiers;

internal char *
get_key_name(s32 key) {
    local_persist char* key_names[KEY_MAX] = {
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

enum Mouse_Button {
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT,
};


enum Platform_Event_Type {
    PLATFORM_EVENT_INVALID,
    
    PLATFORM_EVENT_KEY_BEGIN,
    PLATFORM_EVENT_CHARACTER_INPUT,
    PLATFORM_EVENT_KEY_PRESS,
    PLATFORM_EVENT_KEY_RELEASE,
    PLATFORM_EVENT_KEY_END,
    
    PLATFORM_EVENT_MOUSE_BEGIN,
    PLATFORM_EVENT_MOUSE_PRESS,
    PLATFORM_EVENT_MOUSE_RELEASE,
    PLATFORM_EVENT_MOUSE_MOVE,
    PLATFORM_EVENT_MOUSE_SCROLL,
    PLATFORM_EVENT_MOUSE_END,
    
    PLATFORM_EVENT_COUNT,
};

struct Platform_Event {
    Platform_Event_Type type;
    Key key;
    
    Mouse_Button mouse_button;
    Key_Modifiers modifiers;
    u64 character;
    
    v2f position;
    v2f delta;
    v2f scroll;
};

struct Platform {
    
    
    Arena permanent_arena;
    Arena frame_arena;
    
    void* globals;
    
    String8 executable_folder_absolute_path;
    String8 executable_absolute_path;
    String8 working_directory_path;
    
    volatile b32 quit;
    b32 vsync;
    b32 fullscreen;
    v2i window_size;
    f32 current_time;
    f32 target_fps;
    b32 wait_for_events_to_update;
    b32 pump_events;
    
    
    v2f mouse_position;
    u64 event_count;
    Platform_Event events[4096];
    
    f32* sample;
    u32 sample_count;
    u32 samples_per_second;
    
    void* (*heap_alloc)(u32 size);
    void* (*heap_free)(void* data);
    void* (*reserve)(u64 size);
    void (*release)(void* memory);
    void (*commit)(void* memory, u64 size);
    void (*uncommit)(void* memory, u64 size);
    void (*output_error)(char *error_type, char *error_format, ...);
    void (*save_to_file)(String8 path, void* data, u64 length);
    void (*append_to_file)(String8 path, void* data, u64 length);
    void (*load_file)(Arena* arena, String8 path, void** data, u64* length);
    char* (*load_file_and_null_terminate)(Arena* aerna, String8 path);
    void (*delete_file)(String8 path);
    b32 (*make_directory)(String8 path);
    b32 (*does_file_exist)(String8 path);
    b32 (*copy_file)(String8 dest, String8 source);
    
    f32 (*get_time)(void);
    u64 (*get_cycles)(void);
    void (*reset_cursor)(void);
    void (*set_cursor_to_horizontal_resize)(void);
    void (*set_cursor_to_vertical_resize)(void);
    void (*set_cursor_to_ibar)(void);
    void (*refresh_screen)(void);
    void* (*load_opengl_procedure)(char *name);
};


global Platform* platform = 0;

#define PERMANENT_LOAD __declspec(dllexport) void permanent_load(Platform* platform_)
typedef void Permanent_Load_Callback(Platform*);
internal void permanent_load_stub(Platform* _) {}

#define HOT_LOAD __declspec(dllexport) void hot_load(Platform* platform_)
typedef void Hot_Load_Callback(Platform*);
internal void hot_load_stub(Platform* _) {}

#define HOT_UNLOAD __declspec(dllexport) void hot_unload(void)
typedef void Hot_Unload_Callback(void);
internal void hot_unload_stub(void) {}

#define UPDATE __declspec(dllexport) void update(void)
typedef void Update_Callback(void);
internal void update_stub(void) {}

#define BEGIN_C_EXPORT extern "C" {
#define END_C_EXPORT }