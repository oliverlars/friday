
// NOTE(Oliver): this is global state for the render pass
// may not be needed, we'll see
struct {
    int x;
    int y;
    int x_offset;
    
    Pool node_pool;
} friday;

// NOTE(Oliver): probably should do this better
internal inline int
get_friday_x(){
    return friday.x + friday.x_offset;
}

internal inline int
get_friday_y(){
    return friday.y;
}


// NOTE(Oliver): 0xAABBGGRR 
union Colour {
    u32 packed;
    struct {
        u8 a;
        u8 b;
        u8 g;
        u8 r;
    };
};

struct Theme {
    Colour base;
    Colour base_margin;
    Colour text;
    Colour text_light;
    Colour text_comment;
    Colour text_function;
    Colour text_type;
    Colour cursor;
    Colour error;
};

global Theme theme;

internal void
load_theme_ayu(){
    
    theme.base.packed = 0x0f1419ff;
    theme.base_margin.packed = 0x0a0e12ff;
    theme.text.packed = 0xFFFFFFff;
    theme.text_light.packed = 0x262c33ff;
    theme.text_comment.packed = 0xffc2d94d;
    theme.text_function.packed = 0xff5ac2ff;
    theme.text_type.packed = 0xffff29719;
    theme.cursor.packed = 0xe08c17ff;
    theme.error.packed = 0xffcc3333;
}

enum Command_Type {
    COMMAND_RECTANGLE,
    COMMAND_CIRCLE,
    COMMAND_RECTANGLE_OUTLINE,
    COMMAND_GLYPH,
    
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
};


struct Command_Rectangle: Command {
    Command_Rectangle() { type = COMMAND_RECTANGLE; }
#define BYTES_PER_RECTANGLE (9*sizeof(f32)) 
    // NOTE(Oliver): bytes include size of colour, which is 4*sizeof(f32)
    f32 x, y;
    f32 width, height;
    f32 corner_radius;
};

struct Command_Circle : Command {
    Command_Circle() { type = COMMAND_CIRCLE; }
#define BYTES_PER_CIRCLE (7*sizeof(f32))
    f32 x, y;
    f32 radius;
};

struct Command_Rectangle_Outline : Command {
    Command_Rectangle_Outline() { type = COMMAND_RECTANGLE_OUTLINE; }
#define BYTES_PER_RECTANGLE_OUTLINE (10*sizeof(f32))
    f32 x, y;
    f32 width, height;
    f32 border_size;
    f32 corner_radius;
};

struct Font;

struct Command_Glyph : Command {
    Command_Glyph() { type = COMMAND_GLYPH; }
#define BYTES_PER_GLYPH (12*sizeof(f32))
    f32 x, y;
    f32 width, height;
    f32 u, v;
    f32 u_width, v_height; 
    Font* font;
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

const int MAX_DRAW = 8192*4;

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
    Arena commands;
    Command* head = nullptr;
    Command* tail = nullptr;
    
    // TODO(Oliver): potential opportunities
    // to make these arenas when i understand
    // them better
    Array<Font> fonts;
    
    Array<f32> rectangle_attribs;
    Array<f32> rectangle_outline_attribs;
    Array<f32> circle_attribs;
    Array<f32> glyph_attribs;
    
    Arena shape_attribs;
    Arena frame_arena;
} renderer;

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

#define make_command(type) (type*) new (arena_allocate(&renderer.commands, sizeof(type))) type()

internal void
insert_command(Command* next_command){
    if(renderer.head){
        
        Command* command = renderer.head;
        while(command->next){
            command = command->next;
        }
        command->next = next_command;
        
    }else{
        // first node 
        renderer.head = next_command;
        //renderer.tail = renderer.head;
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

internal inline void
push_rectangle(f32 x, f32 y, f32 width, f32 height, f32 radius, u32 colour = 0xFF00FFFF){
    
    auto* rectangle = make_command(Command_Rectangle);
    rectangle->x = x;
    rectangle->y = y;
    rectangle->width = width;
    rectangle->height = height;
    rectangle->corner_radius = radius;
    rectangle->colour.packed = colour;
    insert_command(rectangle);
}

internal inline void
push_rectangle_outline(f32 x, f32 y, f32 width, f32 height, f32 border, f32 radius, u32 colour = 0xFF00FFFF){
    
    auto rectangle = make_command(Command_Rectangle_Outline);
    rectangle->x = x;
    rectangle->y = y;
    rectangle->width = width;
    rectangle->height = height;
    rectangle->border_size = border;
    rectangle->corner_radius = radius;
    rectangle->colour.packed = colour;
    insert_command(rectangle);
}

internal inline void
push_circle(f32 x, f32 y, f32 radius, u32 colour = 0xFF00FFFF){
    
    auto circle = make_command(Command_Circle);
    circle->x = x;
    circle->y = y;
    circle->radius = radius;
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
    
    auto glyph = make_command(Command_Glyph);
    glyph->x = x0;
    glyph->y = y0;
    glyph->width = x1 - x0;
    glyph->height = y1 - y0;
    glyph->u = s0;
    glyph->v = t0;
    glyph->u_width = s1 - s0;
    glyph->v_height = t1 - t0;
    glyph->colour.packed = colour;
    insert_command(glyph);
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
        result += x1 - x0;
    }
    return result*renderer.fonts[0].scale;
}

internal void
init_opengl_renderer(){
    
    renderer.shape_attribs = 
        subdivide_arena(&platform.temporary_arena, MAX_DRAW*16);
    
    renderer.frame_arena = subdivide_arena(&platform.temporary_arena, 8192);
    
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
            "float dist = box_no_pointy(gl_FragCoord.xy - (out_pos + out_dim/2), out_dim/2, out_radius*min(out_dim.x, out_dim.y)/2);\n"
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
            "float inner = length(max(abs(p)-b*border+r,0.0))-r;\n"
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
    
}

internal void
process_and_draw_commands(){
    
    OPTICK_EVENT();
    int num_rectangle_verts = 0;
    int num_rectangle_outline_verts = 0;
    int num_circle_verts = 0;
    int num_glyph_verts = 0;
    
    Arena* arena = &renderer.shape_attribs;
    f32* rectangle_attribs = (f32*)arena_allocate(arena, MAX_DRAW);
    f32* rectangle_attribs_start = rectangle_attribs;
    
    f32* rectangle_outline_attribs = (f32*)arena_allocate(arena, MAX_DRAW);
    f32* rectangle_outline_attribs_start = rectangle_outline_attribs;
    
    f32* circle_attribs = (f32*)arena_allocate(arena, MAX_DRAW);
    f32* circle_attribs_start = circle_attribs;
    
    f32* glyph_attribs = (f32*)arena_allocate(arena, MAX_DRAW*2);
    f32* glyph_attribs_start = glyph_attribs;
    
    for(Command* command = renderer.head; command; command = command->next){
        switch(command->type){
            case COMMAND_RECTANGLE:{
                auto rectangle = reinterpret_cast<Command_Rectangle*>(command);
                // NOTE(Oliver): there is probably a better way of doing this
                // but we need to upload attributes per vert so we need to 
                // duplicate them in the array
                for(int i = 0; i < 6; i++){
                    *rectangle_attribs++ = rectangle->x;
                    *rectangle_attribs++ = rectangle->y;
                    *rectangle_attribs++ = rectangle->width;
                    *rectangle_attribs++ = rectangle->height;
                    *rectangle_attribs++ = rectangle->corner_radius;
                    *rectangle_attribs++ = rectangle->colour.a/255.0f;
                    *rectangle_attribs++ = rectangle->colour.b/255.0f;
                    *rectangle_attribs++ = rectangle->colour.g/255.0f;
                    *rectangle_attribs++ = rectangle->colour.r/255.0f;
                }
                
                num_rectangle_verts += 6;
            }break;
            
            case COMMAND_RECTANGLE_OUTLINE:{
                auto rectangle = reinterpret_cast<Command_Rectangle_Outline*>(command); 
                
                for(int i = 0; i < 6; i++){
                    *rectangle_outline_attribs++ = rectangle->x;
                    *rectangle_outline_attribs++ = rectangle->y;
                    *rectangle_outline_attribs++ = rectangle->width;
                    *rectangle_outline_attribs++ = rectangle->height;
                    *rectangle_outline_attribs++ = rectangle->border_size;
                    *rectangle_outline_attribs++ = rectangle->corner_radius;
                    *rectangle_outline_attribs++ = rectangle->colour.a/255.0f;
                    *rectangle_outline_attribs++ = rectangle->colour.b/255.0f;
                    *rectangle_outline_attribs++ = rectangle->colour.g/255.0f;
                    *rectangle_outline_attribs++ = rectangle->colour.r/255.0f;
                }
                num_rectangle_outline_verts += 6;
            }break;
            
            case COMMAND_CIRCLE:{
                auto circle = reinterpret_cast<Command_Circle*>(command); 
                
                for(int i = 0; i < 6; i++){
                    *circle_attribs++ = circle->x;
                    *circle_attribs++ = circle->y;
                    *circle_attribs++ = circle->radius;
                    *circle_attribs++ = circle->colour.a/255.0f;
                    *circle_attribs++ = circle->colour.b/255.0f;
                    *circle_attribs++ = circle->colour.g/255.0f;
                    *circle_attribs++ = circle->colour.r/255.0f;
                }
                num_circle_verts += 6;
            }break;
            
            case COMMAND_GLYPH: {
                auto glyph = reinterpret_cast<Command_Glyph*>(command);
                for(int i = 0; i < 6; i++){
                    *glyph_attribs++ = glyph->x;
                    *glyph_attribs++ = glyph->y;
                    *glyph_attribs++ = glyph->width;
                    *glyph_attribs++ = glyph->height;
                    *glyph_attribs++ = glyph->u;
                    *glyph_attribs++ = glyph->v;
                    *glyph_attribs++ = glyph->u_width;
                    *glyph_attribs++ = glyph->v_height;
                    *glyph_attribs++ = glyph->colour.a/255.0f;
                    *glyph_attribs++ = glyph->colour.b/255.0f;
                    *glyph_attribs++ = glyph->colour.g/255.0f;
                    *glyph_attribs++ = glyph->colour.r/255.0f;
                    
                }
                num_glyph_verts += 6;
            }break;
        }
    }
    // NOTE(Oliver): draw filled rects data
    {
        glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[COMMAND_RECTANGLE]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 
                        MAX_DRAW, 
                        rectangle_attribs_start);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glUseProgram(renderer.programs[COMMAND_RECTANGLE]);
        
        float resolution[2] = {platform.width, platform.height};
        glUniform2fv(renderer.resolution_uniforms[COMMAND_RECTANGLE], 1, resolution);
        
        glBindVertexArray(renderer.vaos[COMMAND_RECTANGLE]);
        glDrawArrays(GL_TRIANGLES, 0, num_rectangle_verts);
        glUseProgram(0);
    }
    
    // NOTE(Oliver): draw rectangle outlines
    {
        glBindBuffer(GL_ARRAY_BUFFER, get_buffer_rectangle_outline());
        glBufferSubData(GL_ARRAY_BUFFER, 0, 
                        MAX_DRAW,
                        rectangle_outline_attribs_start);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glUseProgram(get_program_rectangle_outline());
        
        float resolution[2] = {platform.width, platform.height};
        glUniform2fv(renderer.resolution_uniforms[COMMAND_RECTANGLE_OUTLINE], 1, resolution);
        
        glBindVertexArray(get_vao_rectangle_outline());
        glDrawArrays(GL_TRIANGLES, 0, num_rectangle_outline_verts);
        glUseProgram(0);
    }
    
    // NOTE(Oliver): draw circle data
    {
        glBindBuffer(GL_ARRAY_BUFFER, get_buffer_circle());
        glBufferSubData(GL_ARRAY_BUFFER, 0, 
                        MAX_DRAW,
                        circle_attribs_start);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glUseProgram(get_program_circle());
        
        float resolution[2] = {platform.width, platform.height};
        glUniform2fv(renderer.resolution_uniforms[COMMAND_CIRCLE], 1, resolution);
        
        glBindVertexArray(get_vao_circle());
        glDrawArrays(GL_TRIANGLES, 0, num_circle_verts);
        glUseProgram(0);
    }
    // NOTE(Oliver): draw glyph data
    {
        glBindBuffer(GL_ARRAY_BUFFER, get_buffer_glyph());
        glBufferSubData(GL_ARRAY_BUFFER, 0, 
                        MAX_DRAW*10,
                        glyph_attribs_start);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glUseProgram(get_program_glyph());
        
        float resolution[2] = {platform.width, platform.height};
        glUniform2fv(renderer.resolution_uniforms[COMMAND_GLYPH], 1, resolution);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer.texture);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glBindVertexArray(get_vao_glyph());
        glDrawArrays(GL_TRIANGLES, 0, num_glyph_verts);
        glUseProgram(0);
    }
}


internal void
opengl_start_frame() {
    OPTICK_EVENT();
    
    arena_reset(&renderer.commands);
    arena_reset(&renderer.shape_attribs);
    
    renderer.head = nullptr;
    renderer.tail = nullptr;
    
    friday.x = 640;
    friday.y = 360;
    friday.x_offset = 0;
    
}

internal void
opengl_end_frame() {
    OPTICK_EVENT();
    
    
    glViewport(0, 0, platform.width, platform.height);
    glClearColor(theme.base_margin.r/255.0f,
                 theme.base_margin.g/255.0f,
                 theme.base_margin.b/255.0f,
                 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    process_and_draw_commands();
    
    
    glUseProgram(0);
}

internal void
new_line(){
    friday.y -= renderer.fonts[0].line_height;
    friday.x_offset = 0;
}

internal void
indent(){
    friday.x_offset += 40;
}

internal void
draw_string(char* string, u32 colour = 0xFFFFFFFF){
    push_string(get_friday_x(),get_friday_y(), string, colour);
    friday.x_offset += get_text_width(string);
}

internal void
draw_string(String8 string, u32 colour = 0xFFFFFFFF){
    push_string8(get_friday_x(), get_friday_y(), string, colour);
    friday.x_offset += get_text_width(string);
}

internal bool
is_mouse_in_rect(f32 x, f32 y, f32 width, f32 height){
    return platform.mouse_x <= (x + width) && platform.mouse_x >= x &&
        platform.mouse_y <= (y + height) && platform.mouse_y >= y;
}

internal void
draw_leaf(Node* leaf){
    f32 width = get_text_width(leaf->name);
    f32 offset = 5.0f;
    width += 2*offset;
    f32 line_height = renderer.fonts[0].line_height;
    f32 height = line_height;
    f32 x = get_friday_x() - offset;
    f32 y = get_friday_y() - line_height*0.25;
    
    u32 text_colour = theme.text.packed;
    
    if(is_mouse_in_rect(x, y, width, height)){
        push_rectangle(x, y, width, height, 0.2, theme.cursor.packed);
        text_colour = theme.base.packed;
    }
    
    draw_string(leaf->name, text_colour);
    
}

internal void
draw_misc(char* string){
    draw_string(string, theme.text_light.packed);
}

internal void
draw_misc(String8 string){
    draw_string(string, theme.text_light.packed);
}

internal void
render_graph(Node* root){
    if(!root) return;
    switch(root->type){
        
        case NODE_BINARY:{
            auto binary = &root->binary;
            render_graph(binary->left);
            
            String8 op = {};
            Arena* arena = &renderer.frame_arena;
            switch(binary->op_type){
                case OP_PLUS: op = make_string(arena, " + ");break;
                case OP_MINUS: op = make_string(arena, " - ");break;
                case OP_DIVIDE: op = make_string(arena, " / ");break;
                case OP_MULTIPLY: op = make_string(arena, " * ");break;
            }
            draw_misc(op);
            
            render_graph(binary->right);
        }break;
        
        case NODE_LITERAL:{
            Arena* arena = &renderer.frame_arena;
            
            auto literal = &root->literal;
            char* string = (char*)arena_allocate(arena, 256);
            snprintf(string, 256, "%d", literal->_int);
            draw_string(string);
        }break;
        
        case NODE_STRUCT: {
            auto _struct = &root->_struct;
            draw_leaf(root);
            draw_misc(" :: struct {");
            new_line();
            indent();
            for(Node* member = _struct->members; member; member = member->next){
                render_graph(member);
            }
            new_line();
            draw_misc("}");
            new_line();
        }break;
        
        case NODE_DECLARATION: {
            auto decl = root->declaration.declaration;
            draw_leaf(root);
            //draw_string(root->name);
            draw_misc(" :");
            static f32 width = 0.0f;
            if(is_mouse_in_rect(friday.x + friday.x_offset-20, friday.y-20, 40, 40)){
                width = exp(1.1f + width);
                width = width > 20 ? 20 : width;
                if(platform.mouse_left_clicked && !root->declaration.type_usage){
                    Pool* pool = &friday.node_pool;
                    root->declaration.type_usage = make_node(pool, NODE_TYPE_USAGE);
                    root->declaration.type_usage->name = 
                        make_string(&platform.permanent_arena, "integer");
                }
            }else {
                width = exp(-width);
                width = width < 0 ? 0 : width;
            }
            render_graph(root->declaration.type_usage);
            friday.x += width;
            draw_misc("= ");
            render_graph(decl);
            draw_misc(";");
        }break;
        
        case NODE_SCOPE: {
            auto scope = &root->scope;
            for(Node* stmt = scope->statements; stmt; stmt = stmt->next){
                render_graph(stmt);
            }
        }break;
        
        case NODE_FUNCTION: {
        }break;
        
        case NODE_CALL: {
            
        }break;
        
        case NODE_TYPE_USAGE: {
            draw_leaf(root);
        }break;
    }
}
