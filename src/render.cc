
enum Command_Type {
    COMMAND_RECTANGLE,
    COMMAND_CIRCLE,
    COMMAND_TEXT,
    COMMAND_RECTANGLE_OUTLINE,
    
    COMMAND_COUNT
};

// NOTE(Oliver): Maybe a linked list for this would be better
struct Command {
    Command_Type type;
    u32 colour;
    Command* previous = nullptr;
    Command* next = nullptr;
};

struct Command_Rectangle: Command {
    Command_Rectangle() { type = COMMAND_RECTANGLE; }
#define BYTES_PER_RECTANGLE (5*sizeof(f32))
    f32 x, y;
    f32 width, height;
    f32 corner_radius;
};

struct Command_Circle : Command {
    Command_Circle() { type = COMMAND_CIRCLE; }
#define BYTES_PER_CIRCLE (3*sizeof(f32))
    f32 x, y;
    f32 radius;
};

struct Command_Rectangle_Outline : Command {
    Command_Rectangle_Outline() { type = COMMAND_RECTANGLE_OUTLINE; }
#define BYTES_PER_RECTANGLE_OUTLINE (6*sizeof(f32))
    f32 x, y;
    f32 width, height;
    f32 border_size;
    f32 corner_radius;
};


const int MAX_DRAW = 8192;

global struct {
    
    GLuint vaos[COMMAND_COUNT];
    GLuint buffers[COMMAND_COUNT];
    GLuint programs[COMMAND_COUNT];
    
    GLuint ortho_uniform;
    GLuint view_uniform;
    GLuint resolution_uniform;
    // NOTE(Oliver): this should probably just be static
    // we do have a max draw value anyway
    //Array<Command*> commands;
    Memory_Pool commands;
    Command* head = nullptr;
    Command* tail = nullptr;
    
    Array<f32> rectangle_attribs;
    Array<f32> rectangle_outline_attribs;
    Array<f32> circle_attribs;
    Array<f32> text_attribs;
    
} renderer;

#define make_command(type) (type*) new (renderer.commands.allocate(sizeof(type))) type()

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
get_vao_text() {
    return renderer.vaos[COMMAND_TEXT];
}

internal inline GLuint
get_buffer_text() {
    return renderer.buffers[COMMAND_TEXT];
}

internal inline GLuint
get_program_text() {
    return renderer.programs[COMMAND_TEXT];
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
push_rectangle(f32 x, f32 y, f32 width, f32 height, f32 radius){
    
    auto* rectangle = make_command(Command_Rectangle);
    rectangle->x = x;
    rectangle->y = y;
    rectangle->width = width;
    rectangle->height = height;
    rectangle->corner_radius = radius;
    insert_command(rectangle);
}

internal inline void
push_rectangle_outline(f32 x, f32 y, f32 width, f32 height, f32 border, f32 radius){
    
    auto rectangle = make_command(Command_Rectangle_Outline);
    rectangle->x = x;
    rectangle->y = y;
    rectangle->width = width;
    rectangle->height = height;
    rectangle->border_size = border;
    rectangle->corner_radius = radius;
    insert_command(rectangle);
}

internal inline void
push_circle(f32 x, f32 y, f32 radius){
    
    auto circle = make_command(Command_Circle);
    circle->x = x;
    circle->y = y;
    circle->radius = radius;
    insert_command(circle);
}

internal void
init_opengl_renderer(){
    
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
        
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, 2, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE, reinterpret_cast<void*>(0));
        
        glEnableVertexAttribArray(dim);
        glVertexAttribPointer(dim, 2, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE, reinterpret_cast<void*>(sizeof(f32)*2));
        
        glEnableVertexAttribArray(radius);
        glVertexAttribPointer(radius, 1, GL_FLOAT, false, 
                              BYTES_PER_RECTANGLE, reinterpret_cast<void*>(sizeof(f32)*4));
        
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
        
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, 2, GL_FLOAT, false, 
                              BYTES_PER_CIRCLE, reinterpret_cast<void*>(0));
        
        glEnableVertexAttribArray(radius);
        glVertexAttribPointer(radius, 1, GL_FLOAT, false, 
                              BYTES_PER_CIRCLE, reinterpret_cast<void*>(sizeof(f32)*2));
        
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
            "uniform mat4x4 ortho;\n"
            "uniform mat4x4 view;\n"
            "uniform vec2 resolution;\n"
            "out vec2 out_pos;\n"
            "out vec2 out_dim;\n"
            "out float out_radius;\n"
            
            "void main(){\n"
            "vec2 vertices[] = vec2[](vec2(-1, -1), vec2(1,-1), vec2(1,1),\n"
            "vec2(-1,-1), vec2(-1, 1), vec2(1, 1));"
            "vec4 screen_position = vec4(vertices[(gl_VertexID % 6)], 0, 1);\n"
            "screen_position.xy *= (vec4(dim/resolution, 0, 1)).xy;\n"
            "screen_position.xy += 2*(vec4((pos+dim/2)/resolution,0,1)).xy -1;\n"
            "gl_Position = screen_position;\n"
            "out_pos = pos;\n"
            "out_radius = radius;\n"
            "out_dim = dim;\n"
            "}\n";
        
        GLchar* rectangle_fs =  
            "#version 330 core\n"
            "in vec2 out_pos; \n"
            "in vec2 out_dim; \n"
            "in float out_radius; \n"
            "out vec4 colour;\n"
            "uniform vec2 in_position;\n"
            
            "float box_no_pointy(vec2 p, vec2 b, float r){\n"
            "return length(max(abs(p)-b+r,0.0))-r;\n"
            "}\n"
            
            "void main(){\n"
            "float dist = box_no_pointy(gl_FragCoord.xy - (out_pos + out_dim/2), out_dim/2, out_radius*min(out_dim.x, out_dim.y)/2);\n"
            "float alpha = mix(1, 0,  smoothstep(0, 2, dist));\n"
            "vec3 debug_colour = mix(vec3(1,0,0), vec3(0,1,0), smoothstep(0, 1, dist));\n"
            "colour = vec4(1,0,0, alpha);\n"
            "}\n";
        
        GLuint program = make_program(rectangle_vs, rectangle_fs);
        
        renderer.resolution_uniform = glGetUniformLocation(program, "resolution");
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
            "uniform mat4x4 ortho;\n"
            "uniform mat4x4 view;\n"
            "uniform vec2 resolution;\n"
            "out vec2 out_pos;\n"
            "out vec2 out_dim;\n"
            "out float out_radius;\n"
            "out float out_border;\n"
            "out vec2 out_res;\n"
            
            "void main(){\n"
            "vec2 vertices[] = vec2[](vec2(-1, -1), vec2(1,-1), vec2(1,1),\n"
            "vec2(-1,-1), vec2(-1, 1), vec2(1, 1));"
            "vec4 screen_position = vec4(vertices[(gl_VertexID % 6)], 0, 1);\n"
            "screen_position.xy *= (vec4(dim/resolution, 0, 1)).xy;\n"
            "screen_position.xy += 2*(vec4((pos+dim/2)/resolution,0,1)).xy -1;\n"
            "gl_Position = screen_position;\n"
            "out_pos = pos;\n"
            "out_radius = radius;\n"
            "out_dim = dim;\n"
            "out_border = border_size;\n"
            "out_res = resolution;\n"
            "}\n";
        
        GLchar* rectangle_fs =  
            "#version 330 core\n"
            "in vec2 out_pos; \n"
            "in vec2 out_dim; \n"
            "in float out_radius; \n"
            "in float out_border; \n"
            "in vec2 out_res; \n"
            "out vec4 colour;\n"
            "uniform vec2 in_position;\n"
            
            "float box_no_pointy(vec2 p, vec2 b, float r, float border){\n"
            "float inner = length(max(abs(p)-b*border+r*border,0.0))-r*border;\n"
            "float outer = length(max(abs(p)-b+r,0.0))-r;\n"
            "return max(-inner, outer);\n"
            "}\n"
            "float box_dist(vec2 p, vec2 size, float radius, float border){\n"
            "size -= vec2(radius);\n"
            "vec2 d = abs(p) - size;\n"
            "return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - radius;\n"
            "}\n"
            "void main(){\n"
            "float dist = box_no_pointy(gl_FragCoord.xy - (out_pos + out_dim/2), out_dim/2, 5 + 0*out_radius*min(out_dim.x, out_dim.y)/2, 0.3);\n"
            "if(dist <= 0.0001) { dist = 0.0001; }\n"
            "float alpha = mix(1, 0,  smoothstep(0, 2, dist));\n"
            "vec3 debug_colour = mix(vec3(1,0,0), vec3(0,1,0), smoothstep(0, 1, dist));\n"
            "colour = vec4(0, 1, 0, alpha);\n"
            "}\n";
        
        GLuint program = make_program(rectangle_vs, rectangle_fs);
        
        renderer.resolution_uniform = glGetUniformLocation(program, "resolution");
        
        renderer.programs[COMMAND_RECTANGLE_OUTLINE] = program;
        
    }
    
    // NOTE(Oliver): init rectangle outline shader
    {
        GLchar* circle_vs =  
            "#version 330 core\n"
            "layout(location = 0) in vec2 pos; \n"
            "layout(location = 2) in float radius; \n"
            "uniform mat4x4 ortho;\n"
            "uniform mat4x4 view;\n"
            "uniform vec2 resolution;\n"
            "out vec2 out_pos;\n"
            "out float out_radius;\n"
            "out vec2 out_res;\n"
            
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
            "}\n";
        
        GLchar* circle_fs =  
            "#version 330 core\n"
            "in vec2 out_pos; \n"
            "in float out_radius; \n"
            "in float out_border; \n"
            "in vec2 out_res; \n"
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
            "colour = vec4(0, 1, 0, alpha);\n"
            "}\n";
        
        GLuint program = make_program(circle_vs, circle_fs);
        
        renderer.resolution_uniform = glGetUniformLocation(program, "resolution");
        
        renderer.programs[COMMAND_CIRCLE] = program;
        
    }
    
}

internal void
opengl_start_frame() {
    renderer.commands.reset();
    renderer.rectangle_attribs.reset();
    renderer.rectangle_outline_attribs.reset();
    renderer.circle_attribs.reset();
    renderer.head = nullptr;
    renderer.tail = nullptr;
}

internal inline void
push_rectangle_attribs(f32 attribs){
    renderer.rectangle_attribs.insert(attribs);
}

internal inline void
push_rectangle_outline_attribs(f32 attribs){
    renderer.rectangle_outline_attribs.insert(attribs);
}

internal inline void
push_circle_attribs(f32 attribs){
    renderer.circle_attribs.insert(attribs);
}

internal void
process_and_draw_commands(){
    int num_rectangle_verts = 0;
    int num_rectangle_outline_verts = 0;
    int num_circle_verts = 0;
    
    for(Command* command = renderer.head; command; command = command->next){
        switch(command->type){
            case COMMAND_RECTANGLE:{
                auto rectangle = reinterpret_cast<Command_Rectangle*>(command);
                
                // NOTE(Oliver): there is probably a better way of doing this
                // but we need to upload attributes per vert so we need to 
                // duplicate them in the array
                for(int i = 0; i < 6; i++){
                    push_rectangle_attribs(rectangle->x);
                    push_rectangle_attribs(rectangle->y);
                    push_rectangle_attribs(rectangle->width);
                    push_rectangle_attribs(rectangle->height);
                    push_rectangle_attribs(rectangle->corner_radius);
                }
                num_rectangle_verts += 6;
            }break;
            
            case COMMAND_RECTANGLE_OUTLINE:{
                auto rectangle = reinterpret_cast<Command_Rectangle_Outline*>(command); 
                
                for(int i = 0; i < 6; i++){
                    push_rectangle_outline_attribs(rectangle->x);
                    push_rectangle_outline_attribs(rectangle->y);
                    push_rectangle_outline_attribs(rectangle->width);
                    push_rectangle_outline_attribs(rectangle->height);
                    push_rectangle_outline_attribs(rectangle->border_size);
                    push_rectangle_outline_attribs(rectangle->corner_radius);
                }
                num_rectangle_outline_verts += 6;
            }break;
            
            case COMMAND_CIRCLE:{
                auto circle = reinterpret_cast<Command_Circle*>(command); 
                
                for(int i = 0; i < 6; i++){
                    push_circle_attribs(circle->x);
                    push_circle_attribs(circle->y);
                    push_circle_attribs(circle->radius);
                }
                num_circle_verts += 6;
            }break;
            
        }
    }
    
    // NOTE(Oliver): draw filled rects data
    {
        glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[COMMAND_RECTANGLE]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 
                        renderer.rectangle_attribs.bytes_used(), 
                        renderer.rectangle_attribs.data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glUseProgram(renderer.programs[COMMAND_RECTANGLE]);
        
        float resolution[2] = {platform.width, platform.height};
        glUniform2fv(renderer.resolution_uniform, 1, resolution);
        
        mat4x4 projection = ortho(0, platform.width, 0, platform.height);
        mat4x4 view = translate(0, 0);
        
        glUniformMatrix4fv(renderer.ortho_uniform, 1, GL_TRUE, projection.e);
        glUniformMatrix4fv(renderer.view_uniform, 1, GL_TRUE, view.e);
        
        glBindVertexArray(renderer.vaos[COMMAND_RECTANGLE]);
        glDrawArrays(GL_TRIANGLES, 0, num_rectangle_verts);
        glUseProgram(0);
    }
    
    // NOTE(Oliver): draw rectangle outlines
    {
        glBindBuffer(GL_ARRAY_BUFFER, get_buffer_rectangle_outline());
        glBufferSubData(GL_ARRAY_BUFFER, 0, 
                        renderer.rectangle_outline_attribs.bytes_used(), 
                        renderer.rectangle_outline_attribs.data);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glUseProgram(get_program_rectangle_outline());
        
        float resolution[2] = {platform.width, platform.height};
        glUniform2fv(renderer.resolution_uniform, 1, resolution);
        
        mat4x4 projection = ortho(0, platform.width, 0, platform.height);
        mat4x4 view = translate(0, 0);
        
        glUniformMatrix4fv(renderer.ortho_uniform, 1, GL_TRUE, projection.e);
        glUniformMatrix4fv(renderer.view_uniform, 1, GL_TRUE, view.e);
        
        glBindVertexArray(get_vao_rectangle_outline());
        glDrawArrays(GL_TRIANGLES, 0, num_rectangle_outline_verts);
        glUseProgram(0);
    }
    
    // NOTE(Oliver): draw circle data
    {
        glBindBuffer(GL_ARRAY_BUFFER, get_buffer_circle());
        glBufferSubData(GL_ARRAY_BUFFER, 0, 
                        renderer.circle_attribs.bytes_used(), 
                        renderer.circle_attribs.data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glUseProgram(get_program_circle());
        
        float resolution[2] = {platform.width, platform.height};
        glUniform2fv(renderer.resolution_uniform, 1, resolution);
        
        mat4x4 projection = ortho(0, platform.width, 0, platform.height);
        mat4x4 view = translate(0, 0);
        
        glUniformMatrix4fv(renderer.ortho_uniform, 1, GL_TRUE, projection.e);
        glUniformMatrix4fv(renderer.view_uniform, 1, GL_TRUE, view.e);
        
        glBindVertexArray(get_vao_circle());
        glDrawArrays(GL_TRIANGLES, 0, num_circle_verts);
        glUseProgram(0);
    }
    
}

internal void
opengl_end_frame() {
    glBindVertexArray(renderer.vaos[COMMAND_RECTANGLE]);
    
    glViewport(0, 0, platform.width, platform.height);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    process_and_draw_commands();
    
    
    glUseProgram(0);
}
