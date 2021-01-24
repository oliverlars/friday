@echo off

set application_name=friday
set build_options= -DBUILD_WIN32=1
set compile_flags= -nologo /FS /W0 /Zi /FC /Zc:preprocessor /I ../src/
set common_link_flags= opengl32.lib -opt:ref -incremental:no /Debug:FULL
set platform_link_flags= gdi32.lib user32.lib winmm.lib %common_link_flags% friday.res

if not exist build mkdir build
pushd build
del *.pdb
start /b /wait "" "clang-cl.exe"  %build_options% %compile_flags% ../src/win32/win32_entry.cc /link %platform_link_flags% /out:%application_name%.exe
start /b /wait "" "clang-cl.exe"  %build_options% %compile_flags% ../src/friday.cc /LD /link %common_link_flags% /out:%application_name%.dll
popd