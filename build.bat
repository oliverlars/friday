@echo off

set application_name=friday
set build_options= -DBUILD_WIN32=1
set compile_flags= -nologo /W0 /Zi /FC /I ../src/
set common_link_flags= opengl32.lib -opt:ref -incremental:no /Debug:fastlink
set platform_link_flags= gdi32.lib user32.lib winmm.lib %common_link_flags%

if not exist build mkdir build
pushd build
start /b /wait "" "cl.exe"  %build_options% %compile_flags% ../src/win32/win32_entry.cc /link %platform_link_flags% /out:%application_name%.exe
start /b /wait "" "cl.exe"  %build_options% %compile_flags% ../src/friday.cc /LD /link %common_link_flags% /out:%application_name%.dll
popd