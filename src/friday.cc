#include "extras.h"
#include "maths.h"
#include "memory.h"
#include "strings.h"
#include "platform.h"
#include "opengl.h"
#include "render.h"
#include "ui.h"
#include "present.h"
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
#include "present.cc"

global Friday_Globals* globals = 0;

internal void
initialise_globals(){
    globals = (Friday_Globals*)platform->globals;
    renderer = globals->renderer;
    ui = globals->ui;
    
}

internal void
start_frame(){
    ui->root = nullptr;
    opengl_start_frame();
}

internal void
end_frame(){
    opengl_end_frame();
}

#define FRAME defer_loop(start_frame(), end_frame())

BEGIN_C_EXPORT

PERMANENT_LOAD {
    platform = platform_;
    
    platform->globals = push_type(&platform->permanent_arena, Friday_Globals);
    globals = (Friday_Globals*)platform->globals;
    globals->renderer = push_type_zero(&platform->permanent_arena, Renderer_State);
    globals->ui = push_type_zero(&platform->permanent_arena, UI_State);
    load_all_opengl_procs();
    globals->renderer->font = load_sdf_font("../fonts/friday_default.fnt");
    initialise_globals();
    init_opengl_renderer();
    init_shaders();
    load_theme_dots();
    
}

HOT_LOAD {
    platform = platform_;
    load_all_opengl_procs();
    initialise_globals();
    load_theme_dots();
}

HOT_UNLOAD {
    
}

UPDATE {
    FRAME
    {
        
        UI_WINDOW(v4f(platform->window_size.width/2.0f, 
                      platform->window_size.height/2.0f, 400 + sinf(platform->get_time()*5)*20.0f, 
                      cosf(platform->get_time()*5)*20.0f + 200), "Properties") {
            UI_ROW {
                button("Uh Oh");
                button("Widgetables");
            }
            UI_ROW UI_WIDTHFILL {
                button("AAAAAAA");
                button("3");
            }
            UI_ROW UI_WIDTHFILL{
                for(int i = 0; i < 5; i++){
                    xspacer(i*10);
                    button("%d", i);
                }
                
            }
            for(int i = 5; i > 0; i--){
                button("%d", i);
            }
        }
        
#if 1        
        UI_WINDOW(v4f(platform->window_size.width/2.0f - 600, 
                      platform->window_size.height/2.0f + 200, 
                      800+ sinf(platform->get_time()*5)*20.0f, 
                      cosf(platform->get_time()*5)*20.0f + 500), "Code Editor") {
            UI_ROW {
                present_keyword("where is the text?");
            }
            
        }
#endif
        ForEachWidgetSibling(ui->root){
            layout_widgets(it);
            render_widgets(it);
        }
    }
    platform->refresh_screen();
}

END_C_EXPORT