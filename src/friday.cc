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

extern "C" {
    PERMANENT_LOAD {
        platform = platform_;
        load_all_opengl_procs();
    }
    
    HOT_LOAD {
        platform = platform_;
    }
    
    HOT_UNLOAD {
    }
    
    UPDATE {
        glClearColor(1, 0,0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        platform->refresh_screen();
    }
}