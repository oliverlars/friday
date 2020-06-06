
union v2 {
    struct {
        f32 x, y;
    };
    struct {
        f32 u, v;
    };
};

struct mat4x4 {
    float e[16];
};


internal mat4x4
ortho(f32 left, f32 right, f32 bottom, f32 top){
    
    // NOTE(Oliver): C is stupid
    f32 _near = -1;
    f32 _far = 1;
    
    f32 a = 2.0f/(right - left);
    f32 b = 2.0f/(top - bottom);
    f32 c = -2.0f/(_far - _near);
    
    f32 tx = -((right + left)/(right - left));
    f32 ty = -((top + bottom)/(top - bottom));
    f32 tz = -((_far+_near)/(_far-_near));
    
    mat4x4 result;
    result = {
        a, 0, 0, tx,
        0, b, 0, ty,
        0, 0, c, tz,
        0, 0, 0, 1
    };
    
    return result;
}

internal mat4x4
ortho(f32 size, f32 aspect){
    
    f32 right = size * aspect;
    f32 left = -right;
    
    f32 top = size;
    f32 bottom = -top;
    
    return ortho(left, right, top, bottom);
}

internal mat4x4
translate(f32 x, f32 y){
    
    mat4x4 result = {};
    result = {
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    return result;
}