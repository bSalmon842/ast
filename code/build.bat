@echo off

REM -MTd for debug build
set commonFlagsCompiler=-MTd -nologo -Gm- -GR- -fp:fast -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4996 -FC -Z7 -DAST_INTERNAL=1 -DAST_SLOW=1
set commonFlagsLinker= -incremental:no -opt:ref 

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

cl %commonFlagsCompiler% ..\code\w32_ast.cpp /link %commonFlagsLinker% user32.lib gdi32.lib ole32.lib
popd
