/*
Project: Asteroids
File: ast_platform.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_PLATFORM_H

#include <stdio.h>

#include "ast_utility.h"

#if AST_INTERNAL
struct Debug_ReadFileResult
{
    u64 contentsSize;
    void *contents;
};

#define DEBUG_PLATFORM_READ_FILE(funcName) Debug_ReadFileResult funcName(char *filename)
typedef DEBUG_PLATFORM_READ_FILE(debug_platformReadFile);

#define DEBUG_PLATFORM_FREE_FILE(funcName) void funcName(void *memory)
typedef DEBUG_PLATFORM_FREE_FILE(debug_platformFreeFile);

enum
{
    DebugCycleCounter_GameUpdateRender,
    DebugCycleCounter_RenderBitmap_Slow,
    DebugCycleCounter_ProcessPixel,
    
    DebugCycleCounter_Count,
};
struct DebugCycleCounter
{
    u64 cycleCount;
    u32 hitCount;
};

#if 1
#define BEGIN_TIMED_BLOCK(ID) u64 startCycleCount_##ID = __rdtsc();
#define END_TIMED_BLOCK(ID) debugGlobalMem->counters[DebugCycleCounter_##ID].cycleCount += __rdtsc() - startCycleCount_##ID; ++debugGlobalMem->counters[DebugCycleCounter_##ID].hitCount;
#define END_TIMED_BLOCK_COUNTED(ID, COUNT) debugGlobalMem->counters[DebugCycleCounter_##ID].cycleCount += __rdtsc() - startCycleCount_##ID; debugGlobalMem->counters[DebugCycleCounter_##ID].hitCount += COUNT;
#else
#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK_COUNTED(ID, COUNT)
#endif
#endif // AST_INTERNAL

#define PLATFORM_MEM_ALLOC(funcName) void *funcName(usize size)
typedef PLATFORM_MEM_ALLOC(platformMemAlloc);

#define PLATFORM_MEM_FREE(funcName) void funcName(void *memory)
typedef PLATFORM_MEM_FREE(platformMemFree);

typedef struct
{
    void *memory;
    s32 width;
    s32 height;
    u32 pitch;
} Game_BackBuffer;

typedef struct
{
    s32 samplesPerSecond;
    s32 sampleCount;
    s16 *samples;
} Game_AudioBuffer;

typedef struct
{
    b32 endedFrameDown;
    s32 halfTransitionCount;
} Game_ButtonState;

typedef struct
{
    b32 isConnected;
    
    union
    {
        Game_ButtonState keys[6];
        struct
        {
            Game_ButtonState keyW;
            Game_ButtonState keyA;
            Game_ButtonState keyD;
            Game_ButtonState keySpace;
            Game_ButtonState keyEsc;
            Game_ButtonState keyF1;
        };
    };
} Game_Keyboard;

typedef struct
{
    b32 exeReloaded;
    f32 deltaTime;
    
    Game_Keyboard keyboard;
} Game_Input;

typedef struct
{
    debug_platformFreeFile *Debug_FreeFile;
    debug_platformReadFile *Debug_ReadFile;
    
    platformMemAlloc *MemAlloc;
    platformMemFree *MemFree;
} PlatformAPI;

typedef struct
{
    u64 permaStorageSize;
    u64 transStorageSize;
    void *permaStorage;
    void *transStorage;
    
    PlatformAPI platform;
    
#if AST_INTERNAL
    DebugCycleCounter counters[DebugCycleCounter_Count];
#endif
} Game_Memory;

#if AST_INTERNAL
extern Game_Memory *debugGlobalMem;
#endif

#define GAME_UPDATE_RENDER(funcName) void funcName(Game_BackBuffer *backBuffer, Game_Memory *memory, Game_Input *input, Game_AudioBuffer *audioBuffer)
typedef GAME_UPDATE_RENDER(game_updateRender);

#define AST_PLATFORM_H
#endif //AST_PLATFORM_H
