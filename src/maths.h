
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

union v2i {
    struct{
        int x;
        int y;
    };
    struct {
        int u;
        int v;
    };
    struct {
        int width;
        int height;
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
        union{
            struct{
                f32 z;
                f32 w;
            };
            struct{
                f32 width;
                f32 height;
            };
        };
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
    
};
