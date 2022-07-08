@echo off

echo REBUILDING WHILE LIVE

time /T
date /T
echo.

REM -MTd for debug build
set commonFlagsCompiler=-MTd -nologo -Gm- -GS- -GR- -fp:fast -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4996 -wd4311 -wd4302 -FC -Z7 -DAST_INTERNAL=1 -DAST_SLOW=1
set commonFlagsLinker= -incremental:no -opt:ref

IF NOT EXIST ..\build mkdir ..\build
IF NOT EXIST ..\build\asset_build mkdir ..\build\asset_build
pushd ..\build

REM 64-bit
del *.pdb > NUL 2> NUL
echo WAITING FOR PDB > lock.tmp

echo COMPILING DLL
cl %commonFlagsCompiler% -DTRANSLATION_UNIT_INDEX=0 ..\code\ast.cpp -LD /link %commonFlagsLinker% -PDB:game_%random%.pdb -EXPORT:Game_UpdateRender -EXPORT:Game_DebugFrameEnd -EXPORT:Game_InitialiseDebugState
del lock.tmp

popd
