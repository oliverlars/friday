#include "extras.h"
#include "maths.h"
#include "memory.h"
#include "strings.h"
#include "platform.h"
#include "opengl.h"

#include "extras.cc"
#include "maths.cc"
#include "memory.cc"
#include "strings.cc"
#include "platform.cc"


#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"


#include "render.cc"

BEGIN_C_EXPORT

PERMANENT_LOAD {
    platform = platform_;
    load_all_opengl_procs();
    init_opengl_renderer();
    init_shaders();
}

HOT_LOAD {
    platform = platform_;
}

HOT_UNLOAD {
    
}

UPDATE {
    opengl_start_frame();
    {
        push_rectangle(30, 50, 400, 400, 1, 0xFF0000FF);
    }
    opengl_end_frame();
    platform->refresh_screen();
}

END_C_EXPORT