/*
Project: Asteroids
File: w32_ast.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef W32_AST_H

typedef struct
{
    BITMAPINFO info;
    void *memory;
    s32 width;
    s32 height;
    u32 pitch;
} W32_BackBuffer;

typedef struct
{
    s32 width;
    s32 height;
} W32_WindowDims;

typedef struct
{
    IAudioClient *audioClient;
    IAudioRenderClient *renderClient;
    s32 samplesPerSecond;
    s32 bytesPerSample;
    s32 secondaryBufferSize;
    s32 latencySampleCount;
    u32 runningSampleIndex;
} W32_AudioOutput;

typedef struct
{
    u32 playCursor;
    u32 writeCursor;
} W32_DebugTimeMarker;

typedef struct
{
    HANDLE w32Handle;
} W32_FileHandle;

typedef struct
{
    HANDLE findHandle;
    WIN32_FIND_DATAW findData;
} W32_FileGroup;

#define W32_AST_H
#endif //W32_AST_H
