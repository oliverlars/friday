
// NOTE(Oliver): this is global state for the render pass
// may not be needed, we'll see

struct Bitmap {
    int width, height;
    u8* data;
    int channel_count;
    GLuint texture;
};

internal Bitmap 
make_bitmap(char* filename){
    int x, y, n;
    u8* data = stbi_load(filename, &x, &y, &n, 0);
    
    Bitmap result;
    result.data = data;
    result.width = x;
    result.height = y;
    result.channel_count = n;
    
    glGenTextures(1, &result.texture);
    glBindTexture(GL_TEXTURE_2D, result.texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result.width,
                 result.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 result.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return result;
}

internal Bitmap 
make_bitmap(String8 filename){
    int x, y, n;
    char filename_cstr[512];
    for(int i = 0; i < filename.length; i++){
        filename_cstr[i] = filename.text[i];
    }
    filename_cstr[filename.length] = 0;
    return make_bitmap(filename_cstr);
}

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

struct Lexer {
    char* pos;
};

internal void
advance_lexer(Lexer* l){
    l->pos++;
}

#define is_newline(x) ((x) == '\n' || (x) == '\r')
#define is_whitespace(x) ((x) == ' ' || (x) == '\t' || (x) == '\v' || (x) == '\f' || is_newline(x))
#define is_digit(x) ((x) <= '9' && (x) >= '0')
#define is_upper_hex(x) (is_digit(x) || ((x) <= 'F' && (x) >= 'A'))
#define is_lower_hex(x) (is_digit(x) || ((x) <= 'f' && (x) >= 'a'))
#define is_hex(x) (is_upper(x)  || is_lower_hex(x))
#define is_lower_alpha(x) ((x) <= 'z' && (x) >= 'a')
#define is_upper_alpha(x) ((x) <= 'Z' && (x) >= 'A')
#define is_alpha(x) ((is_lower_alpha(x) || is_upper_alpha(x)))

internal void
gobble_whitespace(Lexer* l){
    while(is_whitespace(*l->pos)){
        advance_lexer(l);
    }
}

internal String8
read_token(Lexer* l){
    gobble_whitespace(l);
    
    char c = *l->pos;
    String8 token = {};
    token.text = l->pos;
    token.length = 1;
    advance_lexer(l);
    switch(c){
        case '\0':{}break;
        case ':':{} break;
        case ',':{} break;
        case '=':{} break;
        case '\"':{
            while(*l->pos != '\"'){
                advance_lexer(l);
            }
            token.text++;
            token.length = l->pos - token.text;
            advance_lexer(l);
        } break;
        
        default:{
            if(is_alpha(c)){
                while(is_alpha(*l->pos)){advance_lexer(l); }
                token.length = l->pos - token.text;
            }else if(is_digit(c)){
                while(is_digit(*l->pos)){ advance_lexer(l); }
                token.length = l->pos - token.text;
            }else if(c == '-'){
                while(is_digit(*l->pos)){ advance_lexer(l); }
                token.length = l->pos - token.text;
            }
        }break;
    }
    return token;
}

internal void
expect_token(Lexer* l, char* string){
    assert(string_eq(read_token(l), string));
}

internal SDFFont
load_sdf_font(char* filename){
    FILE* file = fopen(filename, "r");
    
    char* buffer = 0;
    if(file){
        fseek(file, 0, SEEK_END);
        u64 size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        buffer = (char*)calloc(size+2, sizeof(char));
        buffer[size] = 0;
        buffer[size-1] = '\n';
        fread(buffer, size, 1, file);
        fclose(file);
    }
    
    SDFFont font = {};
    Lexer l = {buffer};
    
    while(!string_eq(read_token(&l), "size")){
        
    }
    expect_token(&l, "=");
    int size = string_to_int(read_token(&l));
    
    while(!string_eq(read_token(&l), "padding")){
        
    }
    expect_token(&l, "=");
    v4i padding = {};
    padding.x = string_to_int(read_token(&l));
    expect_token(&l, ",");
    padding.y = string_to_int(read_token(&l));
    expect_token(&l, ",");
    padding.z = string_to_int(read_token(&l));
    expect_token(&l, ",");
    padding.w = string_to_int(read_token(&l));
    
    
    while(!string_eq(read_token(&l), "lineHeight")){
        
    }
    expect_token(&l, "=");
    int line_height = string_to_int(read_token(&l));
    
    while(!string_eq(read_token(&l), "file")){
    }
    
    expect_token(&l, "=");
    
    {
        Temp_Arena temp_arena(platform.permanent_arena);
        String8 filename = prepend_to_string(&temp_arena.arena, "../fonts/", read_token(&l));
        font.bitmap = make_bitmap(filename);
    }
    
    assert(font.bitmap.data);
    expect_token(&l, "chars");
    expect_token(&l, "count");
    expect_token(&l, "=");
    read_token(&l);
    
    while(string_eq(read_token(&l), "char")){
        expect_token(&l, "id");
        expect_token(&l, "=");
        int id = string_to_int(read_token(&l));
        
        expect_token(&l, "x");
        expect_token(&l, "=");
        font.characters[id].x = string_to_int(read_token(&l));
        
        expect_token(&l, "y");
        expect_token(&l, "=");
        font.characters[id].y = string_to_int(read_token(&l));
        
        expect_token(&l, "width");
        expect_token(&l, "=");
        font.characters[id].width = string_to_int(read_token(&l));
        
        expect_token(&l, "height");
        expect_token(&l, "=");
        font.characters[id].height = string_to_int(read_token(&l));
        
        expect_token(&l, "xoffset");
        expect_token(&l, "=");
        font.characters[id].x_offset = string_to_int(read_token(&l));
        
        expect_token(&l, "yoffset");
        expect_token(&l, "=");
        font.characters[id].y_offset = string_to_int(read_token(&l));
        
        expect_token(&l, "xadvance");
        expect_token(&l, "=");
        font.characters[id].x_advance = string_to_int(read_token(&l));
        
        expect_token(&l, "page");
        expect_token(&l, "=");
        expect_token(&l, "0");
        
        expect_token(&l, "chnl");
        expect_token(&l, "=");
        expect_token(&l, "0");
        
        gobble_whitespace(&l);
        
    }
    
    font.size = size;
    assert(font.size == 55);
    font.line_height = line_height;
    font.scale = 20.0f/(f32)font.size;
    font.padding = padding;
    return font;
}

enum Menu_Type {
    MENU_TYPE_USAGE,
    MENU_CREATE_NODE
};

struct {
    int x;
    int y;
    int x_offset;
    int y_offset;
    int scroll_amount;
    int indent;
    
    int pan_offset_x;
    int pan_offset_y;
    
    int delta_x;
    int delta_y;
    
    int cursor_index;
    Node* active_node;
    
    Node* program_root;
    
    Node* test_node;
    
    Pool node_pool;
    
    f32 menu_x, menu_y;
    Node* menu_node;
    int selected;
    
    int minimap_x;
    int minimap_y;
    int minimap_x_offset;
    int minimap_y_offset;
    
    int LOD;
    
    
    f32 cursor_x;
    f32 cursor_y;
    
    f32 cursor_target_x;
    f32 cursor_target_y;
    
    
    Node* selected_node;
    
} friday;

enum Command_Type {
    COMMAND_RECTANGLE,
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

global struct {
    
    GLuint vaos[COMMAND_COUNT];
    GLuint buffers[COMMAND_COUNT];
    GLuint programs[COMMAND_COUNT];
    
    // TODO(Oliver): remove these, not needed anymore
    GLuint ortho_uniform;
    GLuint view_uniform;
    
    GLuint resolution_uniforms[COMMAND_COUNT];
    
    GLuint texture;
    
    Command* head = nullptr;
    Command* tail = nullptr;
    
    SDFFont font;
    
    Arena shape_attribs;
    Arena frame_arena;
    Arena temp_string_arena;
} renderer;


internal int
get_font_line_height(f32 font_scale = 1.0f) {
    font_scale *= renderer.font.scale;
    return renderer.font.line_height*font_scale;
}

internal Command*
make_command(Command_Type type){
    Command* command = (Command*)arena_allocate(&renderer.frame_arena, sizeof(Command));
    command->type = type;
    command->next = nullptr;
    command->previous = nullptr;
    return command;
}

internal void
insert_command(Command* next_command){
    
    if(!renderer.head){
        renderer.head = next_command;
        renderer.tail = renderer.head;
    }else {
        renderer.tail->next = next_command;
        renderer.tail = renderer.tail->next;
    }
}

internal inline GLuint
get_vao_rectangle() {
    return renderer.vaos[COMMAND_RECTANGLE];
}

internal inline GLuint
get_buffer_rectangle() {
    return renderer.buffers[COMMAND_RECTANGLE];
}

internal inline GLuint
get_program_rectangle() {
    return renderer.programs[COMMAND_RECTANGLE];
}

internal inline GLuint
get_vao_glyph() {
    return renderer.vaos[COMMAND_GLYPH];
}

internal inline GLuint
get_buffer_glyph() {
    return renderer.buffers[COMMAND_GLYPH];
}

internal inline GLuint
get_program_glyph() {
    return renderer.programs[COMMAND_GLYPH];
}

internal inline GLuint
get_vao_rectangle_outline() {
    return renderer.vaos[COMMAND_RECTANGLE_OUTLINE];
}

internal inline GLuint
get_buffer_rectangle_outline() {
    return renderer.buffers[COMMAND_RECTANGLE_OUTLINE];
}

internal inline GLuint
get_program_rectangle_outline() {
    return renderer.programs[COMMAND_RECTANGLE_OUTLINE];
}

internal inline GLuint
get_vao_circle() {
    return renderer.vaos[COMMAND_CIRCLE];
}

internal inline GLuint
get_buffer_circle() {
    return renderer.buffers[COMMAND_CIRCLE];
}

internal inline GLuint
get_program_circle() {
    return renderer.programs[COMMAND_CIRCLE];
}

internal inline GLuint
get_vao_rectangle_textured() {
    return renderer.vaos[COMMAND_RECTANGLE_TEXTURED];
}

internal inline GLuint
get_buffer_rectangle_textured() {
    return renderer.buffers[COMMAND_RECTANGLE_TEXTURED];
}

internal inline GLuint
get_program_rectangle_textured() {
    return renderer.programs[COMMAND_RECTANGLE_TEXTURED];
}

internal inline void
push_rectangle(f32 x, f32 y, f32 width, f32 height, f32 radius, u32 colour){
    
    auto rectangle = make_command(COMMAND_RECTANGLE);
    rectangle->rectangle.x = x;
    rectangle->rectangle.y = y;
    rectangle->rectangle.width = width;
    rectangle->rectangle.height = height;
    rectangle->rectangle.corner_radius = radius;
    rectangle->colour.packed = colour;
    insert_command(rectangle);
}

internal inline void
push_clip_range_begin(f32 x, f32 y, f32 width, f32 height){
    auto clip_range = make_command(COMMAND_CLIP_RANGE_BEGIN);
    clip_range->clip_range.x = x;
    clip_range->clip_range.y = y;
    clip_range->clip_range.width = width;
    clip_range->clip_range.height = height;
    insert_command(clip_range);
}

internal inline void
push_clip_range_end(){
    auto clip_range = make_command(COMMAND_CLIP_RANGE_END);
    insert_command(clip_range);
}

internal inline void
push_rectangle(v4f rect, f32 radius, u32 colour){
    
    auto rectangle = make_command(COMMAND_RECTANGLE);
    rectangle->rectangle.x = rect.x;
    rectangle->rectangle.y = rect.y;
    rectangle->rectangle.width = rect.width;
    rectangle->rectangle.height = rect.height;
    rectangle->rectangle.corner_radius = radius;
    rectangle->colour.packed = colour;
    insert_command(rectangle);
}

internal inline void
push_rectangle_outline(f32 x, f32 y, f32 width, f32 height, f32 border, f32 radius, u32 colour = 0xFF00FFFF){
    
    auto rectangle = make_command(COMMAND_RECTANGLE_OUTLINE);
    rectangle->rectangle_outline.x = x;
    rectangle->rectangle_outline.y = y;
    rectangle->rectangle_outline.width = width;
    rectangle->rectangle_outline.height = height;
    rectangle->rectangle_outline.border_size = border;
    rectangle->rectangle_outline.corner_radius = radius;
    rectangle->colour.packed = colour;
    insert_command(rectangle);
}

internal inline void
push_circle(f32 x, f32 y, f32 radius, u32 colour = 0xFF00FFFF){
    
    auto circle = make_command(COMMAND_CIRCLE);
    circle->circle.x = x;
    circle->circle.y = y;
    circle->circle.radius = radius;
    circle->colour.packed = colour;
    insert_command(circle);
}


internal void
push_glyph(v4f positions, v4f uvs, u32 colour){
    
    f32 x0, x1, y0, y1, s0, s1, t0, t1;
    x0 = positions.x0;
    x1 = positions.x1;
    y0 = -positions.y0;
    y1 = -positions.y1;
    
    s0 = uvs.x0; s1 = uvs.x1;
    t0 = uvs.y0; t1 = uvs.y1;
    
    auto glyph = make_command(COMMAND_GLYPH);
    glyph->glyph.x = x0;
    glyph->glyph.y = y0;
    glyph->glyph.width = x1 - x0;
    glyph->glyph.height = y1 - y0;
    glyph->glyph.u = s0;
    glyph->glyph.v = t0;
    glyph->glyph.u_width = s1 - s0;
    glyph->glyph.v_height = t1 - t0;
    glyph->colour.packed = colour;
    insert_command(glyph);
}

internal inline void
push_rectangle_textured(f32 x, f32 y, f32 width, f32 height, f32 radius, Bitmap bitmap){
    
    auto rectangle = make_command(COMMAND_RECTANGLE_TEXTURED);
    rectangle->rectangle_textured.x = x;
    rectangle->rectangle_textured.y = y;
    rectangle->rectangle_textured.width = width;
    rectangle->rectangle_textured.height = height;
    rectangle->rectangle_textured.corner_radius = radius;
    rectangle->rectangle_textured.u = 0;
    rectangle->rectangle_textured.v = 0;
    rectangle->rectangle_textured.u_width = width;
    rectangle->rectangle_textured.v_height = height;
    rectangle->rectangle_textured.bitmap = bitmap;
    insert_command(rectangle);
}

internal void
push_string(f32 x, f32 y, char* text, u32 colour, f32 font_scale = 1.0f){
    
    y = -y;
    y -= get_font_line_height(font_scale);
    font_scale *= renderer.font.scale;
    // NOTE(Oliver): '#' is used for ID purposes
    while(*text && *text != '#'){
        if(*text >= 32 && *text < 128){
            auto font = renderer.font;
            auto c  = font.characters[*text];
            v4f positions = v4f(x + c.x_offset*font_scale, 
                                y + c.y_offset*font_scale, 
                                x + c.x_offset*font_scale + c.width*font_scale,
                                y + c.y_offset*font_scale + c.height*font_scale);
            v4f uvs = v4f(c.x/512.0f, c.y/512.0f, (c.x + c.width)/512.0f, (c.y+c.height)/512.0f);
            push_glyph(positions, uvs, colour);
            x += (c.x_advance-font.padding.x)*font_scale;
        }
        text++;
    }
}

internal void
push_string8(f32 x, f32 y, String8 string, u32 colour, f32 font_scale = 1.0f){
    
    y = -y;
    y -= get_font_line_height(font_scale);
    font_scale *= renderer.font.scale;
    
    // NOTE(Oliver): '#' is used for ID purposes
    for(int i = 0; i < string.length; i++){
        char text = string[i];
        if(text == '#'){break;}
        //while(string.text && *string.text && *string.text != '#'){
        if(text >= 32 && text < 128){
            auto font = renderer.font;
            auto c  = font.characters[text];
            v4f positions = v4f(x + c.x_offset*font_scale, 
                                y + c.y_offset*font_scale, 
                                x + c.x_offset*font_scale + c.width*font_scale,
                                y + c.y_offset*font_scale + c.height*font_scale);
            v4f uvs = v4f(c.x/512.0f, c.y/512.0f, (c.x + c.width)/512.0f, (c.y+c.height)/512.0f);
            push_glyph(positions, uvs, colour);
            x += (c.x_advance - font.padding.x)*font_scale;
        }
    }
}

internal f32
get_text_width(char* text, f32 font_scale = 1.0f){
    font_scale *= renderer.font.scale;
    f32 result = 0;
    while(text && *text){
        if(*text == '#') break;
        int id = *text;
        result += (renderer.font.characters[id].x_advance-renderer.font.padding.x)*font_scale;
        text++;
    }
    return result;
}

internal f32
get_text_width(String8 string, f32 font_scale = 1.0f){
    font_scale *= renderer.font.scale;
    f32 result = 0;
    for(int i = 0; i < string.length; i++){
        int id = string[i];
        result += (renderer.font.characters[id].x_advance-renderer.font.padding.x)*font_scale;
    }
    return result;
}

internal f32
get_text_width_n(char* text, int n, f32 font_scale = 1.0f){
    font_scale *= renderer.font.scale;
    f32 result = 0;
    int i = 0;
    while(text && *text && i < n){
        if(*text == '#') break;
        int id = *text;
        result += (renderer.font.characters[id].x_advance-renderer.font.padding.x)*font_scale;
        text++;
        i++;
    }
    return result;
}

internal f32
get_text_width_n(String8 string, int n, f32 font_scale = 1.0f){
    font_scale *= renderer.font.scale;
    f32 result = 0;
    for(int i = 0; (i < string.length) &&  i < n; i++){
        int id = string[i];
        result += (renderer.font.characters[id].x_advance-renderer.font.padding.x)*font_scale;
    }
    return result;
}

internal v4f
get_text_bbox(f32 x, f32 y, String8 string, f32 font_scale = 1.0f, f32 border = 5.0f){
    f32 width = get_text_width(string, font_scale) + border*2;
    f32 height = get_font_line_height(font_scale);
    x -= border;
    return v4f(x, y, width, height);
}

internal v4f
get_text_bbox(f32 x, f32 y, char* string, f32 font_scale = 1.0f, f32 border = 5.0f){
    f32 width = get_text_width(string, font_scale) + border*2;
    f32 height = get_font_line_height(font_scale);
    x -= border;
    return v4f(x, y, width, height);
}


internal void
init_opengl_renderer(){
    
    renderer.shape_attribs = 
        subdivide_arena(&platform.temporary_arena, MAX_DRAW*16);
    
    renderer.frame_arena = subdivide_arena(&platform.temporary_arena, 8192*4);
    
    renderer.temp_string_arena = subdivide_arena(&platform.temporary_arena, 4096);
    
    {
        glGenVertexArrays(1, &renderer.vaos[COMMAND_RECTANGLE]);
        glBindVertexArray(renderer.vaos[COMMAND_RECTANGLE]);
        
        glGenBuffers(1, &renderer.buffers[COMMAND_RECTANGLE]);
        glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[COMMAND_RECTANGLE]);
        
        // NOTE(Oliver): yeah maybe sort these constants out, looks wacko
        glBufferData(GL_ARRAY_BUFFER, MAX_DRAW*BYTES_PER_RECTANGLE, 0, GL_DYNAMIC_DRAW);
        
        GLuint pos = 0;
        GLuint dim = 2;
        GLuint radius = 3;
        GLuint colour = 4;
        
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, 2, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE, reinterpret_cast<void*>(0));
        
        glEnableVertexAttribArray(dim);
        glVertexAttribPointer(dim, 2, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE, reinterpret_cast<void*>(sizeof(f32)*2));
        
        glEnableVertexAttribArray(radius);
        glVertexAttribPointer(radius, 1, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE, reinterpret_cast<void*>(sizeof(f32)*4));
        
        glEnableVertexAttribArray(colour);
        glVertexAttribPointer(colour, 4, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE, reinterpret_cast<void*>(sizeof(f32)*5));
        
    }
    
    {
        glGenVertexArrays(1, &renderer.vaos[COMMAND_RECTANGLE_OUTLINE]);
        glBindVertexArray(renderer.vaos[COMMAND_RECTANGLE_OUTLINE]);
        
        glGenBuffers(1, &renderer.buffers[COMMAND_RECTANGLE_OUTLINE]);
        glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[COMMAND_RECTANGLE_OUTLINE]);
        
        // NOTE(Oliver): yeah maybe sort these constants out, looks wacko
        glBufferData(GL_ARRAY_BUFFER, MAX_DRAW*BYTES_PER_RECTANGLE_OUTLINE, 0, GL_DYNAMIC_DRAW);
        
        GLuint pos = 0;
        GLuint dim = 2;
        GLuint border_size = 3;
        GLuint radius = 4;
        GLuint colour = 5;
        
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, 2, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE_OUTLINE, reinterpret_cast<void*>(0));
        
        glEnableVertexAttribArray(dim);
        glVertexAttribPointer(dim, 2, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE_OUTLINE, reinterpret_cast<void*>(sizeof(f32)*2));
        
        glEnableVertexAttribArray(border_size);
        glVertexAttribPointer(border_size, 1, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE_OUTLINE, reinterpret_cast<void*>(sizeof(f32)*4));
        
        glEnableVertexAttribArray(radius);
        glVertexAttribPointer(radius, 1, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE_OUTLINE, reinterpret_cast<void*>(sizeof(f32)*5));
        
        glEnableVertexAttribArray(colour);
        glVertexAttribPointer(colour, 4, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE_OUTLINE, reinterpret_cast<void*>(sizeof(f32)*6));
        
    }
    
    {
        glGenVertexArrays(1, &renderer.vaos[COMMAND_CIRCLE]);
        glBindVertexArray(get_vao_circle());
        
        glGenBuffers(1, &renderer.buffers[COMMAND_CIRCLE]);
        glBindBuffer(GL_ARRAY_BUFFER, get_buffer_circle());
        
        // NOTE(Oliver): yeah maybe sort these constants out, looks wacko
        glBufferData(GL_ARRAY_BUFFER, MAX_DRAW*BYTES_PER_CIRCLE, 0, GL_DYNAMIC_DRAW);
        
        GLuint pos = 0;
        GLuint radius = 2;
        GLuint colour = 3;
        
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, 2, GL_FLOAT, false, 
                              BYTES_PER_CIRCLE, reinterpret_cast<void*>(0));
        
        glEnableVertexAttribArray(radius);
        glVertexAttribPointer(radius, 1, GL_FLOAT, false, 
                              BYTES_PER_CIRCLE, reinterpret_cast<void*>(sizeof(f32)*2));
        
        glEnableVertexAttribArray(colour);
        glVertexAttribPointer(colour, 4, GL_FLOAT, false, 
                              BYTES_PER_CIRCLE, reinterpret_cast<void*>(sizeof(f32)*3));
        
    }
    
    {
        glGenVertexArrays(1, &renderer.vaos[COMMAND_GLYPH]);
        glBindVertexArray(get_vao_glyph());
        
        glGenBuffers(1, &renderer.buffers[COMMAND_GLYPH]);
        glBindBuffer(GL_ARRAY_BUFFER, get_buffer_glyph());
        
        // NOTE(Oliver): yeah maybe sort these constants out, looks wacko
        glBufferData(GL_ARRAY_BUFFER, MAX_DRAW*BYTES_PER_GLYPH, 0, GL_DYNAMIC_DRAW);
        
        GLuint pos = 0;
        GLuint pos_dim = 2;
        GLuint uv = 4;
        GLuint uv_dim = 6;
        GLuint colour = 8;
        
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, 2, GL_FLOAT, false, 
                              BYTES_PER_GLYPH, reinterpret_cast<void*>(0));
        
        glEnableVertexAttribArray(pos_dim);
        glVertexAttribPointer(pos_dim, 2, GL_FLOAT, false, 
                              BYTES_PER_GLYPH, reinterpret_cast<void*>(sizeof(f32)*2));
        
        glEnableVertexAttribArray(uv);
        glVertexAttribPointer(uv, 2, GL_FLOAT, false, 
                              BYTES_PER_GLYPH, reinterpret_cast<void*>(sizeof(f32)*4));
        
        glEnableVertexAttribArray(uv_dim);
        glVertexAttribPointer(uv_dim, 2, GL_FLOAT, false, 
                              BYTES_PER_GLYPH, reinterpret_cast<void*>(sizeof(f32)*6));
        
        glEnableVertexAttribArray(colour);
        glVertexAttribPointer(colour, 4, GL_FLOAT, false, 
                              BYTES_PER_GLYPH, reinterpret_cast<void*>(sizeof(f32)*8));
        
    }
    
    {
        glGenVertexArrays(1, &renderer.vaos[COMMAND_RECTANGLE_TEXTURED]);
        glBindVertexArray(get_vao_rectangle_textured());
        
        glGenBuffers(1, &renderer.buffers[COMMAND_RECTANGLE_TEXTURED]);
        glBindBuffer(GL_ARRAY_BUFFER, get_buffer_rectangle_textured());
        
        // NOTE(Oliver): yeah maybe sort these constants out, looks wacko
        glBufferData(GL_ARRAY_BUFFER, MAX_DRAW*BYTES_PER_RECTANGLE_TEXTURED, 0, GL_DYNAMIC_DRAW);
        
        GLuint pos = 0;
        GLuint pos_dim = 2;
        GLuint radius = 4;
        GLuint uv = 5;
        GLuint uv_dim = 7;
        
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, 2, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE_TEXTURED, reinterpret_cast<void*>(0));
        
        glEnableVertexAttribArray(pos_dim);
        glVertexAttribPointer(pos_dim, 2, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE_TEXTURED, reinterpret_cast<void*>(sizeof(f32)*2));
        
        glEnableVertexAttribArray(radius);
        glVertexAttribPointer(radius, 1, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE_TEXTURED, reinterpret_cast<void*>(sizeof(f32)*4));
        
        glEnableVertexAttribArray(uv);
        glVertexAttribPointer(uv, 2, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE_TEXTURED, reinterpret_cast<void*>(sizeof(f32)*5));
        
        glEnableVertexAttribArray(uv_dim);
        glVertexAttribPointer(uv_dim, 2, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE_TEXTURED, reinterpret_cast<void*>(sizeof(f32)*7));
        
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
}

internal GLuint
make_program(char* vs, char* fs){
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;
    
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vs, nullptr);
    glCompileShader(vertex_shader);
    GLint compiled = 0;
    
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    if(compiled == GL_FALSE){
        GLint length = 0;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &length);
        
        GLchar error_log[4096];
        glGetShaderInfoLog(vertex_shader, length, &length, error_log);
        OutputDebugStringA(error_log);
    }
    
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fs, nullptr);
    glCompileShader(fragment_shader);
    
    compiled = 0;
    
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
    if(compiled == GL_FALSE){
        GLint length = 0;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &length);
        
        GLchar error_log[4096];
        glGetShaderInfoLog(fragment_shader, length, &length, error_log);
        OutputDebugStringA(error_log);
    }
    
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    
    glLinkProgram(program);
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return program;
}

internal void
init_shaders(){
    
    // NOTE(Oliver): init rectangle shader
    {
        GLchar* rectangle_vs =  
            "#version 330 core\n"
            "layout(location = 0) in vec2 pos; \n"
            "layout(location = 2) in vec2 dim; \n"
            "layout(location = 3) in float radius; \n"
            "layout(location = 4) in vec4 colour; \n"
            "uniform mat4x4 ortho;\n"
            "uniform mat4x4 view;\n"
            "uniform vec2 resolution;\n"
            "out vec2 out_pos;\n"
            "out vec2 out_dim;\n"
            "out float out_radius;\n"
            "out vec4 frag_colour;\n"
            
            "void main(){\n"
            "vec2 vertices[] = vec2[](vec2(-1, -1), vec2(1,-1), vec2(1,1),\n"
            "vec2(-1,-1), vec2(-1, 1), vec2(1, 1));"
            "vec4 screen_position = vec4(vertices[(gl_VertexID % 6)], 0, 1);\n"
            "screen_position.xy *= (vec4(1.2*dim/resolution, 0, 1)).xy;\n"
            "screen_position.xy += 2*(vec4((pos+dim/2)/resolution,0,1)).xy -1;\n"
            "gl_Position = screen_position;\n"
            "out_pos = pos;\n"
            "out_radius = radius;\n"
            "out_dim = dim;\n"
            "frag_colour = colour;\n"
            "}\n";
        
        GLchar* rectangle_fs =  
            "#version 330 core\n"
            "in vec2 out_pos; \n"
            "in vec2 out_dim; \n"
            "in float out_radius; \n"
            "in vec4 frag_colour;\n"
            "out vec4 colour;\n"
            "uniform vec2 in_position;\n"
            
            "float box_no_pointy(vec2 p, vec2 b, float r){\n"
            "return length(max(abs(p)-b+r,0.0))-r;\n"
            "}\n"
            
            "void main(){\n"
            "float dist = box_no_pointy(gl_FragCoord.xy - (out_pos + out_dim/2), out_dim/2, out_radius);\n"
            "float alpha = mix(1, 0,  smoothstep(0, 1, dist));\n"
            "vec3 debug_colour = mix(vec3(1,0,0), vec3(0,1,0), smoothstep(0, 1, dist));\n"
            "colour = vec4(frag_colour.rgb, alpha);\n"
            "}\n";
        
        GLuint program = make_program(rectangle_vs, rectangle_fs);
        
        renderer.resolution_uniforms[COMMAND_RECTANGLE] = glGetUniformLocation(program, "resolution");
        renderer.ortho_uniform = glGetUniformLocation(program, "ortho");
        renderer.view_uniform = glGetUniformLocation(program, "view");
        
        renderer.programs[COMMAND_RECTANGLE] = program;
        
    }
    
    
    // NOTE(Oliver): init rectangle outline shader
    {
        GLchar* rectangle_vs =  
            "#version 330 core\n"
            "layout(location = 0) in vec2 pos; \n"
            "layout(location = 2) in vec2 dim; \n"
            "layout(location = 3) in float border_size; \n"
            "layout(location = 4) in float radius; \n"
            "layout(location = 5) in vec4 colour; \n"
            "uniform mat4x4 ortho;\n"
            "uniform mat4x4 view;\n"
            "uniform vec2 resolution;\n"
            "out vec2 out_pos;\n"
            "out vec2 out_dim;\n"
            "out float out_radius;\n"
            "out float out_border;\n"
            "out vec2 out_res;\n"
            "out vec4 frag_colour;\n"
            
            "void main(){\n"
            "vec2 vertices[] = vec2[](vec2(-1, -1), vec2(1,-1), vec2(1,1),\n"
            "vec2(-1,-1), vec2(-1, 1), vec2(1, 1));"
            "vec4 screen_position = vec4(vertices[(gl_VertexID % 6)], 0, 1);\n"
            "screen_position.xy *= (vec4(1.2*dim/resolution, 0, 1)).xy;\n"
            "screen_position.xy += 2*(vec4((pos+dim/2)/resolution,0,1)).xy -1;\n"
            "gl_Position = screen_position;\n"
            "out_pos = pos;\n"
            "out_radius = radius;\n"
            "out_dim = dim;\n"
            "out_border = border_size;\n"
            "out_res = resolution;\n"
            "frag_colour = colour;\n"
            "}\n";
        
        GLchar* rectangle_fs =  
            "#version 330 core\n"
            "in vec2 out_pos; \n"
            "in vec2 out_dim; \n"
            "in float out_radius; \n"
            "in float out_border; \n"
            "in vec2 out_res; \n"
            "in vec4 frag_colour;\n"
            "out vec4 colour;\n"
            "uniform vec2 in_position;\n"
            
            "float box_no_pointy(vec2 p, vec2 b, float r, float border){\n"
            "float border_ = min(b.y,b.x)*border;\n"
            "float border_percent = border*min(b.y,b.x)/max(b.y,b.x);\n"
            "float inner = length(max(abs(p)-(b-border_)+r,0.0))-r*(1.0-border);\n"
            "float outer = length(max(abs(p)-b+r,0.0))-r;\n"
            "return max(-inner, outer);\n"
            "}\n"
            
            "float box_dist(vec2 p, vec2 size, float radius, float border){\n"
            "size -= vec2(radius);\n"
            "vec2 d = abs(p) - size;\n"
            "return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - radius;\n"
            "}\n"
            "void main(){\n"
            "float dist = box_no_pointy(gl_FragCoord.xy - (out_pos + out_dim/2), out_dim/2, out_radius*min(out_dim.x, out_dim.y)/2, out_border);\n"
            "if(dist <= 0.0001) { dist = 0.0001; }\n"
            "float alpha = mix(1, 0,  smoothstep(0, 1, dist));\n"
            "vec3 debug_colour = mix(vec3(1,0,0), vec3(0,1,0), smoothstep(0, 1, dist));\n"
            "colour = vec4(frag_colour.rgb, alpha);\n"
            "}\n";
        
        GLuint program = make_program(rectangle_vs, rectangle_fs);
        
        renderer.resolution_uniforms[COMMAND_RECTANGLE_OUTLINE] = glGetUniformLocation(program, "resolution");
        
        renderer.programs[COMMAND_RECTANGLE_OUTLINE] = program;
        
    }
    
    // NOTE(Oliver): init circle shader
    {
        GLchar* circle_vs =  
            "#version 330 core\n"
            "layout(location = 0) in vec2 pos; \n"
            "layout(location = 2) in float radius; \n"
            "layout(location = 3) in vec4 colour; \n"
            "uniform mat4x4 ortho;\n"
            "uniform mat4x4 view;\n"
            "uniform vec2 resolution;\n"
            "out vec2 out_pos;\n"
            "out float out_radius;\n"
            "out vec2 out_res;\n"
            "out vec4 frag_colour;\n"
            
            "void main(){\n"
            "vec2 vertices[] = vec2[](vec2(-1, -1), vec2(1,-1), vec2(1,1),\n"
            "vec2(-1,-1), vec2(-1, 1), vec2(1, 1));"
            "vec4 screen_position = vec4(vertices[(gl_VertexID % 6)], 0, 1);\n"
            "screen_position.xy *= (vec4((radius*1.2)/resolution, 0, 1)).xy;\n"
            "screen_position.xy += 2*(vec4((pos+radius/2)/resolution,0,1)).xy -1;\n"
            "gl_Position = screen_position;\n"
            "out_pos = pos;\n"
            "out_radius = radius;\n"
            "out_res = resolution;\n"
            "frag_colour = colour;\n"
            "}\n";
        
        GLchar* circle_fs =  
            "#version 330 core\n"
            "in vec2 out_pos; \n"
            "in float out_radius; \n"
            "in float out_border; \n"
            "in vec2 out_res; \n"
            "in vec4 frag_colour;\n"
            "out vec4 colour;\n"
            "uniform vec2 in_position;\n"
            
            "float circle(vec2 p, float r){\n"
            "return length(p) - r;\n"
            "}\n"
            
            "void main(){\n"
            "float dist = circle(gl_FragCoord.xy - (out_pos + out_radius/2), out_radius/2);\n"
            "if(dist <= 0.0001) { dist = 0.0001; }\n"
            "float alpha = mix(1, 0,  smoothstep(0, 2, dist));\n"
            "vec3 debug_colour = mix(vec3(1,0,0), vec3(0,1,0), smoothstep(0, 1, dist));\n"
            "colour = vec4(frag_colour.rgb, alpha);\n"
            "}\n";
        
        GLuint program = make_program(circle_vs, circle_fs);
        
        renderer.resolution_uniforms[COMMAND_CIRCLE] = glGetUniformLocation(program, "resolution");
        
        renderer.programs[COMMAND_CIRCLE] = program;
        
    }
    // NOTE(Oliver): init glyph shader
    {
        GLchar* glyph_vs =  
            "#version 330 core\n"
            "layout(location = 0) in vec2 pos; \n"
            "layout(location = 2) in vec2 pos_dim; \n"
            "layout(location = 4) in vec2 uv;\n"
            "layout(location = 6) in vec2 uv_dim; \n"
            "layout(location = 8) in vec4 colour;\n"
            "uniform mat4x4 ortho;\n"
            "uniform mat4x4 view;\n"
            "uniform vec2 resolution;\n"
            "out vec2 frag_pos;\n"
            "out vec2 frag_uv;\n"
            "out vec4 frag_colour;\n"
            
            "void main(){\n"
            "vec2 vertices[] = vec2[](vec2(-1, -1), vec2(1,-1), vec2(1,1),\n"
            "vec2(-1,-1), vec2(-1, 1), vec2(1, 1));\n"
            "vec2 uvs[] = vec2[](vec2(0, 0), vec2(1,0), vec2(1,1),\n"
            "vec2(0,0), vec2(0, 1), vec2(1, 1));\n"
            "vec4 screen_position = vec4(vertices[(gl_VertexID % 6)], 0, 1);\n"
            "screen_position.xy *= (vec4((pos_dim)/resolution, 0, 1)).xy;\n"
            "screen_position.xy += 2*(vec4((pos+pos_dim/2)/resolution,0,1)).xy -1;\n"
            "frag_uv = uvs[gl_VertexID % 6];\n"
            "frag_uv *= uv_dim;\n"
            "frag_uv += uv;\n"
            "gl_Position = screen_position;\n"
            "frag_pos = pos;\n"
            "frag_colour = colour;\n"
            "}\n";
        
        GLchar* glyph_fs =  
            "#version 330 core\n"
            "in vec2 frag_pos; \n"
            "in vec2 frag_uv; \n"
            "in vec2 frag_dim; \n"
            "in vec4 frag_colour; \n"
            "out vec4 colour;\n"
            "uniform sampler2D atlas;\n"
            
            "float width = 0.5;\n"
            "const float edge = 0.1;\n"
            
            "void main(){\n"
            "float distance = 1.0 - texture(atlas, frag_uv).a;\n"
            "float alpha = 1.0 - smoothstep(width, width + edge, distance);\n"
            "colour = vec4(frag_colour.rgb, alpha);\n"
            "}\n";
        
        GLuint program = make_program(glyph_vs, glyph_fs);
        
        renderer.resolution_uniforms[COMMAND_GLYPH] = glGetUniformLocation(program, "resolution");
        
        renderer.programs[COMMAND_GLYPH] = program;
        
        glGenTextures(1, &renderer.texture);
        glBindTexture(GL_TEXTURE_2D, renderer.texture);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512,
                     512, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     renderer.font.bitmap.data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
    }
    // NOTE(Oliver): init rectangle textured shader
    {
        GLchar* rectangle_vs =  
            "#version 330 core\n"
            "layout(location = 0) in vec2 pos; \n"
            "layout(location = 2) in vec2 pos_dim; \n"
            "layout(location = 4) in float radius; \n"
            "layout(location = 5) in vec2 uv;\n"
            "layout(location = 7) in vec2 uv_dim; \n"
            "uniform mat4x4 ortho;\n"
            "uniform mat4x4 view;\n"
            "uniform vec2 resolution;\n"
            "out vec2 frag_pos;\n"
            "out vec2 frag_uv;\n"
            "out vec2 frag_dim;\n"
            "out float frag_radius;\n"
            "out vec4 frag_colour;\n"
            
            "void main(){\n"
            "vec2 vertices[] = vec2[](vec2(-1, -1), vec2(1,-1), vec2(1,1),\n"
            "vec2(-1,-1), vec2(-1, 1), vec2(1, 1));\n"
            "vec2 uvs[] = vec2[](vec2(0,1), vec2(1,1), vec2(1,0),\n"
            "vec2(0, 1), vec2(0,0), vec2(1,0));\n"
            "vec4 screen_position = vec4(vertices[(gl_VertexID % 6)], 0, 1);\n"
            "screen_position.xy *= (vec4((pos_dim)/resolution, 0, 1)).xy;\n"
            "screen_position.xy += 2*(vec4((pos+pos_dim/2)/resolution,0,1)).xy -1;\n"
            "frag_uv = uvs[gl_VertexID % 6];\n"
            //"frag_uv *= uv_dim;\n"
            //"frag_uv += uv;\n"
            "frag_radius = radius;\n"
            "gl_Position = screen_position;\n"
            "frag_pos = pos;\n"
            "frag_dim = pos_dim;\n"
            "}\n";
        
        GLchar* rectangle_fs =  
            "#version 330 core\n"
            "in vec2 frag_pos; \n"
            "in vec2 frag_uv; \n"
            "in vec2 frag_dim; \n"
            "in float frag_radius;\n"
            "out vec4 colour;\n"
            "uniform sampler2D atlas;\n"
            
            "float box_no_pointy(vec2 p, vec2 b, float r){\n"
            "return length(max(abs(p)-b+r,0.0))-r;\n"
            "}\n"
            
            "void main(){\n"
            "float dist = box_no_pointy(gl_FragCoord.xy - (frag_pos + frag_dim/2), frag_dim/2, frag_radius);\n"
            "float alpha = mix(1, 0,  smoothstep(0, 1, dist));\n"
            "vec4 value = vec4(0);\n"
            "int count = 0;\n"
            "for(float i = -0.015; i < 0.015; i +=0.0025){\n"
            "for(float j = -0.015; j < 0.015; j += 0.0025){\n"
            "count += 1;\n"
            "value += texture(atlas, frag_uv + vec2(i,j));\n"
            "}\n"
            "}\n"
            "value /= count;\n"
            "colour = vec4(value.rgb, min(alpha, value.a));\n"
            "}\n";
        
        GLuint program = make_program(rectangle_vs, rectangle_fs);
        
        renderer.resolution_uniforms[COMMAND_RECTANGLE_TEXTURED] = glGetUniformLocation(program, "resolution");
        
        renderer.programs[COMMAND_RECTANGLE_TEXTURED] = program;
        
    }
    
}

internal void
process_and_draw_commands(){
    
    OPTICK_EVENT();
    
    f32* rectangles = (f32*)arena_allocate(&renderer.shape_attribs, MAX_DRAW*BYTES_PER_RECTANGLE);
    f32* rectangle_outlines = (f32*)arena_allocate(&renderer.shape_attribs, MAX_DRAW*BYTES_PER_RECTANGLE_OUTLINE);
    f32* circles = (f32*)arena_allocate(&renderer.shape_attribs, MAX_DRAW*BYTES_PER_CIRCLE);
    f32* glyphs = (f32*)arena_allocate(&renderer.shape_attribs, MAX_DRAW*BYTES_PER_GLYPH);
    f32* rectangles_textured = (f32*)arena_allocate(&renderer.shape_attribs, MAX_DRAW*BYTES_PER_RECTANGLE_TEXTURED);
    
    
    // TODO(Oliver): batch these in chunks to preserve draw order!
    
    v4f clip_range = v4f(0, 0, platform.width, platform.height);
    bool should_clip = false;
    for(Command* command = renderer.head; command; command = command->next){
        Command* previous_command = command;
        
        switch(command->type){
            case COMMAND_CLIP_RANGE_BEGIN: {
                should_clip = true;
                auto clip = command->clip_range;
                clip_range = v4f(clip.x, clip.y, clip.width, clip.height);
            }break;
            case COMMAND_CLIP_RANGE_END:{
                should_clip = false;
            }break;
            case COMMAND_RECTANGLE:{
                
                int num_verts = 0;
                f32* attribs = rectangles;
                for(Command* _command = command; _command && _command->type == previous_command->type; 
                    _command = _command->next){
                    
                    auto rectangle = _command;
                    for(int i = 0; i < 6; i++){
                        v4f r = v4f(rectangle->rectangle.x,
                                    rectangle->rectangle.y,
                                    rectangle->rectangle.width,
                                    rectangle->rectangle.height);
                        if(!should_clip || (should_clip && is_rect_inside_rect(r, clip_range))){
                            *attribs++ = rectangle->rectangle.x;
                            *attribs++ = rectangle->rectangle.y;
                            *attribs++ = rectangle->rectangle.width;
                            *attribs++ = rectangle->rectangle.height;
                            *attribs++ = rectangle->rectangle.corner_radius;
                            *attribs++ = (rectangle->colour.a/255.0f);
                            *attribs++ = (rectangle->colour.b/255.0f);
                            *attribs++ = (rectangle->colour.g/255.0f);
                            *attribs++ = (rectangle->colour.r/255.0f);
                            num_verts++;
                            
                        }else {
                            *attribs++ = rectangle->rectangle.x;
                            *attribs++ = rectangle->rectangle.y;
                            if(rectangle->rectangle.x + rectangle->rectangle.width < clip_range.x + clip_range.width){
                                *attribs++ = rectangle->rectangle.width;
                            }else {
                                *attribs++ = clip_range.width - rectangle->rectangle.x;
                            }
                            
                            *attribs++ = rectangle->rectangle.height;
                            *attribs++ = rectangle->rectangle.corner_radius;
                            *attribs++ = (rectangle->colour.a/255.0f);
                            *attribs++ = (rectangle->colour.b/255.0f);
                            *attribs++ = (rectangle->colour.g/255.0f);
                            *attribs++ = (rectangle->colour.r/255.0f);
                            num_verts++;
                            
                        }
                    }
                }
                // NOTE(Oliver): draw filled rects data
                {
                    glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[COMMAND_RECTANGLE]);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                    MAX_DRAW*BYTES_PER_RECTANGLE,
                                    rectangles);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                    glUseProgram(renderer.programs[COMMAND_RECTANGLE]);
                    
                    f32 resolution[2] = {(f32)platform.width, (f32)platform.height};
                    glUniform2fv(renderer.resolution_uniforms[COMMAND_RECTANGLE], 1, resolution);
                    
                    glBindVertexArray(renderer.vaos[COMMAND_RECTANGLE]);
                    glDrawArrays(GL_TRIANGLES, 0, num_verts);
                    glUseProgram(0);
                }
                
            }break;
            
            case COMMAND_RECTANGLE_OUTLINE:{
                int num_verts = 0;
                f32* attribs = rectangle_outlines;
                for(Command* _command = command; _command && _command->type == previous_command->type; 
                    _command = _command->next){
                    for(int i = 0; i < 6; i++){
                        auto rectangle = _command; 
                        *attribs++ = rectangle->rectangle_outline.x;
                        *attribs++ = rectangle->rectangle_outline.y;
                        *attribs++ = rectangle->rectangle_outline.width;
                        *attribs++ = rectangle->rectangle_outline.height;
                        *attribs++ = rectangle->rectangle_outline.border_size;
                        *attribs++ = rectangle->rectangle_outline.corner_radius;
                        *attribs++ = (rectangle->colour.a/255.0f);
                        *attribs++ = (rectangle->colour.b/255.0f);
                        *attribs++ = (rectangle->colour.g/255.0f);
                        *attribs++ = (rectangle->colour.r/255.0f);
                    }
                    num_verts += 6;
                }
                
                {
                    glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[COMMAND_RECTANGLE_OUTLINE]);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                    MAX_DRAW*BYTES_PER_RECTANGLE_OUTLINE,
                                    rectangle_outlines);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                    glUseProgram(renderer.programs[COMMAND_RECTANGLE_OUTLINE]);
                    
                    f32 resolution[2] = {(f32)platform.width, (f32)platform.height};
                    glUniform2fv(renderer.resolution_uniforms[COMMAND_RECTANGLE_OUTLINE], 
                                 1, resolution);
                    
                    glBindVertexArray(renderer.vaos[COMMAND_RECTANGLE_OUTLINE]);
                    glDrawArrays(GL_TRIANGLES, 0, num_verts);
                    glUseProgram(0);
                }
                
            }break;
            
            case COMMAND_CIRCLE:{
                f32* attribs = circles;
                int num_verts = 0;
                for(Command* _command = command; _command && 
                    _command->type == previous_command->type ? 
                    (previous_command = _command) : 0; 
                    _command = _command->next){
                    auto circle = _command; 
                    for(int i = 0; i < 6; i++){
                        *attribs++ = circle->circle.x;
                        *attribs++ = circle->circle.y;
                        *attribs++ = circle->circle.radius;
                        *attribs++ = circle->colour.a/255.0f;
                        *attribs++ = circle->colour.b/255.0f;
                        *attribs++ = circle->colour.g/255.0f;
                        *attribs++ = circle->colour.r/255.0f;
                    }
                    num_verts += 6;
                    
                }
                
                // NOTE(Oliver): draw circle data
                {
                    glBindBuffer(GL_ARRAY_BUFFER, get_buffer_circle());
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                    MAX_DRAW*BYTES_PER_CIRCLE,
                                    circles);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                    glUseProgram(get_program_circle());
                    
                    f32 resolution[2] = {(f32)platform.width, (f32)platform.height};
                    glUniform2fv(renderer.resolution_uniforms[COMMAND_CIRCLE], 1, resolution);
                    
                    glBindVertexArray(get_vao_circle());
                    glDrawArrays(GL_TRIANGLES, 0, num_verts);
                    glUseProgram(0);
                }
            }break;
            
            case COMMAND_GLYPH: {
                f32* attribs = glyphs;
                int num_verts = 0;
                for(Command* _command = command; _command && 
                    _command->type == previous_command->type ? 
                    (previous_command = _command) : 0; 
                    _command = _command->next){
                    auto glyph = _command;
                    for(int i = 0; i < 6; i++){
                        v4f r = v4f(glyph->glyph.x,
                                    glyph->glyph.y,
                                    glyph->glyph.width,
                                    glyph->glyph.height);
                        if(!should_clip || (should_clip && is_rect_inside_rect(r, clip_range))){
                            *attribs++ = glyph->glyph.x;
                            *attribs++ = glyph->glyph.y;
                            *attribs++ = glyph->glyph.width;
                            *attribs++ = glyph->glyph.height;
                            *attribs++ = glyph->glyph.u;
                            *attribs++ = glyph->glyph.v;
                            *attribs++ = glyph->glyph.u_width;
                            *attribs++ = glyph->glyph.v_height;
                            *attribs++ = glyph->colour.a/255.0f;
                            *attribs++ = glyph->colour.b/255.0f;
                            *attribs++ = glyph->colour.g/255.0f;
                            *attribs++ = glyph->colour.r/255.0f;
                            num_verts++;
                            
                        }
                    }
                }
                {
                    glBindBuffer(GL_ARRAY_BUFFER, get_buffer_glyph());
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                    MAX_DRAW*BYTES_PER_GLYPH,
                                    glyphs);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                    glUseProgram(get_program_glyph());
                    
                    f32 resolution[2] = {(f32)platform.width, (f32)platform.height};
                    glUniform2fv(renderer.resolution_uniforms[COMMAND_GLYPH], 1, resolution);
                    
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, renderer.texture);
                    
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    
                    glBindVertexArray(get_vao_glyph());
                    glDrawArrays(GL_TRIANGLES, 0, num_verts);
                    glUseProgram(0);
                }
            }break;
            
            case COMMAND_RECTANGLE_TEXTURED: {
                f32* attribs = rectangles_textured;
                
                auto rectangle = command;
                for(int i = 0; i < 6; i++){
                    *attribs++ = rectangle->rectangle_textured.x;
                    *attribs++ = rectangle->rectangle_textured.y;
                    *attribs++ = rectangle->rectangle_textured.width;
                    *attribs++ = rectangle->rectangle_textured.height;
                    *attribs++ = rectangle->rectangle_textured.corner_radius;
                    *attribs++ = rectangle->rectangle_textured.u;
                    *attribs++ = rectangle->rectangle_textured.v;
                    *attribs++ = rectangle->rectangle_textured.u_width;
                    *attribs++ = rectangle->rectangle_textured.v_height;
                }
                
                {
                    glBindBuffer(GL_ARRAY_BUFFER, get_buffer_rectangle_textured());
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                    MAX_DRAW*BYTES_PER_RECTANGLE_TEXTURED,
                                    rectangles_textured);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                    glUseProgram(get_program_rectangle_textured());
                    
                    f32 resolution[2] = {(f32)platform.width, (f32)platform.height};
                    glUniform2fv(renderer.resolution_uniforms[COMMAND_RECTANGLE_TEXTURED], 1, resolution);
                    
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, rectangle->rectangle_textured.bitmap.texture);
                    
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    
                    glBindVertexArray(get_vao_rectangle_textured());
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glUseProgram(0);
                }
            }break;
        }
        command = previous_command;
        
    }
    
}


internal void
opengl_start_frame() {
    OPTICK_EVENT();
    
    arena_reset(&renderer.shape_attribs);
    arena_reset(&renderer.frame_arena);
    arena_reset(&renderer.temp_string_arena);
    
    //arena_reset(&ui_state.frame_arena);
    arena_reset(&ui_state.parameter_arena);
    ui_state.widgets = nullptr;
    
    renderer.head = nullptr;
    renderer.tail = nullptr;
    
}

internal void
opengl_end_frame() {
    OPTICK_EVENT();
    
    
    glViewport(0, 0, platform.width, platform.height);
    glClearColor(theme.background.r/255.0f,
                 theme.background.g/255.0f,
                 theme.background.b/255.0f,
                 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    process_and_draw_commands();
    
    glUseProgram(0);
    
}

Bitmap cursor_bitmap;

internal void
display_modes(){
    
    return;
    
    f32 number_of_modes = 4;
    
    f32 mode_size = 60;
    f32 padding = 10;
    f32 offset = mode_size;
    
    f32 x = platform.width;
    f32 y = platform.height;
    
    for(int i = 0; i < number_of_modes; i++){
        push_rectangle(x-offset*2,y - (mode_size*number_of_modes) + (mode_size+padding)*i, 
                       mode_size, mode_size, 
                       0.3, theme.background.packed);
    }
}

internal UI_ID
button(f32 x, f32 y, char* text, Closure closure){
    auto id = gen_unique_id(text);
    
    v4f bbox = get_text_bbox(x, y, text);
    
    auto widget = ui_push_widget(bbox.x, bbox.y, bbox.width, bbox.height, id, closure);
    if(id == ui_state.clicked_id){
        push_rectangle(bbox, 10, theme.button_highlight.packed);
        push_string(x, y, text, theme.text.packed);
    }else if(id == ui_state.hover_id){
        push_rectangle(bbox, 10, theme.button_highlight.packed);
        push_string(x, y, text, theme.text.packed);
    }else{
        push_string(x, y, text, theme.text.packed);
    }
    
    return id;
}

internal UI_ID
text_button(char* text, f32 x, f32 y, b32* state, Closure closure){
    auto id = gen_unique_id(text);
    f32 line_height = get_font_line_height();
    auto widget = _push_widget(x, y, get_text_width(text), line_height,
                               id, closure);
    widget->clicked = state;
    f32 border = 5.0f;
    if(*state){
        v4f bbox = get_text_bbox(x, y, text);
        push_rectangle(bbox, 10, theme.view_button.packed);
        push_string(x, y, text, theme.text.packed);
    }else if(id == ui_state.hover_id){
        v4f bbox = get_text_bbox(x, y, text);
        push_rectangle(bbox, 10, theme.view_button.packed);
        push_string(x, y, text, theme.text.packed);
    }else{
        v4f bbox = get_text_bbox(x, y, text);
        push_rectangle(bbox, 10, theme.button_highlight.packed);
        push_string(x, y, text, theme.text.packed);
    }
    
    return id;
}

internal UI_ID
text_button(char* text, b32* state, Closure closure){
    f32 x = ui_get_x();
    f32 y = ui_get_y();
    auto id = gen_unique_id(text);
    auto widget = _push_widget(x, y, get_text_width(text), renderer.font.size,
                               id, closure);
    widget->clicked = state;
    f32 line_height = get_font_line_height();
    f32 border = 5.0f;
    auto anim_state = get_animation_state(id);
    if(!anim_state){
        anim_state = init_animation_state(id);
        anim_state->target_rect.x = 0.2f;
    }
    if(*state){
        animate(anim_state);
        
        f32 sx = (1.0 + anim_state->rect.x);
        v4f bbox = get_text_bbox(x, y, text);
        push_rectangle(bbox, 10, theme.view_button.packed);
        push_string(x, y, text, theme.text.packed);
    }else if(id == ui_state.hover_id){
        animate(anim_state);
        
        f32 sx = (1.0 + anim_state->rect.x);
        v4f bbox = get_text_bbox(x, y, text);
        push_rectangle(bbox, 10,
                       theme.view_button.packed);
        push_string(x, y, text, theme.text.packed);
    }else{
        unanimate(anim_state);
        f32 sx = (1.0 + anim_state->rect.x);
        v4f bbox = get_text_bbox(x, y, text);
        push_rectangle(bbox, 10,
                       theme.button_highlight.packed);
        push_string(x, y, text, theme.text.packed);
    }
    
    ui_state.x_offset += get_text_width(text) + 30.0f;
    
    return id;
}

internal UI_ID
icon_button(char* label, f32 x, f32 y, f32 size, Bitmap bitmap, Closure closure){
    auto id = gen_unique_id(label);
    auto widget = _push_widget(x, y, size, size, id, closure);
    
    f32 line_height = get_font_line_height();
    auto anim_state = get_animation_state(id);
    if(!anim_state){
        anim_state = init_animation_state(id);
        anim_state->target_rect.x = 0.5f;
    }
    if(id == ui_state.clicked_id){
        unanimate(anim_state);
        
        f32 sx = (1.0 + anim_state->rect.x);
        size *= sx;
        push_rectangle(x, y, size, size, 10, theme.button_highlight.packed);
        push_rectangle_textured(x+size/4, y+size/4, size/2, size/2, 0, bitmap);
    }else if(id == ui_state.hover_id){
        animate(anim_state);
        
        f32 sx = (1.0 + anim_state->rect.x);
        size *= sx;
        push_rectangle(x, y, size, size, 10, theme.button_highlight.packed);
        push_rectangle_textured(x+size/4, y+size/4, size/2, size/2, 0, bitmap);
    }else{
        unanimate(anim_state);
        
        f32 sx = (1.0 + anim_state->rect.x);
        size *= sx;
        push_rectangle(x, y, size, size, 10, theme.view_button.packed);
        push_rectangle_textured(x+size/4, y+size/4, size/2, size/2, 0, bitmap);
    }
    
    return id;
}


internal UI_ID
small_icon_button(char* label, f32 x, f32 y, f32 size, Bitmap bitmap, b32* state, Closure closure){
    auto id = gen_unique_id(label);
    auto widget = _push_widget(x, y, size, size, id, closure);
    widget->clicked = state;
    f32 line_height = get_font_line_height();
    auto anim_state = get_animation_state(id);
    if(!anim_state){
        anim_state = init_animation_state(id);
        anim_state->target_rect.x = 0.1f;
    }
    if(*state){
        unanimate(anim_state);
        
        f32 sx = (1.0 + anim_state->rect.x);
        size *= sx;
        push_rectangle(x, y, size, size, 10, theme.button_highlight.packed);
        push_rectangle_textured(x+size/4, y+size/4, size/2, size/2, 0, bitmap);
    }else if(id == ui_state.hover_id){
        animate(anim_state);
        
        f32 sx = (1.0 + anim_state->rect.x);
        size *= sx;
        push_rectangle(x, y, size, size, 10, theme.button_highlight.packed);
        push_rectangle_textured(x+size/4, y+size/4, size/2, size/2, 0, bitmap);
    }else{
        unanimate(anim_state);
        
        f32 sx = (1.0 + anim_state->rect.x);
        size *= sx;
        push_rectangle(x, y, size, size, 10, theme.view_button.packed);
        push_rectangle_textured(x+size/4, y+size/4, size/2, size/2, 0, bitmap);
    }
    
    return id;
}


Bitmap bitmap;
Bitmap move_icon;
Bitmap add_icon;
Bitmap options_icon;
Bitmap bin_icon;
internal void
find_node_types_helper(Node* root, Node*** node_list, u64 node_list_length, Node_Type type){
    if(!root) return;
    if(type == root->type){
        **node_list = root;
        (*node_list)++;
    }
    switch(root->type){
        
        case NODE_BINARY:{
            auto binary = &root->binary;
            find_node_types_helper(binary->left, node_list, node_list_length, type);
            find_node_types_helper(binary->right, node_list, node_list_length, type);
        }break;
        
        case NODE_LITERAL:{
            
        }break;
        
        case NODE_STRUCT:{
            auto _struct = &root->_struct;
            for(Node* member = _struct->members; member; member = member->next){
                find_node_types_helper(member, node_list, node_list_length, type);
            }
        }break;
        
        case NODE_DECLARATION:{
            auto decl = root->declaration.expression;
            find_node_types_helper(root->declaration.type_usage, node_list, 
                                   node_list_length, type);
            find_node_types_helper(decl, node_list, node_list_length, type);
        }break;
        
        case NODE_SCOPE:{
            auto scope = &root->scope;
            for(Node* stmt = scope->statements; stmt; stmt = stmt->next){
                find_node_types_helper(stmt, node_list, node_list_length, type);
            }
        }break;
        
        case NODE_TYPE_USAGE:{
        }break;
    }
}

internal void
find_node_types(Node** node_list, u64 node_list_length, Node_Type type){
    find_node_types_helper(friday.program_root, &node_list, node_list_length, type);
}

global bool menu_open = 0;

internal void
draw_menu(f32 x, f32 y, char* label, String8* strings, u64 num_rows, Closure closure){
    auto menu_id = gen_id(label);
    //debug_print("%d", menu_id);
    auto anim_state = get_animation_state(menu_id);
    if(!anim_state){
        anim_state = init_animation_state(menu_id);
    }
    update_animation_state(anim_state,
                           0,
                           0,
                           lerp(anim_state->x_scale, 1.2, 0.2f),
                           lerp(anim_state->y_scale, 1.2, 0.2f));
    //anim_state->y_scale += lerp(anim_state->y_scale, 1.2, 0.2f);
    //anim_state->x_scale += lerp(anim_state->x_scale, 1.2, 0.2f);
    f32 line_height = get_font_line_height();
    f32 height = 40*anim_state->y_scale;
    f32 size_x = 200*anim_state->x_scale;
    f32 size_y = num_rows*height;
    y = y-size_y+line_height;
    friday.y -= size_y;
    
    anim_state->last_updated = platform.tick;
    
    for(int i = 0; i < num_rows; i++){
        f32 text_width = get_text_width(strings[i]);
        text_width *= 1.2;
        size_x = text_width >= size_x ? text_width : size_x;
        
    }
    auto menu_widget = _push_widget(x, y, size_x, size_y, menu_id, closure);
    
    
    push_rectangle(x,y, size_x, num_rows*height, 10, theme.menu.packed);
    
    for(int i = 0; i < num_rows; i++){
        
        String8 string = strings[i];
        f32 offset = i*height;
        push_rectangle(x,y + offset, size_x, height, 10, theme.menu.packed);
        u32 text_colour = theme.text.packed;
        
        
        if(is_mouse_in_rect(x, y + offset, size_x, height)){
            if(platform.mouse_left_clicked){
                
                friday.selected = i;
                
                ui_state.menu_id = -1;
                //friday.is_menu_open = false;
            }
            push_rectangle(x, y+ offset, 
                           size_x, height, 10.0,
                           theme.menu.packed);
            
            text_colour = theme.cursor.packed;
        }
        
        push_string8(x+10, y + i*height+line_height/2, string,
                     text_colour);
    }
    
}

internal void
draw_menu_bar(){
    int size = 40;
    int x = 5;
    push_rectangle(x, platform.height-size, platform.width-10, size+10, 5, theme.panel.packed);
    x += 10;
    push_rectangle_textured(x,platform.height-size/2-30/2, 30, 30, 1, bitmap);
    
    UI_ID file_id = 0;
    UI_ID edit_id = 0;
    UI_ID help_id = 0;
    
    f32 file_x = 0;
    f32 edit_x = 0;
    f32 help_x = 0;
    
    auto menu_callback = [](u8* parameters){
        menu_open = 1;
    };
    
    f32 gap = 30.0f;
    {
        char* file = "File"; 
        
        Closure file_menu = make_closure(menu_callback, 0);
        x+=get_text_width(file);
        file_id = button(x, platform.height-size+5, file, file_menu);
        file_x = x;
        
    }
    
    {
        
        char* edit = "Edit"; 
        Closure edit_menu = make_closure(menu_callback, 0);
        x+=get_text_width(edit)+gap;
        edit_id = button(x, platform.height-size+5, edit, edit_menu);
        edit_x = x;
    }
    
    {
        
        char* help = "Help"; 
        Closure help_menu = make_closure(menu_callback, 0);
        x += get_text_width(help)+gap;
        
        help_id = button(x, platform.height-size+5, help, help_menu);
        help_x = x;
    }
    
    if(help_id == ui_state.hover_id){
        int x = 5;
    }
    
    if(menu_open && file_id == ui_state.clicked_id){
        String8 items[4] = {};
        Arena* arena = &renderer.temp_string_arena;
        items[3] = make_string(arena, "New");
        items[2] = make_string(arena, "Open");
        items[1] = make_string(arena, "Open Recent");
        items[0] = make_string(arena, "Save");
        Closure empty = {};
        draw_menu(file_x, platform.height-size-40, "file_menu",
                  items, 4, empty);
        
    }
    
    if(menu_open && edit_id == ui_state.clicked_id){
        String8 items[5] = {};
        Arena* arena = &renderer.temp_string_arena;
        items[4] = make_string(arena, "Undo");
        items[3] = make_string(arena, "Redo");
        items[2] = make_string(arena, "Undo History");
        items[1] = make_string(arena, "Repeat");
        items[0] = make_string(arena, "Preferences");
        Closure empty = {};
        draw_menu(edit_x, platform.height-size-40, "edit_menu",
                  items, 5, empty);
        
    }
    
    if(menu_open && help_id == ui_state.clicked_id){
        String8 items[2] = {};
        Arena* arena = &renderer.temp_string_arena;
        items[1] = make_string(arena, "About");
        items[0] = make_string(arena, "Splash Screen");
        Closure empty = {};
        draw_menu(help_x, platform.height-size-40, "help_menu",
                  items, 2, empty);
        
    }
    
}

internal void
draw_view_buttons(){
    f32 size = 60;
    f32 x = 30;
    f32 spacing = 15;
    f32 y = platform.height - 150;
    
    Bitmap icons[4] = {move_icon, add_icon, options_icon, bin_icon};
    char* icon_labels[4] = {"move_icon", "add_icon", "options_icon", "bin_icon"};
    for(int i = 0; i < 4; i++){
        
#if 0
        push_rectangle(x, y, size, size, 10, theme.view_button.packed);
        push_rectangle_textured(x+size/4, y+size/4, size/2, size/2, 5, icons[i]);
#endif
        auto callback = [](u8* parameters){};
        Closure closure = make_closure(callback, 0);
        icon_button(icon_labels[i], x, y, size, icons[i], closure);
        y -= size + spacing;
        
    }
    
}


internal UI_ID
radio_button(char* label, f32 x, f32 y, b32* state, Closure closure){
    auto id = gen_unique_id(label);
    
    f32 height = get_font_line_height();
    f32 width = height*2;
    auto anim_state = get_animation_state(id);
    if(!anim_state){
        anim_state = init_animation_state(id);
        anim_state->target_rect.x = width-height;
    }
    f32 start_x = x;
    
    push_string(x, y, label, theme.text.packed);
    x += get_text_width(label);
    
    auto widget = _push_widget(x+5, y-5, width, height, id,
                               closure);
    Colour back_colour = theme.view_button;
    
    widget->clicked = state;
    if(*widget->clicked){
        animate(anim_state);
    }else {
        unanimate(anim_state);
    }
    
    push_rectangle(x+5, y - 5, width, height, height/2, back_colour.packed);
    push_circle(x+5+anim_state->rect.x, y-5, height, theme.text_misc.packed);
    
    return id;
}

internal UI_ID
radio_button(char* label, b32* state, Closure closure){
    f32 x = ui_get_x();
    f32 y = ui_get_y();
    
    auto id = gen_unique_id(label);
    
    f32 height = get_font_line_height();
    f32 width = height*2;
    auto anim_state = get_animation_state(id);
    if(!anim_state){
        anim_state = init_animation_state(id);
        anim_state->target_rect.x = width-height;
    }
    f32 start_x = x;
    
    push_string(x, y, label, theme.text.packed);
    x += get_text_width(label);
    
    auto widget = _push_widget(x+5, y-5, width, height, id,
                               closure);
    Colour back_colour = theme.view_button;
    
    widget->clicked = state;
    if(*widget->clicked){
        animate(anim_state);
    }else {
        unanimate(anim_state);
    }
    
    
    
    push_rectangle(x+5, y - 5, width, height, height/2, back_colour.packed);
    push_circle(x+5+anim_state->rect.x, y-5, height, theme.text_misc.packed);
    
    ui_state.x_offset += get_text_width(label) +  width + 5 + 30;
    
    return id;
}

Bitmap search_icon;
Bitmap run_icon;
Bitmap layers_icon;
Bitmap document_icon;


global bool cursor_size = 0;
global f32 start_width = 0;

internal void 
draw_panels(Panel* root, int posx, int posy, int width, int height, u32 colour = 0xFFFFFFFF){
    if(!root) return;
    auto id = gen_unique_id("panel");
    f32 new_width = width;
    f32 new_height = height;
    f32 new_posx = posx;
    f32 new_posy = posy;
    
    for(int i = 0; i < 2; i++){
        
        if(root->children[i]){
            switch(root->children[i]->split_type){
                case PANEL_SPLIT_HORIZONTAL:{
                    new_height = root->children[i]->height;
                    new_posy += new_height;
                    draw_panels(root->children[i], posx, new_posy, new_width, height-new_height, colour);
                }break;
                case PANEL_SPLIT_VERTICAL:{
                    new_width = root->children[i]->width;
                    new_posx += new_width;
                    draw_panels(root->children[i], new_posx, posy, width-new_width, new_height, colour);
                }break;
            }
        }
        new_posx = 0;
        new_posy = 0;
    }
    //push_rectangle(posx,posy+PANEL_BORDER, PANEL_BORDER, new_height-PANEL_BORDER, 0, 0xFF0000FF);
    v4f panel_border = v4f(posx-PANEL_BORDER,posy+PANEL_BORDER, PANEL_BORDER*2, new_height-PANEL_BORDER);
    if(is_mouse_in_rect(panel_border)){
        SDL_Cursor* cursor;
        cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
        SDL_SetCursor(cursor);
        cursor_size = true;
    }else if(!cursor_size){
        SDL_Cursor* cursor;
        cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
        SDL_SetCursor(cursor);
    }else {
        cursor_size = 0;
    }
    if(!panel_resize && is_mouse_dragged(panel_border)){
        panel_resize = 1;
        old_width = width;
        active_panel = root;
        start_width = root->width;
    }
    
    if(panel_resize){
        
        f32 delta =  (platform.mouse_x-platform.mouse_drag_x);
        if(root == active_panel){
            root->width = start_width + delta;
        }
    }
    auto callback = [](u8* parameters){
        return;
    };
    
    Closure closure = make_closure(callback, 0);
    auto widget = _push_widget(posx+PANEL_BORDER, posy+PANEL_BORDER,
                               new_width-PANEL_BORDER*2, new_height-PANEL_BORDER*2, id,
                               closure);
    if(root->type == PANEL_PROPERTIES){
        ui_begin_panel(posx+PANEL_BORDER+50, posy+PANEL_BORDER, new_width-PANEL_BORDER*2, new_height-PANEL_BORDER*2);
        
        Bitmap icons[] = {
            search_icon,
            run_icon,
            layers_icon,
            document_icon,
        };
        char* icon_labels[] = {
            "search",
            "run",
            "layers",
            "document"
        };
        f32 x = posx + PANEL_BORDER;
        f32 y = new_height-PANEL_BORDER;
        f32 spacing = 15;
        f32 size = 40;
        local_persist b32 previous_states[4] = {true, false, false, false};
        local_persist b32 property_states[4] = {true, false, false, false};
        bool dummy = false;
        int index = -1;
        for(int i = 0; i < 4; i++){
            auto callback = [](u8* parameters){};
            Closure closure = make_closure(callback, 0);
            
            small_icon_button(icon_labels[i], x, y, size, icons[i], &property_states[i], closure);
            
            y -= size + spacing;
            
        }
        for(int i = 0; i < 4; i++){
            if(property_states[i] && !previous_states[i]){
                
                property_states[0] = false;
                property_states[1] = false;
                property_states[2] = false;
                property_states[3] = false;
                property_states[i] = true;
                
                previous_states[0] = property_states[0];
                previous_states[1] = property_states[1];
                previous_states[2] = property_states[2];
                previous_states[3] = property_states[3];
            }
        }
        push_rectangle(posx+PANEL_BORDER+35,posy+PANEL_BORDER, 
                       new_width-PANEL_BORDER*2-35, new_height-PANEL_BORDER*2, 5, colour);
        
        local_persist b32 button_states[6] = {0};
        local_persist b32 state = false;
        if(property_states[0]){
            radio_button("test", &state, {});
        }else if(property_states[1]){
            text_button("compile", &button_states[0], {});
            text_button("edit", &button_states[1], {});
            ui_new_line();
            
            radio_button("flip", &button_states[2], {});
            text_button("interpret", &button_states[3], {});
        }
        
    }else {
        ui_begin_panel(posx+PANEL_BORDER, posy+PANEL_BORDER, new_width-PANEL_BORDER*2, new_height-PANEL_BORDER*2);
        
        push_rectangle(posx+PANEL_BORDER,posy+PANEL_BORDER, 
                       new_width-PANEL_BORDER*2, new_height-PANEL_BORDER*2, 5, colour);
        present(root->presenter);
        
    }
    ui_end_panel();
    
    reset_presenter(root->presenter);
}
