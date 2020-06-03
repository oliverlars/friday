@echo off
pushd build
clang-cl /Zi -Wno-narrowing -nologo /W0 ..\src\friday.cc /link /SUBSYSTEM:WINDOWS /out:friday.exe opengl32.lib user32.lib gdi32.lib ../lib/SDL2main.lib ../lib/SDL2.lib shell32.lib
popd