
 union mat4x4 {
    float e[16];
    struct {
        f32 m00, m01, m02, m03;
        f32 m10, m11, m12, m13;
        f32 m20, m21, m22, m23;
        f32 m30, m31, m32, m33;
    };
};

union mat3x3 {
    f32 e[9];
    struct {
        f32 m00, m01, m02;
        f32 m10, m11, m12;
        f32 m20, m21, m22;
    };
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
translate(f32 x, f32 y, f32 z){
    
    mat4x4 result = {};
    
    result = {
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, 1
    };
    
    return result;
}

internal mat3x3
translate(f32 x, f32 y){
    
    mat3x3 result = {};
    
    result = {
        1, 0, x,
        0, 1, y,
        0, 0, 1
    };
    
    return result;
}

union v2i {
    struct{
        int x;
        int y;
    };
    struct {
        int u;
        int v;
    };
};

union v2f {
    struct{
        f32 x;
        f32 y;
    };
    struct {
        f32 u;
        f32 v;
    };
};

union v3i {
    struct {
        int x;
        int y;
        int z;
    };
    struct {
        int u;
        int v;
        int w;
    };
    struct {
        int r;
        int g;
        int b;
    };
};


union v3f {
    struct {
        f32 x;
        f32 y;
        f32 z;
    };
    struct {
        f32 u;
        f32 v;
        f32 w;
    };
    struct {
        f32 r;
        f32 g;
        f32 b;
    };
};

union v4i {
    struct {
        int x;
        int y;
        int z;
        int w;
    };
    struct {
        int r;
        int g;
        int b;
        int a;
    };
};

union v4f {
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    struct {
        f32 x0;
        f32 y0;
        f32 x1;
        f32 y1;
    };
    struct {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    struct {
        f32 x;
        f32 y;
        f32 width;
        f32 height;
    };
};

internal v2i
make_v2i(int x, int y){
    v2i result;
    result.x = x;
    result.y = y;
    return result;
}
#define v2i(x, y) make_v2i(x, y)

internal v2f
make_v2f(f32 x, f32 y){
    v2f result;
    result.x = x;
    result.y = y;
    return result;
}
#define v2f(x, y) make_v2f(x, y)

internal v3i
make_v3i(int x, int y, int z){
    v3i result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}
#define v3i(x, y, z) make_v3i(x, y, z)

internal v3f
make_v3f(f32 x, f32 y, f32 z){
    v3f result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}
#define v3f(x, y, z) make_v3f(x, y, z)

internal v4i
make_v4i(int x, int y, int z, int w){
    v4i result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
}
#define v4i(x, y, z, w) make_v4i(x, y, z, w)

internal v4f
make_v4f(f32 x, f32 y, f32 z, f32 w){
    v4f result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
}

#define v4f(x, y, z, w) make_v4f(x, y, z, w)


internal f32
lerp(f32 source, f32 target, f32 value){
    return (target - source)*value;
}

internal void
lerp(f32* source, f32 target, f32 value){
    *source += (target - *source)*value;
}

internal void
lerp(int* source, int target, f32 value){
    *source += (target - *source)*value;
}

internal f32
smooth_step(f32 source, f32 target, f32 value){
    
    value = clampf((value - source)/(target - source), 0.0, 1.0f);
    
    return value*value*(3 -2 * value);
    
}

internal v4f
lerp_rects(v4f a, v4f b, f32 amount){
    v4f result = {};
    result.x = (b.x - a.x)*amount;
    result.y = (b.y - a.y)*amount;
    result.z = (b.z - a.z)*amount;
    result.w = (b.w - a.w)*amount;
    return result;
}

internal void
lerp_rects(v4f* a, v4f b, f32 amount){
    a->x += (b.x - a->x)*amount;
    a->y += (b.y - a->y)*amount;
    a->z += (b.z - a->z)*amount;
    a->w += (b.w - a->w)*amount;
}

internal v4f
add_rects(v4f a, v4f b){
    v4f result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}


internal b32
rects_eq(v4f a, v4f b){
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
}

internal b32
rects_similar(v4f a, v4f b, f32 tolerance){
    
    return (fabs(a.x - b.x) < tolerance) && (fabs(a.y - b.y) < tolerance) && 
        (fabs(a.z - b.z) < tolerance) && (fabs(a.w - b.w) < tolerance);
}

internal b32
is_rect_inside_rect(v4f a, v4f b){
    return (a.x >= b.x) && (a.x + a.width <= b.x + b.width) &&
        (a.y >= b.y) && (a.y <= b.y + b.height);
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

internal Colour
lerp_colours(Colour a, Colour b, f32 amount){
    Colour result ={};
    result.a = (a.a - b.a)*amount;
    result.b = (a.b - b.b)*amount;
    result.g = (a.g - b.g)*amount;
    result.r = (a.r - b.r)*amount;
    return result;
}

internal Colour
add_colours(Colour a, Colour b){
    Colour result = {};
    result.a = a.a + b.a;
    result.b = a.b + b.b;
    result.g = a.g + b.g;
    result.r = a.r + b.r;
    return result;
}

internal inline bool
between(f32 value, f32 min, f32 max){
    return (value < max) && value > min;
    
}