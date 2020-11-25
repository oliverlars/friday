#include "extras.h"
#include "maths.h"
#include "memory.h"
#include "strings.h"
#include "platform.h"
#include "opengl.h"
#include "render.h"
#include "ui.h"
#include "friday.h"

#include "extras.cc"
#include "maths.cc"
#include "memory.cc"
#include "strings.cc"
#include "platform.cc"


#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"


#include "render.cc"
#include "ui.cc"

global Friday_Globals* globals = 0;

internal void
initialise_globals(){
    globals = (Friday_Globals*)platform->globals;
    renderer = globals->renderer;
    ui = globals->ui;
    
}

BEGIN_C_EXPORT

PERMANENT_LOAD {
    platform = platform_;
    
    platform->globals = arena_allocate_zero(&platform->permanent_arena, sizeof(Friday_Globals));
    globals = (Friday_Globals*)platform->globals;
    globals->renderer = (Renderer_State*)arena_allocate_zero(&platform->permanent_arena, sizeof(Renderer_State));
    globals->ui = (UI_State*)arena_allocate_zero(&platform->permanent_arena, sizeof(UI_State));
    
    load_all_opengl_procs();
    
    globals->renderer->font = load_sdf_font("../fonts/friday_default.fnt");
    initialise_globals();
    init_opengl_renderer();
    init_shaders();
    load_theme_ayu();
    
}

HOT_LOAD {
    platform = platform_;
    load_all_opengl_procs();
    initialise_globals();
    load_theme_ayu();
}

HOT_UNLOAD {
    
}

UPDATE {
    opengl_start_frame();
    
    opengl_end_frame();
    platform->refresh_screen();
}

END_C_EXPORT