
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

internal v4f
make_v4f(v2f pos, v2f size){
    v4f result;
    result.pos = pos;
    result.size = size;
    return result;
}

#define v4f(x, y, z, w) make_v4f(x, y, z, w)
#define v4f2(pos, size) make_v4f(pos, size)


internal f32
clampf(f32 value, f32 min, f32 max){
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

internal void
clampf(f32* value, f32 min, f32 max){
    if(*value < min) *value = min;
    if(*value > max) *value = max;
}

internal int
clampi(int value, int min, int max){
    if(max< min) max = min;
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

internal void
clampi(int* value, int min, int max){
    if(max< min) max = min;
    if(*value < min) *value = min;
    if(*value > max) *value = max;
}

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

internal v4f
inflate_rect(v4f a, f32 amount){
    v4f result = a;
    result.x -= amount;
    result.width += amount*2;
    result.y -= amount;
    result.height += amount*2;
    return result;
}

internal f32
aspectof(v2f a){
    if(a.x > a.y){
        return a.x/a.y;
    }else {
        return a.y/a.x;
    }
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
colour_from_v4f(v4f rgba){
    Colour result = {};
    result.a = rgba.a*255.0f;
    result.b = rgba.b*255.0f;
    result.g = rgba.g*255.0f;
    result.r = rgba.r*255.0f;
    return result;
}

internal v4f
v4f_from_colour(Colour colour){
    v4f result = {};
    result.a = colour.a/255.0f;
    result.b = colour.b/255.0f;
    result.g = colour.g/255.0f;
    result.r = colour.r/255.0f;
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

internal v4f
union_rects(v4f a, v4f b){
    v4f result = {};
    result.x = min(a.x, b.x);
    result.y = min(a.x, b.x);
    result.width = max(a.width, b.width);
    result.height = max(a.height, b.height);
    return result;
}

internal v4f
divide_rect(v4f a, f32 b){
    v4f result = a;
    result.x /= b;
    result.y /= b;
    result.z /= b;
    result.w /= b;
    return result;
}

internal inline bool
between(f32 value, f32 min, f32 max){
    return (value < max) && value > min;
}

internal inline v4f
rect_border(v4f rect, f32 border){
    return v4f(rect.x + border, rect.y + border,
               rect.width - border*2,
               rect.height - border*2);
}

internal bool
is_in_rect(v2f pos, v4f rect){
    return pos.x <= (rect.x + rect.width) && pos.x >= rect.x &&
        pos.y <= (rect.y + rect.height) && pos.y >= rect.y;
}

internal inline bool
is_in_bottom_or_top_border(v2f pos, v4f rect, f32 thickness){
    bool top = is_in_rect(pos, v4f(rect.x, rect.y, rect.width, thickness));
    bool bottom = is_in_rect(pos, v4f(rect.x, rect.y+rect.height-thickness, rect.width, thickness));
    return top || bottom;
    
}

internal inline bool
is_in_left_or_right_border(v2f pos, v4f rect, f32 thickness){
    bool left = is_in_rect(pos, v4f(rect.x, rect.y, thickness, rect.height));
    bool right = is_in_rect(pos, v4f(rect.x+rect.width-thickness, rect.y, thickness, rect.height));
    return left || right;
}

internal inline bool
is_in_rect_border(v2f pos, v4f rect, f32 thickness){
    return is_in_left_or_right_border(pos, rect, thickness) ||
        is_in_bottom_or_top_border(pos, rect, thickness);
}


inline v2f
operator+(v2f a, v2f b){
    v2f result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

inline v2f
operator+=(v2f& a, v2f b){
    a = a + b;
}

inline v2f
operator-(v2f a, v2f b){
    v2f result = {};
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

inline v2f
operator-=(v2f& a, v2f b){
    a = a - b;
}

inline v2f
operator-(v2f a){
    v2f result = {};
    result.x = -a.x;
    result.y = -a.y;
    return result;
}

inline v2f
operator*(v2f a, f32 b){
    v2f result = {};
    result.x = a.x*b;
    result.y = a.y*b;
    return result;
}

inline v2f
operator/(v2f a, f32 b){
    v2f result = {};
    result.x = a.x/b;
    result.y = a.y/b;
    return result;
}

inline v3f
operator+(v3f a, v3f b){
    v3f result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

inline v3f
operator+=(v3f& a, v3f b){
    a = a + b;
}

inline v3f
operator-(v3f a, v3f b){
    v3f result = {};
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

inline v3f
operator-=(v3f& a, v3f b){
    a = a - b;
}

inline v3f
operator-(v3f a){
    v3f result = {};
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    return result;
}

inline v3f
operator*(v3f a, f32 b){
    v3f result = {};
    result.x = a.x*b;
    result.y = a.y*b;
    result.z = a.z*b;
    return result;
}

inline v3f
operator/(v3f a, f32 b){
    v3f result = {};
    result.x = a.x/b;
    result.y = a.y/b;
    result.z = a.z/b;
    return result;
}

inline v4f
operator+(v4f a, v4f b){
    v4f result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

inline v4f
operator+=(v4f& a, v4f b){
    a = a + b;
}

inline v4f
operator-(v4f a, v4f b){
    v4f result = {};
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return result;
}

inline v4f
operator-=(v4f& a, v4f b){
    a = a - b;
}

inline v4f
operator-(v4f a){
    v4f result = {};
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;
    return result;
}

inline v4f
operator*(v4f a, f32 b){
    v4f result = {};
    result.x = a.x*b;
    result.y = a.y*b;
    result.z = a.z*b;
    result.w = a.w*b;
    return result;
}

inline v4f
operator/(v4f a, f32 b){
    v4f result = {};
    result.x = a.x/b;
    result.y = a.y/b;
    result.z = a.z/b;
    result.w = a.w/b;
    return result;
}


inline v2i
operator+(v2i a, v2i b){
    v2i result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

inline v2i
operator+=(v2i& a, v2i b){
    a = a + b;
}

inline v2i
operator-(v2i a, v2i b){
    v2i result = {};
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

inline v2i
operator-=(v2i& a, v2i b){
    a = a - b;
}

inline v2i
operator-(v2i a){
    v2i result = {};
    result.x = -a.x;
    result.y = -a.y;
    return result;
}

inline v2i
operator*(v2i a, s32 b){
    v2i result = {};
    result.x = a.x*b;
    result.y = a.y*b;
    return result;
}

inline v2i
operator/(v2i a, s32 b){
    v2i result = {};
    result.x = a.x/b;
    result.y = a.y/b;
    return result;
}

inline v3i
operator+(v3i a, v3i b){
    v3i result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

inline v3i
operator+=(v3i& a, v3i b){
    a = a + b;
}

inline v3i
operator-(v3i a, v3i b){
    v3i result = {};
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

inline v3i
operator-=(v3i& a, v3i b){
    a = a - b;
}

inline v3i
operator-(v3i a){
    v3i result = {};
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    return result;
}

inline v3i
operator*(v3i a, s32 b){
    v3i result = {};
    result.x = a.x*b;
    result.y = a.y*b;
    result.z = a.z*b;
    return result;
}

inline v3i
operator/(v3i a, s32 b){
    v3i result = {};
    result.x = a.x/b;
    result.y = a.y/b;
    result.z = a.z/b;
    return result;
}

inline v4i
operator+(v4i a, v4i b){
    v4i result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

inline v4i
operator+=(v4i& a, v4i b){
    a = a + b;
}

inline v4i
operator-(v4i a, v4i b){
    v4i result = {};
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return result;
}

inline v4i
operator-=(v4i& a, v4i b){
    a = a - b;
}

inline v4i
operator-(v4i a){
    v4i result = {};
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;
    return result;
}

inline v4i
operator*(v4i a, s32 b){
    v4i result = {};
    result.x = a.x*b;
    result.y = a.y*b;
    result.z = a.z*b;
    result.w = a.w*b;
    return result;
}

inline v4i
operator/(v4i a, s32 b){
    v4i result = {};
    result.x = a.x/b;
    result.y = a.y/b;
    result.z = a.z/b;
    result.w = a.w/b;
    return result;
}