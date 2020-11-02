
struct Bitmap {
    int width, height;
    u8* data;
    int channel_count;
    GLuint texture;
};

struct Character { 
    
    int x;
    int y;
    int width;
    int height;
    
    int x_offset;
    int y_offset;
    int x_advance;
};

struct SDFFont {
    Character characters[256];
    Bitmap bitmap;
    int line_height;
    int size;
    f32 scale;
    v4i padding;
};

enum Menu_Type {
    MENU_TYPE_USAGE,
    MENU_CREATE_NODE
};

enum Command_Type {
    COMMAND_RECTANGLE,
    COMMAND_TRIANGLE,
    COMMAND_CIRCLE,
    COMMAND_RECTANGLE_OUTLINE,
    COMMAND_GLYPH,
    COMMAND_RECTANGLE_TEXTURED,
    
    COMMAND_CLIP_RANGE_BEGIN,
    COMMAND_CLIP_RANGE_END,
    
    COMMAND_COUNT
};

struct Command {
    Command_Type type;
    union {
        struct {
            u8 r, g, b, a;
        };
        u32 packed;
    } colour;
    Command* previous = nullptr;
    Command* next = nullptr;
    
    union{
        
        struct {
#define BYTES_PER_RECTANGLE (9*sizeof(f32)) 
            // NOTE(Oliver): bytes include size of colour, which is 4*sizeof(f32)
            f32 x, y;
            f32 width, height;
            f32 corner_radius;
        }rectangle;
        
        struct {
#define BYTES_PER_TRIANGLE (8*sizeof(f32)) // NOTE(Oliver): we secretly add two size fields
            f32 x, y;
            f32 size;
        }triangle;
        
        struct {
#define BYTES_PER_CIRCLE (7*sizeof(f32))
            f32 x, y;
            f32 radius;
        }circle;
        
        struct {
#define BYTES_PER_RECTANGLE_OUTLINE (10*sizeof(f32))
            f32 x, y;
            f32 width, height;
            f32 border_size;
            f32 corner_radius;
        }rectangle_outline;
        
        struct {
#define BYTES_PER_GLYPH (12*sizeof(f32))
            f32 x, y;
            f32 width, height;
            f32 u, v;
            f32 u_width, v_height; 
        } glyph;
        
        struct {
#define BYTES_PER_RECTANGLE_TEXTURED (9*sizeof(f32))
            f32 x, y;
            f32 width, height;
            f32 corner_radius;
            f32 u, v;
            f32 u_width, v_height;
            
            Bitmap bitmap;
        }rectangle_textured;
        
        struct {
            f32 x, y;
            f32 width, height;
        } clip_range;
    };
};

const int MAX_DRAW = 8192;

struct Renderer {
    
    GLuint vaos[COMMAND_COUNT];
    GLuint buffers[COMMAND_COUNT];
    GLuint programs[COMMAND_COUNT];
    
    // TODO(Oliver): remove these, not needed anymore
    GLuint ortho_uniform;
    GLuint view_uniform;
    
    GLuint resolution_uniforms[COMMAND_COUNT];
    GLuint clip_range_uniforms[COMMAND_COUNT];
    
    GLuint texture;
    
    Command* head = nullptr;
    Command* tail = nullptr;
    
    SDFFont font;
    
    Arena frame_arena;
};

global Renderer* renderer = 0;