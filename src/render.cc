
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
    };
};

struct Font {
    stbtt_fontinfo info;
    f32 line_height;
    f32 scale;
    f32 ascent;
    f32 descent;
    // NOTE(Oliver): we store the font
    // texture here
#define FONT_BITMAP_SIZE 4096
    u8* bitmap;
    
#define CHAR_INFO_SIZE ('~' - ' ')
    stbtt_packedchar char_infos[CHAR_INFO_SIZE];
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
    
    // NOTE(Oliver): this should probably just be static
    // we do have a max draw value anyway
    //Array<Command*> commands;
    Command* head = nullptr;
    Command* tail = nullptr;
    
    // TODO(Oliver): remove last array and 
    // move to arena
    Array<Font> fonts;
    
    Arena shape_attribs;
    Arena frame_arena;
    Arena temp_string_arena;
} renderer;

internal Command*
make_command(Command_Type type){
    Command* command = (Command*)arena_allocate(&renderer.frame_arena, sizeof(Command));
    command->type = type;
    command->next = nullptr;
    command->previous = nullptr;
    return command;
}

internal Font
init_font(char* font_name, int font_size){
    
    Font result;
    
    FILE* font_file = fopen(font_name, "rb");
    fseek(font_file, 0, SEEK_END);
    u64 size = ftell(font_file); 
    fseek(font_file, 0, SEEK_SET);
    
    u8* ttf_buffer = (u8*)malloc(size);
    
    fread(ttf_buffer, size, 1, font_file);
    fclose(font_file);
    
    
    assert(stbtt_InitFont(&result.info, ttf_buffer, 0));
    
    result.line_height = font_size;
    result.scale = stbtt_ScaleForPixelHeight(&result.info, result.line_height);
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&result.info, &ascent, &descent, &line_gap);
    result.ascent = result.scale * ascent;
    result.descent = result.scale * descent;
    
    int advance_x, left_side_bearing;
    stbtt_GetCodepointHMetrics(&result.info, ' ', 
                               &advance_x, &left_side_bearing);
    assert(advance_x > 0);
    
    result.bitmap = (u8*)malloc(FONT_BITMAP_SIZE*FONT_BITMAP_SIZE);
    
    stbtt_pack_context context;
    assert(stbtt_PackBegin(&context, result.bitmap, FONT_BITMAP_SIZE, 
                           FONT_BITMAP_SIZE, 0, 1, nullptr));
    
    
    stbtt_PackFontRange(&context, ttf_buffer, 0, result.line_height,
                        ' ', CHAR_INFO_SIZE, result.char_infos);
    
    
    stbtt_PackEnd(&context);
    
    return result;
}


internal void
insert_command(Command* next_command){
    if(!renderer.tail){
        renderer.head = next_command;
        renderer.tail = renderer.head;
    }else {
        renderer.tail->next = next_command;
        renderer.tail = renderer.tail->next;
    }
    return;
    
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
push_glyph(stbtt_aligned_quad quad, u32 colour){
    
    f32 x0, x1, y0, y1, s0, s1, t0, t1;
    x0 = quad.x0;
    x1 = quad.x1;
    y0 = -quad.y0;
    y1 = -quad.y1;
    
    s0 = quad.s0; s1 = quad.s1;
    t0 = quad.t0; t1 = quad.t1;
    
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
push_string(f32 x, f32 y, char* text, u32 colour = 0xFF00FFFF){
    
    y = -y;
    
    // NOTE(Oliver): '#' is used for ID purposes
    while(*text && *text != '#'){
        if(*text >= 32 && *text < 128){
            stbtt_aligned_quad quad;
            stbtt_GetPackedQuad(renderer.fonts[0].char_infos, 4096, 4096,
                                *text-32, &x, &y, &quad, 1);
            push_glyph(quad, colour);
        }
        text++;
    }
}

internal void
push_string8(f32 x, f32 y, String8 string, u32 colour = 0xFF00FFFF){
    
    y = -y;
    
    // NOTE(Oliver): '#' is used for ID purposes
    for(int i = 0; i < string.length; i++){
        char text = string[i];
        if(text == '#'){break;}
        //while(string.text && *string.text && *string.text != '#'){
        if(text >= 32 && text < 128){
            stbtt_aligned_quad quad;
            stbtt_GetPackedQuad(renderer.fonts[0].char_infos, 4096, 4096,
                                text-32, &x, &y, &quad, 1);
            push_glyph(quad, colour);
        }
    }
}

internal f32
get_text_width(char* text){
    f32 result = 0;
    while(text && *text){
        if(*text == '#') break;
        s32 advance_x;
        s32 left_side_bearing;
        stbtt_GetCodepointHMetrics(&renderer.fonts[0].info, *text, &advance_x, &left_side_bearing);
        int x0, x1, y0, y1;;
        stbtt_GetCodepointBitmapBox(&renderer.fonts[0].info, *text, renderer.fonts[0].scale, 
                                    renderer.fonts[0].scale,
                                    &x0, &y0, &x1, &y1);
        result += advance_x;
        result += x1 - x0;
        text++;
    }
    return result*renderer.fonts[0].scale;
}

internal f32
get_text_width(String8 string){
    f32 result = 0;
    for(int i = 0; i < string.length; i++){
        char text = string[i];
        if(text == '#') break;
        s32 advance_x;
        s32 left_side_bearing;
        stbtt_GetCodepointHMetrics(&renderer.fonts[0].info, text, &advance_x, &left_side_bearing);
        int x0, x1, y0, y1;
        stbtt_GetCodepointBitmapBox(&renderer.fonts[0].info, text, renderer.fonts[0].scale, 
                                    renderer.fonts[0].scale,
                                    &x0, &y0, &x1, &y1);
        result += advance_x;
        //result += x1 - x0;
    }
    return result*renderer.fonts[0].scale;
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
            
            "void main(){\n"
            "float alpha = texture(atlas, frag_uv).r;\n"
            "colour = vec4(frag_colour.rgb, alpha);\n"
            "}\n";
        
        GLuint program = make_program(glyph_vs, glyph_fs);
        
        renderer.resolution_uniforms[COMMAND_GLYPH] = glGetUniformLocation(program, "resolution");
        
        renderer.programs[COMMAND_GLYPH] = program;
        
        glGenTextures(1, &renderer.texture);
        glBindTexture(GL_TEXTURE_2D, renderer.texture);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, FONT_BITMAP_SIZE,
                     FONT_BITMAP_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE,
                     renderer.fonts[0].bitmap);
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
    
    for(Command* command = renderer.head; command; command = command->next){
        Command* previous_command = command;
        
        switch(command->type){
            case COMMAND_RECTANGLE:{
                
                int num_verts = 0;
                f32* attribs = rectangles;
                for(Command* _command = command; _command && _command->type == previous_command->type; 
                    _command = _command->next){
                    
                    auto rectangle = _command;
                    for(int i = 0; i < 6; i++){
                        *attribs++ = rectangle->rectangle.x;
                        *attribs++ = rectangle->rectangle.y;
                        *attribs++ = rectangle->rectangle.width;
                        *attribs++ = rectangle->rectangle.height;
                        *attribs++ = rectangle->rectangle.corner_radius;
                        *attribs++ = (rectangle->colour.a/255.0f);
                        *attribs++ = (rectangle->colour.b/255.0f);
                        *attribs++ = (rectangle->colour.g/255.0f);
                        *attribs++ = (rectangle->colour.r/255.0f);
                        
                    }
                    num_verts += 6;
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
                                    MAX_DRAW,
                                    attribs);
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
                        
                    }
                    num_verts += 6;
                    
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
    auto widget = _push_widget(x, y, get_text_width(text), renderer.fonts[0].line_height,
                               id, closure);
    f32 line_height = renderer.fonts[0].line_height;
    f32 border = 5.0f;
    if(id == ui_state.clicked_id){
        
        push_rectangle(x-border, y, get_text_width(text)+border*2, line_height, 10,
                       theme.button_highlight.packed);
        push_string(x, y+line_height/4, text, theme.text.packed);
    }else if(id == ui_state.hover_id){
        push_rectangle(x-border, y, get_text_width(text)+border*2, line_height, 10,
                       theme.button_highlight.packed);
        push_string(x, y+line_height/4, text, theme.text.packed);
    }else{
        push_string(x, y+line_height/4, text, theme.text.packed);
    }
    
    return id;
}

internal UI_ID
icon_button(char* label, f32 x, f32 y, f32 size, Bitmap bitmap, Closure closure){
    auto id = gen_unique_id(label);
    auto widget = _push_widget(x, y, size, size, id, closure);
    
    f32 line_height = renderer.fonts[0].line_height;
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
    auto menu_id = gen_unique_id(label);
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
    f32 line_height = renderer.fonts[0].line_height;
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
    int x = 0;
    push_rectangle(x += 5, platform.height-size, platform.width-10, size+10, 5, theme.panel.packed);
    push_rectangle_textured(x += 10,platform.height-size/2-30/2, 30, 30, 1, bitmap);
    
    UI_ID file_id, edit_id, help_id;
    
    f32 file_x;
    f32 edit_x;
    f32 help_x;
    
    f32 gap = 30.0f;
    {
        char* file = "File"; 
        auto file_menu_callback = [](u8* parameters){
            menu_open = 1;
        };
        Closure file_menu = make_closure(file_menu_callback, 0);
        file_id = button(x+=get_text_width(file), platform.height-size+5, file, file_menu);
        file_x = x;
        
    }
    
    {
        auto edit_menu_callback = [](u8* parameters){
            menu_open = 1;
        };
        char* edit = "Edit"; 
        Closure edit_menu = make_closure(edit_menu_callback, 0);
        edit_id = button(x+=get_text_width(edit)+gap, platform.height-size+5, edit, edit_menu);
        edit_x = x;
    }
    
    {
        auto help_menu_callback = [](u8* parameters){
            menu_open = 1;
        };
        char* help = "Help"; 
        Closure help_menu = make_closure(help_menu_callback, 0);
        help_id = button(x+=get_text_width(help)+gap, platform.height-size+5, help, help_menu);
        help_x = x;
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
draw_status_bar(){
    int size = 40;
    int x = 0;
    push_rectangle(x += 5, -size, platform.width-10, size*2, 10, theme.panel.packed);
    
    char* file = "active node: "; 
    push_string(x+=5, size/2-renderer.fonts[0].line_height/2, 
                file, theme.text.packed);
    
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