/*
Project: Asteroids
File: ast_asset.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_ASSET_H

enum BitmapID : u32
{
    BitmapID_Null,
    
    BitmapID_Player_NoTrail,
    BitmapID_Asteroid0,
    BitmapID_Asteroid1,
    BitmapID_Asteroid2,
    BitmapID_Asteroid3,
    BitmapID_UFO_Large,
};

#pragma pack(push, 1)
typedef struct
{
    v2s dims;
    u32 id;
    u32 handle;
} BitmapInfo;

typedef struct
{
    v2s dims;
    s32 baseline;
    char glyph;
    u32 handle;
} GlyphInfo;

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

enum AssetType : u8
{
    AssetType_Null,
    
    AssetType_Bitmap,
    AssetType_Audio,
    AssetType_Glyph,
    
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

#define AST_ASSET_H
#endif //AST_ASSET_H
