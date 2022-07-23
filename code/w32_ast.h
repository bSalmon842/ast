/*
Project: Asteroids
File: w32_ast.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef W32_AST_H

struct W32_BackBuffer
{
    BITMAPINFO info;
    void *memory;
    s32 width;
    s32 height;
    u32 pitch;
};

struct W32_WindowDims
{
    u32 width;
    u32 height;
};

struct W32_AudioInfo
{
    ma_rb *ringBuffer;
    Game_Audio *gameAudio;
    game_fillAudioSamples *FillAudioSamples;
};

struct W32_DebugTimeMarker
{
    u32 playCursor;
    u32 writeCursor;
};

struct W32_FileHandle
{
    HANDLE w32Handle;
};

struct W32_FileGroup
{
    HANDLE findHandle;
    WIN32_FIND_DATAW findData;
};

#define W32_AST_H
#endif //W32_AST_H
