
// NOTE(Oliver): this is global state for the render pass
// may not be needed, we'll see

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

internal SDFFont
load_sdf_font(char* filename){
    
    char* buffer = 0;
    
    u64 file_size = 0;
    platform->load_file(&platform->permanent_arena, string_from_cstr(filename),
                        (void**)&buffer, &file_size);
    
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
        String8  token = read_token(&l);
        String8 filename = make_stringf(&platform->permanent_arena, "../fonts/%.*s", token.length, token.text);
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
    font.line_height = line_height*1.2f;
    font.scale = 22.0f/(f32)font.size;
    font.padding = padding;
    return font;
}


internal int
get_font_line_height( f32 font_scale = 1.0f) {
    font_scale *= renderer->font.scale;
    return renderer->font.line_height*font_scale;
}

internal Command*
make_command(Command_Type type){
    Command* command = push_type(&platform->frame_arena, Command);
    command->type = type;
    command->next = nullptr;
    command->previous = nullptr;
    return command;
}

internal void
insert_command( Command* next_command){
    
    if(!renderer->head){
        renderer->head = next_command;
        renderer->tail = renderer->head;
    }else {
        renderer->tail->next = next_command;
        renderer->tail = renderer->tail->next;
    }
}

internal void
push_clip_range_begin( v4f rect){
    auto clip_range = make_command(COMMAND_CLIP_RANGE_BEGIN);
    clip_range->clip_range.x = rect.x;
    clip_range->clip_range.y = rect.y;
    clip_range->clip_range.width = rect.width;
    clip_range->clip_range.height = rect.height;
    insert_command(clip_range);
}

internal inline void
push_clip_range_end(){
    auto clip_range = make_command(COMMAND_CLIP_RANGE_END);
    insert_command(clip_range);
}

#define RENDER_CLIP(rect) defer_loop(push_clip_range_begin(rect), push_clip_range_end())

internal inline void
push_rectangle( v4f rect, f32 radius, Colour colour){
    
    auto rectangle = make_command(COMMAND_RECTANGLE);
    rectangle->rectangle.x = rect.x;
    rectangle->rectangle.y = rect.y;
    rectangle->rectangle.width = rect.width;
    rectangle->rectangle.height = rect.height;
    rectangle->rectangle.corner_radius = radius;
    rectangle->colour = colour;
    insert_command(rectangle);
}

internal inline void
push_triangle( v2f pos, f32 size, f32 rotation, Colour colour){
    
    auto triangle = make_command(COMMAND_TRIANGLE);
    triangle->triangle.x = pos.x;
    triangle->triangle.y = pos.y;
    triangle->triangle.size = sqrt(size*2.0)/2.0;
    triangle->triangle.rotation = rotation;
    triangle->colour = colour;
    insert_command(triangle);
}

internal inline void
push_rectangle_outline( v4f rect, f32 border, f32 radius, Colour colour = {0xFF00FFFF}){
    
    auto rectangle = make_command(COMMAND_RECTANGLE_OUTLINE);
    rectangle->rectangle_outline.x = rect.x;
    rectangle->rectangle_outline.y = rect.y;
    rectangle->rectangle_outline.width = rect.width;
    rectangle->rectangle_outline.height = rect.height;
    rectangle->rectangle_outline.border_size = border;
    rectangle->rectangle_outline.corner_radius = radius;
    rectangle->colour = colour;
    insert_command(rectangle);
}

internal inline void
push_circle( v2f pos, f32 radius, Colour colour = {0xFF00FFFF}){
    
    auto circle = make_command(COMMAND_CIRCLE);
    circle->circle.x = pos.x;
    circle->circle.y = pos.y;
    circle->circle.radius = radius;
    circle->colour = colour;
    insert_command(circle);
}


internal void
push_glyph( v4f positions, v4f uvs, Colour colour){
    
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
    glyph->colour = colour;
    insert_command(glyph);
}

internal inline void
push_rectangle_textured( v4f rect, f32 radius, Bitmap bitmap){
    
    auto rectangle = make_command(COMMAND_RECTANGLE_TEXTURED);
    rectangle->rectangle_textured.x = rect.x;
    rectangle->rectangle_textured.y = rect.y;
    rectangle->rectangle_textured.width = rect.width;
    rectangle->rectangle_textured.height = rect.height;
    rectangle->rectangle_textured.corner_radius = radius;
    rectangle->rectangle_textured.u = 0;
    rectangle->rectangle_textured.v = 0;
    rectangle->rectangle_textured.u_width = rect.width;
    rectangle->rectangle_textured.v_height = rect.height;
    rectangle->rectangle_textured.bitmap = bitmap;
    insert_command(rectangle);
}


internal inline void
push_bezier(v2f v0, v2f v1, v2f v2, f32 thickness, Colour colour = {0xFF00FFFF}){
    
    auto bezier = make_command(COMMAND_BEZIER);
    bezier->bezier.thickness = thickness;
    bezier->bezier.v0 = v0;
    bezier->bezier.v1 = v1;
    bezier->bezier.v2 = v2;
    bezier->colour = colour;
    insert_command(bezier);
}

internal inline void
push_line(v2f v0, v2f v1, f32 thickness=1.0, Colour colour = {0xFF00FFFF}){
    
    auto bezier = make_command(COMMAND_BEZIER);
    bezier->bezier.thickness = thickness;
    bezier->bezier.v0 = v0;
    bezier->bezier.v1 = v0 + v1/2.0;
    bezier->bezier.v2 = v1;
    bezier->colour = colour;
    insert_command(bezier);
}

internal void
push_string(v2f pos, String8 string, Colour colour, f32 font_scale = 1.0f){
    
    pos.y = -pos.y;
    pos.y -= get_font_line_height(font_scale);
    font_scale *= renderer->font.scale;
    
    // NOTE(Oliver): '#' is used for ID purposes
    for(int i = 0; i < string.length; i++){
        char text = string.text[i];
        if(text == '#'){break;}
        //while(string.text && *string.text && *string.text != '#'){
        if(text >= 32 && text < 128){
            auto font = renderer->font;
            auto c  = font.characters[text];
            v4f positions = v4f(pos.x + c.x_offset*font_scale, 
                                pos.y + c.y_offset*font_scale, 
                                pos.x + c.x_offset*font_scale + c.width*font_scale,
                                pos.y + c.y_offset*font_scale + c.height*font_scale);
            v4f uvs = v4f(c.x/512.0f, c.y/512.0f, (c.x + c.width)/512.0f, (c.y+c.height)/512.0f);
            push_glyph(positions, uvs, colour);
            pos.x += (c.x_advance - font.padding.x)*font_scale;
        }
    }
}

internal void
push_string(v2f pos, char* text, Colour colour, f32 font_scale = 1.0f){
    push_string(pos, string_from_cstr(text), colour, font_scale);
}

internal void
push_stringi(v2f pos, String8 string, Colour colour, int index, f32 font_scale = 1.0f){
    
    pos.y = -pos.y;
    pos.y -= get_font_line_height(font_scale);
    font_scale *= renderer->font.scale;
    
    // NOTE(Oliver): '#' is used for ID purposes
    for(int i = 0; i < string.length; i++){
        char text = string.text[i];
        if(text == '#'){break;}
        //while(string.text && *string.text && *string.text != '#'){
        if(text >= 32 && text < 128){
            auto font = renderer->font;
            auto c  = font.characters[text];
            v4f positions = v4f(pos.x + c.x_offset*font_scale, 
                                pos.y + c.y_offset*font_scale, 
                                pos.x + c.x_offset*font_scale + c.width*font_scale,
                                pos.y + c.y_offset*font_scale + c.height*font_scale);
            v4f uvs = v4f(c.x/512.0f, c.y/512.0f, (c.x + c.width)/512.0f, (c.y+c.height)/512.0f);
            if(i == index)
                push_glyph(positions, uvs, colour);
            pos.x += (c.x_advance - font.padding.x)*font_scale;
        }
    }
}

internal f32
get_text_width(char* text, f32 font_scale = 1.0f){
    font_scale *= renderer->font.scale;
    f32 result = 0;
    while(text && *text){
        if(*text == '#') break;
        int id = *text;
        result += (renderer->font.characters[id].x_advance-renderer->font.padding.x)*font_scale;
        text++;
    }
    return result;
}

internal f32
get_text_width(String8 string, f32 font_scale = 1.0f){
    font_scale *= renderer->font.scale;
    f32 result = 0;
    for(int i = 0; i < string.length; i++){
        int id = string.text[i];
        result += (renderer->font.characters[id].x_advance-renderer->font.padding.x)*font_scale;
    }
    return result;
}

internal f32
get_text_width_n(char* text, int n, f32 font_scale = 1.0f){
    font_scale *= renderer->font.scale;
    f32 result = 0;
    int i = 0;
    while(text && *text && i < n){
        if(*text == '#') break;
        int id = *text;
        result += (renderer->font.characters[id].x_advance-renderer->font.padding.x)*font_scale;
        text++;
        i++;
    }
    return result;
}

internal f32
get_text_width_n(String8 string, int n, f32 font_scale = 1.0f){
    font_scale *= renderer->font.scale;
    f32 result = 0;
    for(int i = 0; (i < string.length) &&  i < n; i++){
        int id = string.text[i];
        result += (renderer->font.characters[id].x_advance-renderer->font.padding.x)*font_scale;
    }
    return result;
}

internal v4f
get_text_bbox(v2f pos, String8 string, f32 font_scale = 1.0f, f32 border = 5.0f){
    f32 width = get_text_width(string, font_scale) + border*2;
    f32 height = get_font_line_height(font_scale);
    pos.x -= border;
    return v4f(pos.x, pos.y, width, height);
}

internal v4f
get_text_bbox(v2f pos, char* string, f32 font_scale = 1.0f, f32 border = 5.0f){
    f32 width = get_text_width(string, font_scale) + border*2;
    f32 height = get_font_line_height(font_scale);
    pos.x -= border;
    return v4f(pos.x, pos.y, width, height);
}

internal v2f
get_text_size(char* string, f32 font_scale = 1.0f){
    v2f size = {};
    size.width = get_text_width(string, font_scale);
    size.height = get_font_line_height(font_scale);
    return size;
}

internal v2f
get_text_size(String8 string, f32 font_scale = 1.0f){
    v2f size = {};
    size.width = get_text_width(string, font_scale);
    size.height = get_font_line_height(font_scale);
    return size;
}

internal f32
text_scale_from_pixels(String8 string, f32 pixels){
    v2f size = get_text_size(string);
    f32 scale = (size.width + pixels)/size.width;
    return scale;
}

internal void
init_opengl_renderer(){
    
    platform->frame_arena = subdivide_arena(&platform->frame_arena, Megabytes(1));
    
    
    {
        glGenVertexArrays(1, &renderer->vaos[COMMAND_RECTANGLE]);
        glBindVertexArray(renderer->vaos[COMMAND_RECTANGLE]);
        
        glGenBuffers(1, &renderer->buffers[COMMAND_RECTANGLE]);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_RECTANGLE]);
        
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
        glGenVertexArrays(1, &renderer->vaos[COMMAND_TRIANGLE]);
        glBindVertexArray(renderer->vaos[COMMAND_TRIANGLE]);
        
        glGenBuffers(1, &renderer->buffers[COMMAND_TRIANGLE]);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_TRIANGLE]);
        
        // NOTE(Oliver): yeah maybe sort these constants out, looks wacko
        glBufferData(GL_ARRAY_BUFFER, MAX_DRAW*BYTES_PER_TRIANGLE, 0, GL_DYNAMIC_DRAW);
        
        GLuint pos = 0;
        GLuint dim = 2;
        GLuint rot = 4;
        GLuint colour = 5;
        
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, 2, GL_FLOAT, false, 
                              BYTES_PER_TRIANGLE, reinterpret_cast<void*>(0));
        
        glEnableVertexAttribArray(dim);
        glVertexAttribPointer(dim, 2, GL_FLOAT, false, 
                              BYTES_PER_TRIANGLE, reinterpret_cast<void*>(sizeof(f32)*2));
        
        glEnableVertexAttribArray(rot);
        glVertexAttribPointer(rot, 1, GL_FLOAT, false, 
                              BYTES_PER_TRIANGLE, reinterpret_cast<void*>(sizeof(f32)*4));
        
        glEnableVertexAttribArray(colour);
        glVertexAttribPointer(colour, 4, GL_FLOAT, false, 
                              BYTES_PER_TRIANGLE, reinterpret_cast<void*>(sizeof(f32)*5));
        
    }
    
    {
        glGenVertexArrays(1, &renderer->vaos[COMMAND_RECTANGLE_OUTLINE]);
        glBindVertexArray(renderer->vaos[COMMAND_RECTANGLE_OUTLINE]);
        
        glGenBuffers(1, &renderer->buffers[COMMAND_RECTANGLE_OUTLINE]);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_RECTANGLE_OUTLINE]);
        
        // NOTE(Oliver): yeah maybe sort these constants out, looks wacko
        glBufferData(GL_ARRAY_BUFFER, MAX_DRAW*BYTES_PER_RECTANGLE_OUTLINE, 0, GL_DYNAMIC_DRAW);
        
        GLuint pos = 0;
        GLuint dim = 2;
        GLuint border_size = 4;
        GLuint radius = 5;
        GLuint colour = 6;
        
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
        glGenVertexArrays(1, &renderer->vaos[COMMAND_CIRCLE]);
        glBindVertexArray(renderer->vaos[COMMAND_CIRCLE]);
        
        glGenBuffers(1, &renderer->buffers[COMMAND_CIRCLE]);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_CIRCLE]);
        
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
        glGenVertexArrays(1, &renderer->vaos[COMMAND_GLYPH]);
        glBindVertexArray(renderer->vaos[COMMAND_GLYPH]);
        
        glGenBuffers(1, &renderer->buffers[COMMAND_GLYPH]);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_GLYPH]);
        
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
        glGenVertexArrays(1, &renderer->vaos[COMMAND_RECTANGLE_TEXTURED]);
        glBindVertexArray(renderer->vaos[COMMAND_RECTANGLE_TEXTURED]);
        
        glGenBuffers(1, &renderer->buffers[COMMAND_RECTANGLE_TEXTURED]);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vaos[COMMAND_RECTANGLE_TEXTURED]);
        
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
    
    {
        glGenVertexArrays(1, &renderer->vaos[COMMAND_BEZIER]);
        glBindVertexArray(renderer->vaos[COMMAND_BEZIER]);
        
        glGenBuffers(1, &renderer->buffers[COMMAND_BEZIER]);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_BEZIER]);
        
        // NOTE(Oliver): yeah maybe sort these constants out, looks wacko
        glBufferData(GL_ARRAY_BUFFER, MAX_DRAW*BYTES_PER_BEZIER, 0, GL_DYNAMIC_DRAW);
        
        GLuint thickness = 0;
        GLuint v0 = 1;
        GLuint v1 = 3;
        GLuint v2 = 5;
        GLuint colour = 7;
        
        glEnableVertexAttribArray(thickness);
        glVertexAttribPointer(thickness, 1, GL_FLOAT, false, 
                              BYTES_PER_BEZIER, reinterpret_cast<void*>(sizeof(f32)*0));
        
        glEnableVertexAttribArray(v0);
        glVertexAttribPointer(v0, 2, GL_FLOAT, false, 
                              BYTES_PER_BEZIER, reinterpret_cast<void*>(sizeof(f32)*1));
        
        glEnableVertexAttribArray(v1);
        glVertexAttribPointer(v1, 2, GL_FLOAT, false, 
                              BYTES_PER_BEZIER, reinterpret_cast<void*>(sizeof(f32)*3));
        
        glEnableVertexAttribArray(v2);
        glVertexAttribPointer(v2, 2, GL_FLOAT, false, 
                              BYTES_PER_BEZIER, reinterpret_cast<void*>(sizeof(f32)*5));
        
        glEnableVertexAttribArray(colour);
        glVertexAttribPointer(colour, 4, GL_FLOAT, false, 
                              BYTES_PER_BEZIER, reinterpret_cast<void*>(sizeof(f32)*7));
        
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
        OutputDebugStringA(vs);
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
        OutputDebugStringA(fs);
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
            "uniform vec4 clip_range;\n"
            
            "float box_no_pointy(vec2 p, vec2 b, float r){\n"
            "return length(max(abs(p)-b+r,0.0))-r;\n"
            "}\n"
            
            "void main(){\n"
            "float dist = box_no_pointy(gl_FragCoord.xy - (out_pos + out_dim/2), out_dim/2, out_radius);\n"
            "float alpha = clamp(mix(1, 0,  dist), 0, 1);\n"
            "vec3 debug_colour = mix(vec3(1,0,0), vec3(0,1,0), alpha);\n"
            "if(gl_FragCoord.x >= clip_range.x && gl_FragCoord.x <= clip_range.x + clip_range.z &&\n"
            "gl_FragCoord.y >= clip_range.y && gl_FragCoord.y <= clip_range.y + clip_range.w){\n"
            "colour = vec4(frag_colour.rgb, alpha*frag_colour.a);\n"
            "}else {\n"
            "discard;\n"
            "}\n"
            "}\n";
        
        GLuint program = make_program(rectangle_vs, rectangle_fs);
        
        renderer->resolution_uniforms[COMMAND_RECTANGLE] = glGetUniformLocation(program, "resolution");
        renderer->clip_range_uniforms[COMMAND_RECTANGLE] = glGetUniformLocation(program, "clip_range");
        
        renderer->ortho_uniform = glGetUniformLocation(program, "ortho");
        renderer->view_uniform = glGetUniformLocation(program, "view");
        
        renderer->programs[COMMAND_RECTANGLE] = program;
        
    }
    
    // NOTE(Oliver): init triangle shader
    {
        GLchar* triangle_vs =  
            "#version 330 core\n"
            "layout(location = 0) in vec2 pos; \n"
            "layout(location = 2) in vec2 dim; \n"
            "layout(location = 4) in float rot; \n"
            "layout(location = 5) in vec4 colour; \n"
            "uniform mat4x4 ortho;\n"
            "uniform mat4x4 view;\n"
            "uniform vec2 resolution;\n"
            "out vec2 out_pos;\n"
            "out vec2 out_dim;\n"
            "out vec4 frag_colour;\n"
            "out float frag_rot;\n"
            
            "void main(){\n"
            "vec2 vertices[] = vec2[](vec2(-1, -1), vec2(1,-1), vec2(1,1),\n"
            "vec2(-1,-1), vec2(-1, 1), vec2(1, 1));"
            "vec4 screen_position = vec4(vertices[(gl_VertexID % 6)], 0, 1);\n"
            "screen_position.xy *= (vec4(5.0*dim/resolution, 0, 1)).xy;\n"
            "screen_position.xy += 2*(vec4((pos + dim)/resolution,0,1)).xy -1;\n"
            "gl_Position = screen_position;\n"
            "out_pos = pos;\n"
            "out_dim = dim;\n"
            "frag_colour = colour;\n"
            "frag_rot = rot;\n"
            "}\n";
        
        GLchar* triangle_fs =  
            "#version 330 core\n"
            "in vec2 out_pos; \n"
            "in vec2 out_dim; \n"
            "in vec4 frag_colour;\n"
            "in float frag_rot;\n"
            "out vec4 colour;\n"
            "uniform vec2 in_position;\n"
            "uniform vec4 clip_range;\n"
            
            "vec2 rotate(vec2 pos, float r){\n"
            "const float PI = 3.14159;\n"
            "float angle = r * PI * 2 * -1;\n"
            "float sine = sin(angle);\n"
            "float cosine = cos(angle);\n"
            "return vec2(cosine * pos.x + sine * pos.y, cosine * pos.y - sine * pos.x);\n"
            "}\n"
            
            "float triangle(vec2 p, float r)\n"
            "{\n"
            "const float k = sqrt(3.0);\n"
            "p.x = abs(p.x) - r;\n"
            "p.y = p.y + r/k;\n"
            "if( p.x+k*p.y>0.0 ) p=vec2(p.x-k*p.y,-k*p.x-p.y)/2.0;\n"
            "p.x -= clamp( p.x, -2.0*r, 0.0 );\n"
            "float result = -length(p)*sign(p.y) - 0.5;\n"
            "result = min(result, result - 0.5);"
            "return result;\n"
            "}\n"
            
            "void main(){\n"
            "vec2 coord = gl_FragCoord.xy;\n"
            "vec2 pos = rotate(coord.xy - (out_pos + out_dim/2), frag_rot);\n"
            "float dist = triangle(pos, out_dim.x);\n"
            "float alpha = mix(1, 0,  dist);\n"
            "vec3 debug_colour = mix(vec3(1,0,0), vec3(0,1,0), alpha);\n"
            "if(gl_FragCoord.x >= clip_range.x && gl_FragCoord.x <= clip_range.x + clip_range.z &&\n"
            "gl_FragCoord.y >= clip_range.y && gl_FragCoord.y <= clip_range.y + clip_range.w){\n"
            "colour = vec4(frag_colour.rgb, alpha);\n"
            "}else {\n"
            "discard;\n"
            "}\n"
            "}\n";
        
        GLuint program = make_program(triangle_vs, triangle_fs);
        
        renderer->resolution_uniforms[COMMAND_TRIANGLE] = glGetUniformLocation(program, "resolution");
        renderer->clip_range_uniforms[COMMAND_TRIANGLE] = glGetUniformLocation(program, "clip_range");
        
        renderer->ortho_uniform = glGetUniformLocation(program, "ortho");
        renderer->view_uniform = glGetUniformLocation(program, "view");
        
        renderer->programs[COMMAND_TRIANGLE] = program;
        
    }
    
    
    // NOTE(Oliver): init rectangle outline shader
    {
        
        GLchar* rectangle_vs =  
            "#version 330 core\n"
            "layout(location = 0) in vec2 pos; \n"
            "layout(location = 2) in vec2 dim; \n"
            "layout(location = 4) in float border_size; \n"
            "layout(location = 5) in float radius; \n"
            "layout(location = 6) in vec4 colour; \n"
            "uniform mat4x4 ortho;\n"
            "uniform mat4x4 view;\n"
            "uniform vec2 resolution;\n"
            "out vec2 out_pos;\n"
            "out vec2 out_dim;\n"
            "out float out_radius;\n"
            "out float out_border;\n"
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
            "out_border = border_size;\n"
            "}\n";
        
        GLchar* rectangle_fs =   "#version 330 core\n"
            "in vec2 out_pos; \n"
            "in vec2 out_dim; \n"
            "in float out_radius; \n"
            "in float out_border; \n"
            "in vec4 frag_colour;\n"
            "out vec4 colour;\n"
            "uniform vec2 in_position;\n"
            "uniform vec4 clip_range;\n"
            
            "float box_no_pointy(vec2 p, vec2 b, float r){\n"
            "return length(max(abs(p)-b+r,0.0))-r;\n"
            "}\n"
            
            "float sub_sdf(float a, float b) {\n"
            "return max(-a, b);\n"
            "}\n"
            
            "float box_dist(vec2 p, vec2 size, float radius, float border){\n"
            "return sub_sdf(box_no_pointy(p, size - border, radius), box_no_pointy(p, size, radius));\n"
            "}\n"
            
            "void main(){\n"
            "float dist = box_dist(gl_FragCoord.xy - (out_pos + out_dim/2), out_dim/2, out_radius, out_border);\n"
            "float alpha = mix(1, 0,  dist);\n"
            "vec3 debug_colour = mix(vec3(1,0,0), vec3(0,1,0), dist);\n"
            "if(gl_FragCoord.x >= clip_range.x && gl_FragCoord.x <= clip_range.x + clip_range.z &&\n"
            "gl_FragCoord.y >= clip_range.y && gl_FragCoord.y <= clip_range.y + clip_range.w){\n"
            "colour = vec4(frag_colour.rgb, alpha);\n"
            "}else {\n"
            "discard;\n"
            "}\n"
            "}\n";
        
        GLuint program = make_program(rectangle_vs, rectangle_fs);
        
        renderer->resolution_uniforms[COMMAND_RECTANGLE_OUTLINE] = glGetUniformLocation(program, "resolution");
        renderer->clip_range_uniforms[COMMAND_RECTANGLE_OUTLINE] = glGetUniformLocation(program, "clip_range");
        
        
        renderer->programs[COMMAND_RECTANGLE_OUTLINE] = program;
        
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
            "screen_position.xy *= (vec4((radius*2)/resolution, 0, 1)).xy;\n"
            "screen_position.xy += 2*(vec4((pos +vec2(radius/2.0, radius/2.0))/resolution,0,1)).xy -1;\n"
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
        
        renderer->resolution_uniforms[COMMAND_CIRCLE] = glGetUniformLocation(program, "resolution");
        
        renderer->programs[COMMAND_CIRCLE] = program;
        
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
            "out vec2 frag_resolution;\n"
            
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
            "frag_resolution = resolution;\n"
            "}\n";
        
        GLchar* glyph_fs =  
            "#version 330 core\n"
            "in vec2 frag_pos; \n"
            "in vec2 frag_uv; \n"
            "in vec2 frag_dim; \n"
            "in vec4 frag_colour; \n"
            "in vec2 frag_resolution; \n"
            "out vec4 colour;\n"
            "uniform sampler2D atlas;\n"
            "uniform vec4 clip_range;\n"
            
            "float width = 0.45;\n"
            "const float edge = 0.15;\n"
            
            "vec4 get_colour(vec2 uv){\n"
            
            "float distance = 1.0 - texture(atlas, uv).a;\n"
            "float alpha = 1.0 - smoothstep(width, width + edge, distance);\n"
            "vec4 result = vec4(frag_colour.rgb, alpha);\n"
            "return result;\n"
            "}\n"
            
            "void main(){\n"
            
            "if(gl_FragCoord.x >= clip_range.x && gl_FragCoord.x <= clip_range.x + clip_range.z &&\n"
            "gl_FragCoord.y >= clip_range.y && gl_FragCoord.y <= clip_range.y + clip_range.w){\n"
            "colour = get_colour(frag_uv);\n"
            "}else {\n"
            "discard;\n"
            "}\n"
            "}\n";
        
        GLuint program = make_program(glyph_vs, glyph_fs);
        
        renderer->resolution_uniforms[COMMAND_GLYPH] = glGetUniformLocation(program, "resolution");
        renderer->clip_range_uniforms[COMMAND_GLYPH] = glGetUniformLocation(program, "clip_range");
        
        renderer->programs[COMMAND_GLYPH] = program;
        
        glGenTextures(1, &renderer->texture);
        glBindTexture(GL_TEXTURE_2D, renderer->texture);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512,
                     512, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     renderer->font.bitmap.data);
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
            "colour = texture(atlas, frag_uv);\n"
            "}\n";
        
        GLuint program = make_program(rectangle_vs, rectangle_fs);
        
        renderer->resolution_uniforms[COMMAND_RECTANGLE_TEXTURED] = glGetUniformLocation(program, "resolution");
        
        renderer->programs[COMMAND_RECTANGLE_TEXTURED] = program;
        
    }
    
    // NOTE(Oliver): init bezier shader
    {
        GLchar* bezier_vs =  
            "#version 330 core\n"
            "layout(location = 0) in float thickness; \n"
            "layout(location = 1) in vec2 v0; \n"
            "layout(location = 3) in vec2 v1;\n"
            "layout(location = 5) in vec2 v2; \n"
            "layout(location = 7) in vec4 colour; \n"
            "uniform mat4x4 ortho;\n"
            "uniform mat4x4 view;\n"
            "uniform vec2 resolution;\n"
            
            "out vec2 frag_size;\n"
            "out float frag_thickness;\n"
            "out vec2 frag_v0;\n"
            "out vec2 frag_v1;\n"
            "out vec2 frag_v2;\n"
            "out vec4 frag_colour;\n"
            "out vec2 frag_pos;\n"
            
            "void main(){\n"
            "vec2 minv = v0;\n"
            "vec2 maxv = v0;\n"
            "if(v0.x < minv.x ) minv.x = v0.x;\n"
            "if(v0.y < minv.y ) minv.y = v0.y;\n"
            "if(v0.x > maxv.x ) maxv.x = v0.x;\n"
            "if(v0.y > maxv.y ) maxv.y = v0.y;\n"
            "if(v1.x < minv.x ) minv.x = v1.x;\n"
            "if(v1.y < minv.y ) minv.y = v1.y;\n"
            "if(v1.x > maxv.x ) maxv.x = v1.x;\n"
            "if(v1.y > maxv.y ) maxv.y = v1.y;\n"
            "if(v2.x < minv.x ) minv.x = v2.x;\n"
            "if(v2.y < minv.y ) minv.y = v2.y;\n"
            "if(v2.x > maxv.x ) maxv.x = v2.x;\n"
            "if(v2.y > maxv.y ) maxv.y = v2.y;\n"
            "vec2 size = max(vec2(thickness*2, maxv.y), abs(maxv - minv));\n"
            "size *= 2.0;\n"
            "vec2 vertices[] = vec2[](vec2(-1, -1), vec2(1,-1), vec2(1,1),\n"
            "vec2(-1,-1), vec2(-1, 1), vec2(1, 1));\n"
            "vec2 uvs[] = vec2[](vec2(0,1), vec2(1,1), vec2(1,0),\n"
            "vec2(0, 1), vec2(0,0), vec2(1,0));\n"
            "vec4 screen_position = vec4(vertices[(gl_VertexID % 6)], 0, 1);\n"
            "screen_position.xy *= (vec4(size/resolution, 0, 1)).xy;\n"
            "screen_position.xy += 2*(vec4((minv + abs(maxv - minv)/2.0)/resolution,0,1)).xy -1;\n"
            //"screen_position.xy += 2*(vec4((vec2(1280,720)/2.0)/resolution,0,1)).xy -1;\n"
            "gl_Position = screen_position;\n"
            "frag_thickness = thickness;\n"
            "frag_v0 = v0;\n"
            "frag_v1 = v1;\n"
            "frag_v2 = v2;\n"
            "frag_size = size;\n"
            "frag_colour = colour;\n"
            "frag_pos = minv;\n"
            "}\n";
        
        GLchar* bezier_fs =  
            "#version 330 core\n"
            "in float frag_thickness; \n"
            "in vec4 frag_colour; \n"
            "in vec2 frag_v0; \n"
            "in vec2 frag_v1; \n"
            "in vec2 frag_v2; \n"
            "in vec2 frag_size;\n"
            "in vec2 frag_pos;\n"
            "out vec4 colour;\n"
            "uniform vec4 clip_range;\n"
            
            "float dot2( in vec2 v ) { return dot(v,v); }\n"
            "float cross2( in vec2 a, in vec2 b ) { return a.x*b.y - a.y*b.x; }\n"
            
            "// signed distance to a quadratic bezier\n"
            "float bezier(in vec2 pos, in vec2 A, in vec2 B, in vec2 C ) { \n"
            "vec2 a = B - A;\n"
            "vec2 b = A - 2.0*B + C;\n"
            "vec2 c = a * 2.0;\n"
            "vec2 d = A - pos;\n"
            "float kk = 1.0/dot(b,b);\n"
            "float kx = kk * dot(a,b);\n"
            "float ky = kk * (2.0*dot(a,a)+dot(d,b))/3.0;\n"
            "float kz = kk * dot(d,a);  \n"    
            "float res = 0.0;\n"
            "float sgn = 0.0;\n"
            "float p = ky - kx*kx;\n"
            "float p3 = p*p*p;\n"
            "float q = kx*(2.0*kx*kx - 3.0*ky) + kz;\n"
            "float h = q*q + 4.0*p3;\n"
            "if( h>=0.0 ) \n"
            "{   // 1 root\n"
            "h = sqrt(h);\n"
            "vec2 x = (vec2(h,-h)-q)/2.0;\n"
            "vec2 uv = sign(x)*pow(abs(x), vec2(1.0/3.0));\n"
            "float t = clamp( uv.x+uv.y-kx, 0.0, 1.0 );\n"
            "vec2  q = d+(c+b*t)*t;\n"
            "res = dot2(q);\n"
            "sgn = cross2(c+2.0*b*t,q);\n"
            "}\n"
            "else \n"
            "{   // 3 roots\n"
            "float z = sqrt(-p);\n"
            "float v = acos(q/(p*z*2.0))/3.0;\n"
            "float m = cos(v);\n"
            "float n = sin(v)*1.732050808;\n"
            "vec3  t = clamp( vec3(m+m,-n-m,n-m)*z-kx, 0.0, 1.0 );\n"
            "vec2  qx=d+(c+b*t.x)*t.x; float dx=dot2(qx), sx = cross2(c+2.0*b*t.x,qx);\n"
            "vec2  qy=d+(c+b*t.y)*t.y; float dy=dot2(qy), sy = cross2(c+2.0*b*t.y,qy);\n"
            "if( dx<dy ) { res=dx; sgn=sx; } else {res=dy; sgn=sy; }\n"
            "}\n"
            "return sqrt( res )*sign(sgn);\n"
            "}\n"
            
            "float segment( in vec2 p, in vec2 a, in vec2 b ) { \n"
            "vec2 pa = p-a, ba = b-a;\n"
            "float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );\n"
            "return length( pa - ba*h );\n"
            "}\n"
            
            "float udBezier( in vec2 pos, in vec2 A, in vec2 B, in vec2 C )\n"
            "{    \n"
            "vec2 a = B - A;\n"
            "vec2 b = A - 2.0*B + C;\n"
            "vec2 c = a * 2.0;\n"
            "vec2 d = A - pos;\n"
            "if(abs(dot(b,b)) < 0.01){ return segment(pos, A, C); }\n"
            "float kk = 1.0/dot(b,b);\n"
            "float kx = kk * dot(a,b);\n"
            "float ky = kk * (2.0*dot(a,a)+dot(d,b))/3.0;\n"
            "float kz = kk * dot(d,a);   \n"   
            
            "float res = 0.0;\n"
            
            "float p = ky - kx*kx;\n"
            "float p3 = p*p*p;\n"
            "float q = kx*(2.0*kx*kx - 3.0*ky) + kz;\n"
            "float h = q*q + 4.0*p3;\n"
            
            "if( h>=0.0 ) \n"
            "{   // 1 root\n"
            "h = sqrt(h);\n"
            "vec2 x = (vec2(h,-h)-q)/2.0;\n"
            "vec2 uv = sign(x)*pow(abs(x), vec2(1.0/3.0));\n"
            "float t = clamp( uv.x+uv.y-kx, 0.0, 1.0 );\n"
            "res = dot2(d+(c+b*t)*t);\n"
            "}\n"
            "else \n"
            "{   // 3 roots\n"
            "float z = sqrt(-p);\n"
            "float v = acos(q/(p*z*2.0))/3.0;\n"
            "float m = cos(v);\n"
            "float n = sin(v)*1.732050808;\n"
            "vec3  t = clamp( vec3(m+m,-n-m,n-m)*z-kx, 0.0, 1.0 );\n"
            "res = min( dot2(d+(c+b*t.x)*t.x),\n"
            "dot2(d+(c+b*t.y)*t.y) );\n"
            "}\n"
            "return sqrt( res );\n"
            "}\n"
            "void main(){\n"
            "if(gl_FragCoord.x >= clip_range.x && gl_FragCoord.x <= clip_range.x + clip_range.z &&\n"
            "gl_FragCoord.y >= clip_range.y && gl_FragCoord.y <= clip_range.y + clip_range.w){\n"
            "float d = udBezier(gl_FragCoord.xy, frag_v0, frag_v1, frag_v2);\n"
            "float alpha = d;\n"
            "colour = vec4(frag_colour.rgb, 1.0-smoothstep(frag_thickness-1, frag_thickness,abs(d)));\n"
            "}else {\n"
            "discard;\n"
            "}\n"
            "}\n";
        
        GLuint program = make_program(bezier_vs, bezier_fs);
        
        renderer->resolution_uniforms[COMMAND_BEZIER] = glGetUniformLocation(program, "resolution");
        renderer->clip_range_uniforms[COMMAND_BEZIER] = glGetUniformLocation(program, "clip_range");
        
        renderer->programs[COMMAND_BEZIER] = program;
        
    }
    
}

internal void
process_and_draw_commands(){
    ;
    
    
    f32* rectangles = (f32*)push_size_zero(&platform->frame_arena, MAX_DRAW*BYTES_PER_RECTANGLE);
    f32* rectangle_outlines = (f32*)push_size_zero(&platform->frame_arena, MAX_DRAW*BYTES_PER_RECTANGLE_OUTLINE);
    f32* triangles = (f32*)push_size_zero(&platform->frame_arena, MAX_DRAW*BYTES_PER_TRIANGLE);
    f32* circles = (f32*)push_size_zero(&platform->frame_arena, MAX_DRAW*BYTES_PER_CIRCLE);
    f32* glyphs = (f32*)push_size_zero(&platform->frame_arena, MAX_DRAW*BYTES_PER_GLYPH);
    f32* rectangles_textured = (f32*)push_size_zero(&platform->frame_arena, MAX_DRAW*BYTES_PER_RECTANGLE_TEXTURED);
    f32* beziers = (f32*)push_size_zero(&platform->frame_arena, MAX_DRAW*BYTES_PER_BEZIER);
    
    v4f clip_range = v4f(0, 0, platform->window_size.width, platform->window_size.height);
    for(Command* command = renderer->head; command; command = command->next){
        Command* previous_command = command;
        
        switch(command->type){
            case COMMAND_CLIP_RANGE_BEGIN: {
                auto clip = command->clip_range;
                clip_range = v4f(clip.x, 
                                 clip.y, 
                                 clip.width, 
                                 clip.height);
                
            }break;
            case COMMAND_CLIP_RANGE_END:{
                clip_range = v4f(0,0, platform->window_size.width, platform->window_size.height);
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
                        
                        *attribs++ = rectangle->rectangle.x;
                        *attribs++ = rectangle->rectangle.y;
                        *attribs++ = rectangle->rectangle.width;
                        *attribs++ = rectangle->rectangle.height;
                        *attribs++ = rectangle->rectangle.corner_radius;
                        *attribs++ = (rectangle->colour.r/255.0f);
                        *attribs++ = (rectangle->colour.g/255.0f);
                        *attribs++ = (rectangle->colour.b/255.0f);
                        *attribs++ = (rectangle->colour.a/255.0f);
                        num_verts++;
                        
                    }
                }
                // NOTE(Oliver): draw filled rects data
                
                {
                    glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_RECTANGLE]);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                    num_verts*BYTES_PER_RECTANGLE,
                                    rectangles);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                    glUseProgram(renderer->programs[COMMAND_RECTANGLE]);
                    
                    f32 resolution[2] = {(f32)platform->window_size.width, (f32)platform->window_size.height};
                    glUniform2fv(renderer->resolution_uniforms[COMMAND_RECTANGLE], 1, resolution);
                    
                    glUniform4fv(renderer->clip_range_uniforms[COMMAND_RECTANGLE], 1, (f32*)&clip_range);
                    
                    glBindVertexArray(renderer->vaos[COMMAND_RECTANGLE]);
                    glDrawArrays(GL_TRIANGLES, 0, num_verts);
                    
                    glUseProgram(0);
                }
                
            }break;
            
            case COMMAND_TRIANGLE:{
                
                int num_verts = 0;
                f32* attribs = triangles;
                for(Command* _command = command; _command && _command->type == previous_command->type; 
                    _command = _command->next){
                    
                    auto triangle = _command;
                    for(int i = 0; i < 6; i++){
                        v4f r = v4f(triangle->triangle.x,
                                    triangle->triangle.y,
                                    triangle->triangle.size,
                                    triangle->triangle.size);
                        
                        *attribs++ = triangle->triangle.x;
                        *attribs++ = triangle->triangle.y;
                        *attribs++ = triangle->triangle.size;
                        *attribs++ = triangle->triangle.size;
                        *attribs++ = triangle->triangle.rotation;
                        *attribs++ = (triangle->colour.r/255.0f);
                        *attribs++ = (triangle->colour.g/255.0f);
                        *attribs++ = (triangle->colour.b/255.0f);
                        *attribs++ = (triangle->colour.a/255.0f);
                        num_verts++;
                        
                    }
                }
                // NOTE(Oliver): draw filled rects data
                {
                    glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_TRIANGLE]);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                    num_verts*BYTES_PER_TRIANGLE,
                                    triangles);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                    glUseProgram(renderer->programs[COMMAND_TRIANGLE]);
                    
                    f32 resolution[2] = {(f32)platform->window_size.width, (f32)platform->window_size.height};
                    glUniform2fv(renderer->resolution_uniforms[COMMAND_TRIANGLE], 1, resolution);
                    
                    glUniform4fv(renderer->clip_range_uniforms[COMMAND_TRIANGLE], 1, (f32*)&clip_range);
                    
                    glBindVertexArray(renderer->vaos[COMMAND_TRIANGLE]);
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
                        *attribs++ = (rectangle->colour.r/255.0f);
                        *attribs++ = (rectangle->colour.g/255.0f);
                        *attribs++ = (rectangle->colour.b/255.0f);
                        *attribs++ = (rectangle->colour.a/255.0f);
                    }
                    num_verts += 6;
                }
                
                {
                    glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_RECTANGLE_OUTLINE]);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                    num_verts*BYTES_PER_RECTANGLE_OUTLINE,
                                    rectangle_outlines);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                    glUseProgram(renderer->programs[COMMAND_RECTANGLE_OUTLINE]);
                    
                    f32 resolution[2] = {(f32)platform->window_size.width, (f32)platform->window_size.height};
                    glUniform2fv(renderer->resolution_uniforms[COMMAND_RECTANGLE_OUTLINE], 
                                 1, resolution);
                    glUniform4fv(renderer->clip_range_uniforms[COMMAND_RECTANGLE_OUTLINE], 1, (f32*)&clip_range);
                    
                    glBindVertexArray(renderer->vaos[COMMAND_RECTANGLE_OUTLINE]);
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
                    glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_CIRCLE]);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                    num_verts*BYTES_PER_CIRCLE,
                                    circles);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                    glUseProgram(renderer->programs[COMMAND_CIRCLE]);
                    
                    f32 resolution[2] = {(f32)platform->window_size.width, (f32)platform->window_size.height};
                    glUniform2fv(renderer->resolution_uniforms[COMMAND_CIRCLE], 1, resolution);
                    
                    glBindVertexArray(renderer->vaos[COMMAND_CIRCLE]);
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
                        //if(!should_clip || (should_clip && is_rect_inside_rect(r, clip_range))){
                        *attribs++ = glyph->glyph.x;
                        *attribs++ = glyph->glyph.y;
                        *attribs++ = glyph->glyph.width;
                        *attribs++ = glyph->glyph.height;
                        *attribs++ = glyph->glyph.u;
                        *attribs++ = glyph->glyph.v;
                        *attribs++ = glyph->glyph.u_width;
                        *attribs++ = glyph->glyph.v_height;
                        *attribs++ = glyph->colour.r/255.0f;
                        *attribs++ = glyph->colour.g/255.0f;
                        *attribs++ = glyph->colour.b/255.0f;
                        *attribs++ = glyph->colour.a/255.0f;
                        num_verts++;
                        //}
                    }
                }
                {
                    glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_GLYPH]);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                    num_verts*BYTES_PER_GLYPH,
                                    glyphs);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                    glUseProgram(renderer->programs[COMMAND_GLYPH]);
                    
                    f32 resolution[2] = {(f32)platform->window_size.width, (f32)platform->window_size.height};
                    glUniform2fv(renderer->resolution_uniforms[COMMAND_GLYPH], 1, resolution);
                    
                    glUniform4fv(renderer->clip_range_uniforms[COMMAND_GLYPH], 1, (f32*)&clip_range);
                    
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, renderer->texture);
                    
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    
                    glBindVertexArray(renderer->vaos[COMMAND_GLYPH]);
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
                    glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_RECTANGLE_TEXTURED]);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                    6*BYTES_PER_RECTANGLE_TEXTURED,
                                    rectangles_textured);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                    glUseProgram(renderer->programs[COMMAND_RECTANGLE_TEXTURED]);
                    
                    f32 resolution[2] = {(f32)platform->window_size.width, (f32)platform->window_size.height};
                    glUniform2fv(renderer->resolution_uniforms[COMMAND_RECTANGLE_TEXTURED], 1, resolution);
                    
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, rectangle->rectangle_textured.bitmap.texture);
                    
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    
                    glBindVertexArray(renderer->vaos[COMMAND_RECTANGLE_TEXTURED]);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glUseProgram(0);
                }
            }break;
            
            case COMMAND_BEZIER:{
                
                int num_verts = 0;
                f32* attribs = beziers;
                for(Command* _command = command; _command && _command->type == previous_command->type; 
                    _command = _command->next){
                    
                    auto bezier = _command;
                    for(int i = 0; i < 6; i++){
                        
                        *attribs++ = bezier->bezier.thickness;
                        *attribs++ = bezier->bezier.v0.x;
                        *attribs++ = bezier->bezier.v0.y;
                        *attribs++ = bezier->bezier.v1.x;
                        *attribs++ = bezier->bezier.v1.y;
                        *attribs++ = bezier->bezier.v2.x;
                        *attribs++ = bezier->bezier.v2.y;
                        *attribs++ = (bezier->colour.r/255.0f);
                        *attribs++ = (bezier->colour.g/255.0f);
                        *attribs++ = (bezier->colour.b/255.0f);
                        *attribs++ = (bezier->colour.a/255.0f);
                        num_verts++;
                        
                    }
                }
                
                {
                    glBindBuffer(GL_ARRAY_BUFFER, renderer->buffers[COMMAND_BEZIER]);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                    num_verts*BYTES_PER_BEZIER,
                                    beziers);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                    glUseProgram(renderer->programs[COMMAND_BEZIER]);
                    
                    f32 resolution[2] = {(f32)platform->window_size.width, (f32)platform->window_size.height};
                    glUniform2fv(renderer->resolution_uniforms[COMMAND_BEZIER], 1, resolution);
                    glUniform4fv(renderer->clip_range_uniforms[COMMAND_BEZIER], 1, (f32*)&clip_range);
                    
                    glBindVertexArray(renderer->vaos[COMMAND_BEZIER]);
                    glDrawArrays(GL_TRIANGLES, 0, num_verts);
                    glUseProgram(0);
                }
                
            }break;
            
        }
        command = previous_command;
        
    }
    
}


internal void
opengl_start_frame() {
    renderer->head = nullptr;
    renderer->tail = nullptr;
    
}

internal void
opengl_end_frame() {
    glViewport(0, 0, platform->window_size.width, platform->window_size.height);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    process_and_draw_commands();
    
}
