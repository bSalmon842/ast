@echo off

time /T
date /T
echo.

REM -MTd for debug build
set commonFlagsCompiler=-MTd -nologo -Gm- -GS- -GR- -fp:fast -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4996 -FC -Z7 -DAST_INTERNAL=1 -DAST_SLOW=1
set commonFlagsLinker= -incremental:no -opt:ref

IF NOT EXIST ..\build mkdir ..\build
IF NOT EXIST ..\build\asset_build mkdir ..\build\asset_build
IF NOT EXIST ..\build\preproc mkdir ..\build\preproc
pushd ..\build

pushd .\preproc
echo COMPILING PREPROC
cl %commonFlagsCompiler% ..\..\code\preproc\cmt_preproc.cpp /link %commonFlagsLinker%
popd

echo.

REM 64-bit
del *.pdb > NUL 2> NUL
echo WAITING FOR PDB > lock.tmp

echo COMPILING DLL
cl %commonFlagsCompiler% -DTRANSLATION_UNIT_INDEX=0 ..\code\ast.cpp -LD /link %commonFlagsLinker% -PDB:game_%random%.pdb -EXPORT:Game_UpdateRender -EXPORT:Game_DebugFrameEnd -EXPORT:Game_InitialiseDebugState
del lock.tmp

echo.

echo COMPILING PLATFORM
cl %commonFlagsCompiler% -DTRANSLATION_UNIT_INDEX=1 ..\code\w32_ast.cpp /link %commonFlagsLinker% user32.lib gdi32.lib ole32.lib winmm.lib opengl32.lib

echo.

pushd .\asset_build

echo COMPILING ASSET BUILDER
cl %commonFlagsCompiler% ..\..\code\asset_build\asset_build.cpp /link %commonFlagsLinker% gdi32.lib

popd
popd
