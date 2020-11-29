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

internal void
start_frame(){
    opengl_start_frame();
    arena_clear(&globals->ui->frame_arena);
}

internal void
end_frame(){
    opengl_end_frame();
}

BEGIN_C_EXPORT

PERMANENT_LOAD {
    platform = platform_;
    
    platform->globals = push_type(&platform->permanent_arena, Friday_Globals);
    globals = (Friday_Globals*)platform->globals;
    globals->renderer = push_type_zero(&platform->permanent_arena, Renderer_State);
    globals->ui = push_type_zero(&platform->permanent_arena, UI_State);
    globals->ui->frame_arena = subdivide_arena(&platform->permanent_arena, 8192);
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
    start_frame();
    {
        UI_WINDOW(v4f(platform->window_size.width/2.0f, 
                      platform->window_size.height/2.0f, 200, 200), "test") UI_COLUMN {
            button("test button");
        }
        layout_widgets(ui->root);
        render_widgets(ui->root);
    }
    end_frame();
    platform->refresh_screen();
}

END_C_EXPORT