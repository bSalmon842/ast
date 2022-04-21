/*
Project: Asteroids
File: ast_asset.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_ASSET_H

#include "ast_platform.h"

enum BitmapID : u32
{
    BitmapID_Null,
    
    BitmapID_Player_NoTrail,
    BitmapID_Player_Trail,
    BitmapID_Asteroid0,
    BitmapID_Asteroid1,
    BitmapID_Asteroid2,
    BitmapID_Asteroid3,
    BitmapID_UFO_Large,
    
    BitmapID_Count,
};

struct GlyphIdentifier
{
    char glyph;
    char *font;
};

#pragma pack(push, 1)
typedef struct
{
    v2s dims;
    v2f align;
    u32 id;
} BitmapInfo;

typedef struct
{
    v2s dims;
    v2f align;
    char glyph;
    
    char font[32];
} GlyphInfo;

struct Kerning
{
    f32 advance;
    char codepoint0;
    char codepoint1;
};

struct KerningTableInfo
{
    u32 infoCount;
    char font[32];
};

struct FontMetadata
{
    f32 lineGap;
    b32 monospace;
    b32 allCapital;
    
    char font[32];
};

struct Bitmap
{
    BitmapInfo info;
    void *memory;
};

struct Glyph
{
    GlyphInfo info;
    void *memory;
};

struct KerningTable
{
    KerningTableInfo info;
    Kerning *table;
};

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

typedef struct
{
    b32 found;
    u32 index;
} BitScanResult;

#define AAF_CODE(a, b, c, d) ((u32)(a) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))
#define MAGIC_AAF AAF_CODE('B', 'A', 'A', 'F');
typedef struct
{
    u32 magicValue;
    u32 assetCount;
    usize dataSize;
} AAFHeader;

enum AssetLoadState
{
    AssetLoad_Unloaded,
    AssetLoad_Loading,
    AssetLoad_Loaded,
};

enum AssetType : u8
{
    AssetType_Null,
    
    AssetType_Bitmap,
    AssetType_Audio,
    AssetType_Glyph,
    AssetType_KerningTable,
    AssetType_FontMetadata,
    
    AssetType_Count,
};

typedef struct
{
    usize assetSize;
    AssetType type;
    
    union
    {
        BitmapInfo bitmap;
        GlyphInfo glyph;
        KerningTableInfo kerning;
        FontMetadata metadata;
        // Audio audio;
    };
} AssetHeader;
#pragma pack(pop)

typedef struct
{
    u32 assetCount;
    AssetHeader *headers;
    void *assetData;
    
    usize assetDataSize; // NOTE(bSalmon): Only really useful for the Asset Builder
} Game_Assets;

struct LoadedAssetHeader
{
    Platform_FileHandle fileHandle;
    
    AssetType type;
    AssetLoadState volatile loadState;
    u32 textureHandle;
    
    union
    {
        BitmapInfo bitmap;
        GlyphInfo glyph;
        KerningTableInfo kerning;
        FontMetadata metadata;
        // Audio audio;
    };
    
    usize assetSize;
    usize assetOffset;
    void *asset;
};

struct Game_LoadedAssets
{
    u32 assetCount;
    LoadedAssetHeader *headers;
    
    PlatformAPI platform;
    struct Transient_State *transState;
    Platform_ParallelQueue *parallelQueue;
};

#define AST_ASSET_H
#endif //AST_ASSET_H
