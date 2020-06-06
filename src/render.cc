
enum Command_Type {
    COMMAND_RECTANGLE,
    COMMAND_CIRCLE,
    COMMAND_RECTANGLE_OUTLINE,
    
    COMMAND_COUNT
};

// NOTE(Oliver): Maybe a linked list for this would be better
struct Command {
    Command_Type type;
    u32 colour;
};

struct Command_Rectangle: Command {
    Command_Rectangle() { type = COMMAND_RECTANGLE; }
    f32 x, y;
    f32 width, height;
    f32 corner_radius;
};

struct Command_Circle : Command {
    Command_Circle() { type = COMMAND_CIRCLE; }
    s64 x, y;
    u64 radius;
};

struct Command_Rectangle_Outline : Command {
    Command_Rectangle_Outline() { type = COMMAND_RECTANGLE_OUTLINE; }
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
    Array<Command*> commands;
    Array<f32> frame_data;
    
} renderer;

internal inline void
push_rectangle(f32 x, f32 y, f32 width, f32 height, f32 radius){
    // TODO(Oliver): this is straight up leaking memory fix it asap
    Command_Rectangle* rectangle = new Command_Rectangle;
    rectangle->x = x;
    rectangle->y = y;
    rectangle->width = width;
    rectangle->height = height;
    rectangle->corner_radius = radius;
    
    renderer.commands.insert(rectangle);
}

internal void
init_opengl_renderer(){
    
    glGenVertexArrays(1, &renderer.vaos[COMMAND_RECTANGLE]);
    glBindVertexArray(renderer.vaos[COMMAND_RECTANGLE]);
    
    glGenBuffers(1, &renderer.buffers[COMMAND_RECTANGLE]);
    glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[COMMAND_RECTANGLE]);
    
    // NOTE(Oliver): yeah maybe sort these constants out, looks wacko
    glBufferData(GL_ARRAY_BUFFER, MAX_DRAW*sizeof(f32)*5, 0, GL_DYNAMIC_DRAW);
    
    GLuint pos = 0;
    GLuint dim = 2;
    GLuint radius = 3;
    
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(pos, 2, GL_FLOAT, false, 
                          sizeof(f32)*5, reinterpret_cast<void*>(0));
    
    glEnableVertexAttribArray(dim);
    glVertexAttribPointer(dim, 2, GL_FLOAT, false, 
                          sizeof(f32)*5, reinterpret_cast<void*>(sizeof(f32)*2));
    
    glEnableVertexAttribArray(radius);
    glVertexAttribPointer(radius, 1, GL_FLOAT, false, 
                          sizeof(f32)*5, reinterpret_cast<void*>(sizeof(f32)*4));
    
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
            "vec4 screen_position = vec4(vertices[gl_VertexID], 0, 1);\n"
            "screen_position.xy *= (ortho*vec4(dim, 0, 1)).xy;\n"
            "screen_position.xy += (ortho*vec4(pos,0,1)).xy;\n"
            "gl_Position = screen_position;\n"
            //"gl_Position = ortho*view*vec4(pos, 0, 1);\n"
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
            "float dist = box_no_pointy(gl_FragCoord.xy - (out_pos + out_dim/2), out_dim/2, 10);\n"
            "float alpha = mix(1, 0,  smoothstep(0, 1, dist));\n"
            "vec3 debug_colour = mix(vec3(1,0,0), vec3(0,1,0), smoothstep(0, 1, dist));\n"
            "colour = vec4(debug_colour, 1);\n"
            "}\n";
        
        GLuint program = make_program(rectangle_vs, rectangle_fs);
        
        renderer.resolution_uniform = glGetUniformLocation(program, "resolution");
        renderer.ortho_uniform = glGetUniformLocation(program, "ortho");
        renderer.view_uniform = glGetUniformLocation(program, "view");
        
        renderer.programs[COMMAND_RECTANGLE] = program;
        
    }
}

internal void
opengl_start_frame() {
    renderer.commands.reset();
    renderer.frame_data.reset();
}

internal inline void
push_frame_data(f32 data){
    renderer.frame_data.insert(data);
}

internal void
process_and_draw_commands(){
    for(int i = 0; i < renderer.commands.size; i++){
        Command* command = renderer.commands[i];
        switch(command->type){
            case COMMAND_RECTANGLE:{
                auto rectangle = reinterpret_cast<Command_Rectangle*>(command);
                
                for(int i = 0; i < 6; i++){
                    push_frame_data(rectangle->x);
                    push_frame_data(rectangle->y);
                    push_frame_data(rectangle->width);
                    push_frame_data(rectangle->height);
                    push_frame_data(rectangle->corner_radius);
                }
                glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[COMMAND_RECTANGLE]);
                glBufferSubData(GL_ARRAY_BUFFER, 0, renderer.frame_data.bytes_used(), renderer.frame_data.data);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                
                glUseProgram(renderer.programs[COMMAND_RECTANGLE]);
                
                float resolution[2] = {platform.width, platform.height};
                glUniform2fv(renderer.resolution_uniform, 1, resolution);
                
                mat4x4 projection = ortho(0, platform.width, 0, platform.height);
                mat4x4 view = translate(0, 0);
                
                glUniformMatrix4fv(renderer.ortho_uniform, 1, GL_TRUE, projection.e);
                glUniformMatrix4fv(renderer.view_uniform, 1, GL_TRUE, view.e);
                
                glBindVertexArray(renderer.vaos[COMMAND_RECTANGLE]);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glUseProgram(0);
            }break;
        }
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
