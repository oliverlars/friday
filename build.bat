@echo off
pushd build
clang-cl /Zi -Wno-narrowing -nologo /W0 ..\src\friday.cc /link /DEBUG:FULL /INCREMENTAL:NO /SUBSYSTEM:WINDOWS /out:friday.exe user32.lib gdi32.lib ../lib/SDL2main.lib ../lib/SDL2.lib shell32.lib opengl32.lib
popd