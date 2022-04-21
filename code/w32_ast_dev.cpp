/*
Project: Asteroids
File: w32_ast_dev.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#define W32_FILEPATH_MAX_LENGTH 0x104

struct W32_State
{
    char exeName[W32_FILEPATH_MAX_LENGTH];
    char *exeNameBegin;
};

struct W32_ProgramCode
{
    HMODULE programCodeDLL;
    FILETIME dllLastWriteTime;
    
    game_updateRender *UpdateRender;
    game_debugFrameEnd *DebugFrameEnd;
    game_initialiseDebugState *InitialiseDebugState;
    
    b32 isValid;
};

function void W32_GetExeFileName(W32_State *state)
{
	DWORD filenameSize = GetModuleFileNameA(0, state->exeName, sizeof(state->exeName));
	state->exeNameBegin = state->exeName;
	for (char *scan = state->exeName; *scan; ++scan)
	{
		if(*scan == '\\')
        {
            state->exeNameBegin = scan + 1;
        }
    }
}

function void W32_BuildExePathFileName(W32_State *state, char *filename, char *dest)
{
    ConcatStrings(dest, state->exeName, state->exeNameBegin - state->exeName,
                  filename, StringLength(filename));
    
}

inline FILETIME W32_GetFileLastWriteTime(char *filename)
{
    FILETIME result = {};
    
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(filename, GetFileExInfoStandard, &data))
    {
        result = data.ftLastWriteTime;
    }
    
    return result;
}

function W32_ProgramCode W32_LoadProgramCode(char *dllName, char *tempDLLName, char *lockFileName)
{
    W32_ProgramCode result = {};
    
    WIN32_FILE_ATTRIBUTE_DATA ignored;
    if (!GetFileAttributesEx(lockFileName, GetFileExInfoStandard, &ignored))
    {
        result.dllLastWriteTime = W32_GetFileLastWriteTime(dllName);
        
        CopyFileA(dllName, tempDLLName, FALSE);
        result.programCodeDLL = LoadLibrary(tempDLLName);
        if (result.programCodeDLL)
        {
            result.UpdateRender = (game_updateRender *)GetProcAddress(result.programCodeDLL, "Game_UpdateRender");
            result.DebugFrameEnd = (game_debugFrameEnd *)GetProcAddress(result.programCodeDLL, "Game_DebugFrameEnd");
            result.InitialiseDebugState = (game_initialiseDebugState *)GetProcAddress(result.programCodeDLL, "Game_InitialiseDebugState");
            
            result.isValid = (result.UpdateRender && true);
        }
    }
    
    if (!result.isValid)
    {
        result.UpdateRender = 0;
        result.DebugFrameEnd = 0;
        result.InitialiseDebugState = 0;
    }
    
    return result;
}

function void W32_UnloadProgramCode(W32_ProgramCode *programCode)
{
    if(programCode->programCodeDLL)
    {
        FreeLibrary((HMODULE)programCode->programCodeDLL);
    }
    
    programCode->programCodeDLL = 0;
    programCode->isValid = false;
    programCode->UpdateRender = 0;
    programCode->DebugFrameEnd = 0;
    programCode->InitialiseDebugState = 0;
}