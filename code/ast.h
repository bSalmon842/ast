/*
Project: Asteroids
File: ast.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_H

#include "ast_platform.h"
#include "bs842_vector.h"
#include "ast_memory.h"

#define RND_IMPLEMENTATION
#include "rnd.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

Make2DStruct(f32, v2f, V2F);
Make2DStruct(s32, v2s, V2S);
Make3DStruct(f32, v3f, V3F);
Make4DStruct(f32, v4f, V4F, v3f);
MakeRectStruct(v2f, Rect2f);

#include "ast_entity.h"

typedef struct
{
    v2s dims;
    void *memory;
    s32 pitch;
} Bitmap;

#pragma pack(push, 1)
typedef struct
{
    u16 fileType;
    u32 fileSize;
    u16 reserved1;
    u16 reserved2;
    u32 bitmapOffset;
    
    u32 size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bitsPerPixel;
    u32 compression;
    u32 sizeOfBitmap;
    s32 horzResolution;
    s32 vertResolution;
    u32 coloursUsed;
    u32 coloursImportant;
    
    u32 redMask;
    u32 greenMask;
    u32 blueMask;
} BitmapHeader;
#pragma pack(pop)

typedef struct
{
    b32 found;
    u32 index;
} BitScanResult;

typedef struct 
{
    b8 sse3;
    b8 sse4_2;
    b8 avx;
} InstructionSet;

typedef struct
{
    b32 initialised;
    
    b32 paused;
    
    v2f worldDims;
    
    rnd_pcg_t pcg;
    
    Entity entities[128];
    Entity *playerEntity;
    v2f playerDDP;
    
    u8 asteroidCount;
    u8 shotCount;
    u8 maxShots;
    
    u32 score;
    
    Bitmap asteroidBitmaps[4];
    Bitmap playerBitmap;
    
    stbtt_fontinfo gameFont;
    
    InstructionSet availableInstructions;
} Game_State;

typedef struct
{
    b32 initialised;
    
    MemoryRegion transRegion;
} Transient_State;

#define AST_H
#endif //AST_H
