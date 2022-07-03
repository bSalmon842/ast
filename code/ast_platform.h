/*
Project: Asteroids
File: ast_platform.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_PLATFORM_H

#include <stdio.h>

#include "ast_debug_config.h"

#include "ast_utility.h"
#include "ast_memory.h"

struct Platform_FileHandle
{
    b32 errored;
    void *platform;
};

struct Platform_FileGroup
{
    u32 fileCount;
    void *platform;
};

enum Platform_FileType
{
    PlatformFileType_Asset,
    PlatformFileType_Save,
};

enum Platform_WriteType
{
    PlatformWriteType_Overwrite,
    PlatformWriteType_Append,
};

struct Platform_ParallelQueue;

#define PLATFORM_DEBUG_SYSTEM_COMMAND(funcName) void funcName(char *command, char *path)
typedef PLATFORM_DEBUG_SYSTEM_COMMAND(platformDebugSystemCommand);

#define PLATFORM_GET_ALL_FILES_OF_EXT_BEGIN(funcName) Platform_FileGroup funcName(Platform_FileType fileType)
typedef PLATFORM_GET_ALL_FILES_OF_EXT_BEGIN(platformGetAllFilesOfExtBegin);

#define PLATFORM_GET_ALL_FILES_OF_EXT_END(funcName) void funcName(Platform_FileGroup *fileGroup)
typedef PLATFORM_GET_ALL_FILES_OF_EXT_END(platformGetAllFilesOfExtEnd);

#define PLATFORM_OPEN_NEXT_FILE(funcName) Platform_FileHandle funcName(Platform_FileGroup *fileGroup)
typedef PLATFORM_OPEN_NEXT_FILE(platformOpenNextFile);

#define PLATFORM_MARK_FILE_ERROR(funcName) void funcName(Platform_FileHandle *fileHandle, char *errorMsg)
typedef PLATFORM_MARK_FILE_ERROR(platformMarkFileError);

#define PLATFORM_READ_DATA_FROM_FILE(funcName) void funcName(Platform_FileHandle *fileHandle, usize offset, usize size, void *dest)
typedef PLATFORM_READ_DATA_FROM_FILE(platformReadDataFromFile);

#define PLATFORM_OPEN_FILE_FOR_WRITE(funcName) Platform_FileHandle funcName(char *filename, Platform_WriteType writeType)
typedef PLATFORM_OPEN_FILE_FOR_WRITE(platformOpenFileForWrite);

#define PLATFORM_WRITE_INTO_FILE(funcName) void funcName(Platform_FileHandle fileHandle, char *fmtStr, ...)
typedef PLATFORM_WRITE_INTO_FILE(platformWriteIntoFile);

#define PLATFORM_CLOSE_FILE(funcName) void funcName(Platform_FileHandle *fileHandle)
typedef PLATFORM_CLOSE_FILE(platformCloseFile);

#define PLATFORM_MEM_ALLOC(funcName) void *funcName(usize size)
typedef PLATFORM_MEM_ALLOC(platformMemAlloc);

#define PLATFORM_MEM_FREE(funcName) void funcName(void *memory)
typedef PLATFORM_MEM_FREE(platformMemFree);

#define PLATFORM_MICROSECONDS_SINCE_EPOCH(funcName) u64 funcName()
typedef PLATFORM_MICROSECONDS_SINCE_EPOCH(platformMicrosecondsSinceEpoch);

#define PLATFORM_SECONDS_SINCE_EPOCH(funcName) u64 funcName()
typedef PLATFORM_SECONDS_SINCE_EPOCH(platformSecondsSinceEpoch);

#define PLATFORM_ALLOC_TEXTURE(funcName) void funcName(u32 *handle, s32 width, s32 height, void *data)
typedef PLATFORM_ALLOC_TEXTURE(platformAllocTexture);

#define PLATFORM_FREE_TEXTURE(funcName) void funcName(u32 textureHandle)
typedef PLATFORM_FREE_TEXTURE(platformFreeTexture);

#define PARALLEL_WORK_CALLBACK(funcName) void funcName(void *data)
typedef PARALLEL_WORK_CALLBACK(parallelWorkCallback);

#define PLATFORM_ADD_PARALLEL_ENTRY(funcName) void funcName(Platform_ParallelQueue *queue, parallelWorkCallback *callback, void *data)
typedef PLATFORM_ADD_PARALLEL_ENTRY(platformAddParallelEntry);

#define PLATFORM_COMPLETE_ALL_PARALLEL_WORK(funcName) void funcName(Platform_ParallelQueue *queue)
typedef PLATFORM_COMPLETE_ALL_PARALLEL_WORK(platformCompleteAllParallelWork);

#define PLATFORM_HAS_PARALLEL_WORK_FINISHED(funcName) b32 funcName(Platform_ParallelQueue *queue)
typedef PLATFORM_HAS_PARALLEL_WORK_FINISHED(platformHasParallelWorkFinished);

struct ParallelWorkEntry
{
    parallelWorkCallback *callback;
    void *data;
};

struct Platform_ParallelQueue
{
    u32 volatile nextEntryToRead;
    u32 volatile nextEntryToWrite;
    
    u32 volatile completionTarget;
    u32 volatile completedCount;
    
    ParallelWorkEntry entries[256];
    
    void *semaphore;
};

inline b32 Platform_NoFileErrors(Platform_FileHandle *fileHandle)
{
    b32 result = (!fileHandle->errored);
    return result;
}

struct PlatformAPI
{
    platformMemAlloc *MemAlloc;
    platformMemFree *MemFree;
    
    platformMicrosecondsSinceEpoch *MicrosecondsSinceEpoch;
    platformSecondsSinceEpoch *SecondsSinceEpoch;
    
    platformGetAllFilesOfExtBegin *GetAllFilesOfExtBegin;
    platformGetAllFilesOfExtEnd *GetAllFilesOfExtEnd;
    platformOpenNextFile *OpenNextFile;
    platformMarkFileError *MarkFileError;
    platformReadDataFromFile *ReadDataFromFile;
    
    platformOpenFileForWrite *OpenFileForWrite;
    platformWriteIntoFile *WriteIntoFile;
    platformCloseFile *CloseFile;
    
    platformAddParallelEntry *AddParallelEntry;
    platformCompleteAllParallelWork *CompleteAllParallelWork;
    platformHasParallelWorkFinished *HasParallelWorkFinished;
    
    platformAllocTexture *AllocTexture;
    platformFreeTexture *FreeTexture;
    
    platformDebugSystemCommand *DebugSystemCommand;
};

struct Game_RenderCommands
{
    u32 width;
    u32 height;
    
    u8 *pushBufferBase;
    usize pushBufferSize;
    usize maxPushBufferSize;
    
    u32 entryCount;
    struct RenderEntry_Header *headers[10000];
    
    MemoryRegion renderRegion;
    TempMemory renderTemp;
    
    struct Game_LoadedAssets *loadedAssets;
};

inline Game_RenderCommands InitialiseRenderCommands(usize maxPushBufferSize, void *pushBufferBase, u32 width, u32 height)
{
    Game_RenderCommands result = {};
    
    result.width = width;
    result.height = height;
    
    result.pushBufferBase = (u8 *)pushBufferBase;
    result.pushBufferSize = 0;
    result.maxPushBufferSize = maxPushBufferSize;
    
    result.entryCount = 0;
    
    result.loadedAssets = 0;
    
    return result;
}

struct Game_AudioBuffer
{
    s32 samplesPerSecond;
    s32 sampleCount;
    s16 *samples;
};

struct Game_ButtonState
{
    b32 endedFrameDown;
    s32 halfTransitionCount;
};

struct Game_Keyboard
{
    b32 isConnected;
    
    union
    {
        Game_ButtonState keys[14];
        struct
        {
            Game_ButtonState keyW;
            Game_ButtonState keyA;
            Game_ButtonState keyD;
            Game_ButtonState keySpace;
            Game_ButtonState keyEsc;
            Game_ButtonState keyF1;
            Game_ButtonState keyF2;
            Game_ButtonState keyF3;
            Game_ButtonState keyF4;
            Game_ButtonState keyF5;
            
            Game_ButtonState keyUp;
            Game_ButtonState keyDown;
            Game_ButtonState keyLeft;
            Game_ButtonState keyRight;
        };
    };
};

enum MouseButtons
{
    MouseButton_L,
    MouseButton_M,
    MouseButton_R,
};

struct Game_Mouse
{
    s32 x;
    s32 y;
    Game_ButtonState buttons[3];
};

struct Game_Input
{
    b32 exeReloaded;
    f32 deltaTime;
    
    Game_Mouse mouse;
    Game_Keyboard keyboard;
};

inline b32 InputNoRepeat(Game_ButtonState buttonState)
{
    b32 result = buttonState.endedFrameDown && (buttonState.halfTransitionCount % 2 != 0);
    return result;
}

struct InstructionSets
{
    b8 sse3;
    b8 sse4_2;
    b8 avx;
};

#define STORAGE_COUNT 3
#define PERMA_STORAGE_INDEX 0
#define TRANS_STORAGE_INDEX 1
#define DEBUG_STORAGE_INDEX 2
struct StorageInfo
{
    u64 size;
    void *ptr;
    
    MemoryRegion *regions[64];
    u32 regionCount;
};

struct Game_Memory
{
    StorageInfo storage[STORAGE_COUNT];
    
    InstructionSets availableInstructionSets;
    
    Platform_ParallelQueue *parallelQueue;
    PlatformAPI platform;
};

#if AST_INTERNAL
extern Game_Memory *debugGlobalMem;

#define introspect(x)

#include "ast_intrinsics.h"

struct DebugState;
global DebugState *globalDebugState;

enum Debug_VarType
{
    Debug_VarType_b32,
    Debug_VarType_s32,
    Debug_VarType_f32,
    Debug_VarType_v2f,
    Debug_VarType_v3f,
};

inline void GetDebugFormatString(char *fmt, Debug_VarType varType)
{
    switch (varType)
    {
        case Debug_VarType_s32:
        {
            stbsp_sprintf(fmt, "%%s: %%d");
        } break;
        
        case Debug_VarType_b32:
        {
            stbsp_sprintf(fmt, "%%s: %%s (%%d)");
        } break;
        
        case Debug_VarType_f32:
        {
            stbsp_sprintf(fmt, "%%s: %%.03f");
        } break;
        
        case Debug_VarType_v2f:
        {
            stbsp_sprintf(fmt, "%%s: { %%.03f, %%.03f }");
        } break;
        
        case Debug_VarType_v3f:
        {
            stbsp_sprintf(fmt, "%%s: { %%.03f, %%.03f, %%.03f }");
        } break;
        
        INVALID_DEFAULT;
    }
}

// NOTE(bSalmon): Platform Layer Only
#define DEBUG_FRAME_START \
ASSERT(TRANSLATION_UNIT_INDEX == 1); \
globalDebugState->inFrame = true; \
u32 frameIndex = (globalDebugState->table->frameIndex + 1 >= MAX_DEBUG_FRAMES) ? 0 : ++globalDebugState->table->frameIndex; \
globalDebugState->table->frameIndex = frameIndex; \
DebugFrame *currentFrame = &globalDebugState->table->frames[globalDebugState->table->frameIndex]; \
currentFrame->startClock = __rdtsc(); \
currentFrame->totalClock = currentFrame->startClock; 

#define DEBUG_FRAME_END(time) \
ASSERT(TRANSLATION_UNIT_INDEX == 1 && globalDebugState->inFrame); \
currentFrame->totalClock = __rdtsc() - currentFrame->startClock; \
currentFrame->frameTime = time; \
globalDebugState->inFrame = false

// NOTE(bSalmon): All Layers
#define DEBUG_BLOCK_OPEN_(blockName, block) \
if (globalDebugState->inFrame) \
{ \
DebugBlockInfo *blockInfo = &globalDebugState->table->blockInfos[TRANSLATION_UNIT_INDEX][block]; \
blockInfo->cycles -= __rdtsc(); \
blockInfo->hits++; \
blockInfo->name = blockName; \
blockInfo->isMarker = false; \
}

#define DEBUG_BLOCK_OPEN(blockName) u8 block_##blockName = __COUNTER__; DEBUG_BLOCK_OPEN_(#blockName, block_##blockName)

#define DEBUG_FRAME_MARKER(markerName) \
if (globalDebugState->inFrame) \
{ \
DebugFrame *frame = &globalDebugState->table->frames[globalDebugState->table->frameIndex]; \
DebugBlockInfo *blockInfo = &globalDebugState->table->blockInfos[TRANSLATION_UNIT_INDEX][__COUNTER__]; \
blockInfo->cycles = __rdtsc() - frame->totalClock; \
frame->totalClock += blockInfo->cycles; \
blockInfo->hits++; \
blockInfo->name = #markerName; \
blockInfo->isMarker = true; \
}

#define DEBUG_BLOCK_CLOSE_(block) \
if (globalDebugState->inFrame) \
{ \
DebugBlockInfo *blockInfo = &globalDebugState->table->blockInfos[TRANSLATION_UNIT_INDEX][block]; \
blockInfo->cycles += __rdtsc(); \
}

#define DEBUG_BLOCK_CLOSE(blockName) DEBUG_BLOCK_CLOSE_(block_##blockName)

#define DEBUG_BLOCK_AUTO__(name, line) DebugAutoBlock autoBlock_##line(__COUNTER__, name)
#define DEBUG_BLOCK_AUTO_(name, line) DEBUG_BLOCK_AUTO__(name, line)
#define DEBUG_BLOCK_AUTO(name) DEBUG_BLOCK_AUTO_(#name, __LINE__)
#define DEBUG_BLOCK_FUNC DEBUG_BLOCK_AUTO_(__FUNCTION__, __LINE__)
#endif

#define GAME_INITIALISE_DEBUG_STATE(funcName) void funcName(Game_Memory *memory)
typedef GAME_INITIALISE_DEBUG_STATE(game_initialiseDebugState);

#define GAME_DEBUG_FRAME_END(funcName) void funcName(Game_Memory *memory)
typedef GAME_DEBUG_FRAME_END(game_debugFrameEnd);

#define GAME_UPDATE_RENDER(funcName) void funcName(Game_RenderCommands *renderCommands, Game_Memory *memory, Game_Input *input, Game_AudioBuffer *audioBuffer)
typedef GAME_UPDATE_RENDER(game_updateRender);

#define AST_PLATFORM_H
#endif //AST_PLATFORM_H
