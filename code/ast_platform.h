/*
Project: Asteroids
File: ast_platform.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_PLATFORM_H

#include <stdio.h>

#include "ast_utility.h"

typedef struct
{
    b32 errored;
    void *platform;
} Platform_FileHandle;

typedef struct
{
    u32 fileCount;
    void *platform;
} Platform_FileGroup;

enum Platform_FileType
{
    PlatformFileType_Asset,
    PlatformFileType_Save,
};

struct Platform_ParallelQueue;

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
    
    platformAddParallelEntry *AddParallelEntry;
    platformCompleteAllParallelWork *CompleteAllParallelWork;
    platformHasParallelWorkFinished *HasParallelWorkFinished;
    
    platformAllocTexture *AllocTexture;
    platformFreeTexture *FreeTexture;
};

struct Game_RenderCommands
{
    u32 width;
    u32 height;
    
    u8 *pushBufferBase;
    usize pushBufferSize;
    usize maxPushBufferSize;
    
    u32 entryCount;
    
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
} Game_Keyboard;

struct Game_Input
{
    b32 exeReloaded;
    f32 deltaTime;
    
    Game_Keyboard keyboard;
};

struct InstructionSets
{
    b8 sse3;
    b8 sse4_2;
    b8 avx;
};

struct Game_Memory
{
    u64 permaStorageSize;
    u64 transStorageSize;
    u64 debugStorageSize;
    void *permaStorage;
    void *transStorage;
    void *debugStorage;
    
    InstructionSets availableInstructionSets;
    
    Platform_ParallelQueue *parallelQueue;
    PlatformAPI platform;
};

#if AST_INTERNAL
extern Game_Memory *debugGlobalMem;
#endif

struct DebugFrameTimestamp
{
    char *name;
    f32 time;
};

struct DebugFrameInfo
{
    u32 timestampCount;
    DebugFrameTimestamp timestamps[64];
};

inline void DebugRecordTimestamp(DebugFrameInfo *frame, char *name, f32 time)
{
    ASSERT(frame->timestampCount < ARRAY_COUNT(frame->timestamps));
    DebugFrameTimestamp *timestamp = &frame->timestamps[frame->timestampCount++];
    timestamp->name = name;
    timestamp->time = time;
}

#define GAME_DEBUG_FRAME_END(funcName) void funcName(Game_Memory *memory, DebugFrameInfo *frame)
typedef GAME_DEBUG_FRAME_END(game_debugFrameEnd);

#define GAME_UPDATE_RENDER(funcName) void funcName(Game_RenderCommands *renderCommands, Game_Memory *memory, Game_Input *input, Game_AudioBuffer *audioBuffer)
typedef GAME_UPDATE_RENDER(game_updateRender);

#define AST_PLATFORM_H
#endif //AST_PLATFORM_H
