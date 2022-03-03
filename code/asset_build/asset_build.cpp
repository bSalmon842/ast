/*
Project: Asteroids
File: asset_build.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

#include "..\bs842_vector.h"
#include "..\ast_utility.h"

Make2DStruct(f32, v2f, V2F);
Make2DStruct(s32, v2s, V2S);
Make3DStruct(f32, v3f, V3F);
Make4DStruct(f32, v4f, V4F, v3f);

#include "..\ast_math.h"
#include "..\ast_asset.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "..\stb_truetype.h"

typedef struct
{
    usize contentsSize;
    void *contents;
} ReadFileResult;

inline BitScanResult FindLeastSignificantSetBit(u32 value)
{
    BitScanResult result = {};
    
    for (u32 test = 0; test < 32; ++test)
    {
        if (value & (1 << test))
        {
            result.index = test;
            result.found = true;
            break;
        }
    }
    
    return result;
}

function ReadFileResult ReadFile(char *filename)
{
    ReadFileResult result = {};
    
    FILE *file = fopen(filename, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        result.contentsSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        result.contents = calloc(result.contentsSize, 1);
        fread(result.contents, result.contentsSize, 1, file);
        
        fclose(file);
    }
    
    return result;
}

function void AddAsset(Game_Assets *assets, AssetHeader assetHeader, void *assetData)
{
    assets->assetCount++;
    assets->headers = (AssetHeader *)realloc(assets->headers, sizeof(AssetHeader) * assets->assetCount);
    assets->headers[assets->assetCount - 1] = assetHeader;
    
    assets->assetData = realloc(assets->assetData, assets->assetDataSize + assetHeader.assetSize);
    memcpy((u8 *)assets->assetData + assets->assetDataSize, assetData, assetHeader.assetSize);
    assets->assetDataSize += assetHeader.assetSize;
}

function AssetHeader MakeAssetHeader_Bitmap(usize assetSize, AssetType type, BitmapInfo bitmap)
{
    AssetHeader result = {};
    
    result.assetSize = assetSize;
    result.type = type;
    result.bitmap = bitmap;
    
    return result;
}

function AssetHeader MakeAssetHeader_Glyph(usize assetSize, AssetType type, GlyphInfo glyph)
{
    AssetHeader result = {};
    
    result.assetSize = assetSize;
    result.type = type;
    result.glyph = glyph;
    
    return result;
}

function void AddToAssets_Bitmap(Game_Assets *assets, char *filename, BitmapID id)
{
    Bitmap bitmap = {};
    
    ReadFileResult readResult = ReadFile(filename);
    ASSERT(readResult.contents);
    
    BitmapHeader *header = (BitmapHeader *)readResult.contents;
    u32 *pixels = (u32 *)((u8 *)readResult.contents + header->bitmapOffset);
    bitmap.memory = pixels;
    
    bitmap.info.dims = V2S(header->width, header->height);
    bitmap.info.id = id;
    bitmap.info.handle = 0;
    
    ASSERT(bitmap.info.dims.h >= 0);
    ASSERT(header->compression == 3);
    
    u32 redMask = header->redMask;
    u32 greenMask = header->greenMask;
    u32 blueMask = header->blueMask;
    u32 alphaMask = ~(redMask | greenMask | blueMask);
    
    BitScanResult redShift = FindLeastSignificantSetBit(redMask);
    BitScanResult greenShift = FindLeastSignificantSetBit(greenMask);
    BitScanResult blueShift = FindLeastSignificantSetBit(blueMask);
    BitScanResult alphaShift = FindLeastSignificantSetBit(alphaMask);
    
    ASSERT(redShift.found);
    ASSERT(greenShift.found);
    ASSERT(blueShift.found);
    ASSERT(alphaShift.found);
    
    v2s min = V2S(header->width, header->height);
    v2s max = V2S();
    for (s32 y = 0; y < header->height; ++y)
    {
        for (s32 x = 0; x < header->width; ++x)
        {
            u32 pixel = *(pixels + (y * header->width) + x);
            v4f texel = V4F((f32)((pixel >> redShift.index) & 0xFF),
                            (f32)((pixel >> greenShift.index) & 0xFF),
                            (f32)((pixel >> blueShift.index) & 0xFF),
                            (f32)((pixel >> alphaShift.index) & 0xFF));
            if (texel.a > 0.0f)
            {
                if (x < min.x)
                {
                    min.x = x;
                }
                if (y < min.y)
                {
                    min.y = y;
                }
                if (x > max.x)
                {
                    max.x = x;
                }
                if (y > max.y)
                {
                    max.y = y;
                }
            }
        }
    }
    
    max += 1;
    bitmap.info.dims = max - min;
    
    bitmap.memory = calloc(1, (bitmap.info.dims.h * bitmap.info.dims.w) * BITMAP_BYTES_PER_PIXEL);
    
    u32 *srcDest = pixels;
    u32 *outPixel = (u32 *)bitmap.memory;
    for (s32 y = min.y; y < max.y; ++y)
    {
        for (s32 x = min.x; x < max.x; ++x)
        {
            u32 pixel = *(srcDest + (y * header->width) + x);
            v4f texel = V4F((f32)((pixel >> redShift.index) & 0xFF),
                            (f32)((pixel >> greenShift.index) & 0xFF),
                            (f32)((pixel >> blueShift.index) & 0xFF),
                            (f32)((pixel >> alphaShift.index) & 0xFF));
            
            texel.r = Sq(texel.r / 255.0f);
            texel.g = Sq(texel.g / 255.0f);
            texel.b = Sq(texel.b / 255.0f);
            texel.a = texel.a / 255.0f;
            
            texel.rgb *= texel.a;
            
            texel.r = SqRt(texel.r) * 255.0f;
            texel.g = SqRt(texel.g) * 255.0f;
            texel.b = SqRt(texel.b) * 255.0f;
            texel.a = texel.a * 255.0f;
            
            *outPixel++ = (RoundF32ToU32(texel.a) << 24 |
                           RoundF32ToU32(texel.r) << 16 |
                           RoundF32ToU32(texel.g) << 8 |
                           RoundF32ToU32(texel.b));
        }
    }
    
    free(readResult.contents);
    
    s32 pitch = bitmap.info.dims.w * BITMAP_BYTES_PER_PIXEL;
    AssetHeader assetHeader = MakeAssetHeader_Bitmap((usize)(bitmap.info.dims.h * pitch), AssetType_Bitmap, bitmap.info);
    AddAsset(assets, assetHeader, bitmap.memory);
}

function void AddToAssets_Font(Game_Assets *assets, char *fontFilename, char start, char end)
{
    ReadFileResult readResult = ReadFile(fontFilename);
    ASSERT(readResult.contents);
    
    b32 fontInitResult = false;
    stbtt_fontinfo fontInfo = {};
    fontInitResult = stbtt_InitFont(&fontInfo, (u8 *)readResult.contents, 0);
    ASSERT(fontInitResult);
    
    for (u8 glyphIndex = start; glyphIndex <= end; ++glyphIndex)
    {
        Glyph glyph = {};
        glyph.info.glyph = glyphIndex;
        
        f32 scale = stbtt_ScaleForPixelHeight(&fontInfo, 32.0f);
        s32 ascent = RoundF32ToS32((f32)(ttSHORT(fontInfo.data + fontInfo.hhea + 4)) * scale);
        
        s32 charWidth, lsb;
        stbtt_GetCodepointHMetrics(&fontInfo, glyphIndex, &charWidth, &lsb);
        
        s32 x0, x1, y0, y1;
        stbtt_GetGlyphBitmapBoxSubpixel(&fontInfo, stbtt_FindGlyphIndex(&fontInfo, glyphIndex), scale, scale, 0.0f, 0.0f,
                                        &x0, &y0, &x1, &y1);
        
        glyph.info.dims = V2S((x1 - x0) + 1, (y1 - y0) + 1);
        glyph.info.baseline = ascent + y0;
        
        glyph.memory = calloc(1, glyph.info.dims.w * glyph.info.dims.h * BITMAP_BYTES_PER_PIXEL);
        u8 *tempMem = (u8 *)calloc(1, glyph.info.dims.w * glyph.info.dims.h);
        stbtt_MakeCodepointBitmap(&fontInfo, tempMem, glyph.info.dims.w, glyph.info.dims.h, glyph.info.dims.w, scale, scale, glyphIndex);
        
        u8 *src = tempMem;
        u32 *outPixel = (u32 *)glyph.memory;
        for (s32 y = 0; y < glyph.info.dims.h; ++y)
        {
            for (s32 x = 0; x < glyph.info.dims.w; ++x)
            {
                v4f texel = V4F(0xFF, 0xFF, 0xFF, *src++);
                
                texel.r = Sq(texel.r / 255.0f);
                texel.g = Sq(texel.g / 255.0f);
                texel.b = Sq(texel.b / 255.0f);
                texel.a = texel.a / 255.0f;
                
                texel.rgb *= texel.a;
                
                texel.r = SqRt(texel.r) * 255.0f;
                texel.g = SqRt(texel.g) * 255.0f;
                texel.b = SqRt(texel.b) * 255.0f;
                texel.a = texel.a * 255.0f;
                
                *outPixel++ = (RoundF32ToU32(texel.a) << 24 |
                               RoundF32ToU32(texel.r) << 16 |
                               RoundF32ToU32(texel.g) << 8 |
                               RoundF32ToU32(texel.b));
            }
        }
        
        if (glyphIndex == ' ')
        {
            glyph.info.dims = V2S(10, 1);
        }
        
        AssetHeader assetHeader = MakeAssetHeader_Glyph((usize)(glyph.info.dims.h * glyph.info.dims.w * BITMAP_BYTES_PER_PIXEL), AssetType_Glyph, glyph.info);
        AddAsset(assets, assetHeader, glyph.memory);
    }
    
    free(readResult.contents);
}

function void WriteAssetFile(Game_Assets assets, char *filename)
{
    AAFHeader header = {};
    header.magicValue = MAGIC_AAF;
    
    header.assetCount = assets.assetCount;
    header.dataSize = (header.assetCount * sizeof(AssetHeader)) + assets.assetDataSize;
    
    FILE *assetFile = fopen(filename, "wb");
    if (assetFile)
    {
        fwrite(&header, sizeof(AAFHeader), 1, assetFile);
        
        u8 *currDataPoint = (u8 *)assets.assetData;
        usize cumulativeSize = 0;
        for (u32 assetIndex = 0; assetIndex < header.assetCount; ++assetIndex)
        {
            AssetHeader assetHeader = assets.headers[assetIndex];
            fwrite(&assetHeader, sizeof(AssetHeader), 1, assetFile);
            fwrite(currDataPoint, assetHeader.assetSize, 1, assetFile);
            currDataPoint += assetHeader.assetSize;
            cumulativeSize += sizeof(AssetHeader) + assetHeader.assetSize;
            
            if (assetHeader.type == AssetType_Bitmap)
            {
                printf("Saving Bitmap %d to File \"%s\", Asset Size: %lld, Total Size: %lld\n", assetHeader.bitmap.id, filename, sizeof(AssetHeader) + assetHeader.assetSize, cumulativeSize);
            }
            else if (assetHeader.type == AssetType_Glyph)
            {
                printf("Saving Glyph %c to File \"%s\", Asset Size: %lld, Total Size: %lld\n", assetHeader.glyph.glyph, filename, sizeof(AssetHeader) + assetHeader.assetSize, cumulativeSize);
            }
        }
        
        fclose(assetFile);
    }
}

s32 main(s32 argc, char **argv)
{
    _chdir("c:\\work\\ast\\data");
    
    Game_Assets assets = {};
    
    AddToAssets_Bitmap(&assets, "player.bmp", BitmapID_Player_NoTrail);
    AddToAssets_Bitmap(&assets, "ast1.bmp", BitmapID_Asteroid0);
    AddToAssets_Bitmap(&assets, "ast2.bmp", BitmapID_Asteroid1);
    AddToAssets_Bitmap(&assets, "ast3.bmp", BitmapID_Asteroid2);
    AddToAssets_Bitmap(&assets, "ast4.bmp", BitmapID_Asteroid3);
    AddToAssets_Bitmap(&assets, "ufoL.bmp", BitmapID_UFO_Large);
    WriteAssetFile(assets, "graphics.aaf");
    
    printf("\n");
    assets = {}; // NOTE(bSalmon): Causes a mem leak but this is a dev only tool for small (2D) assets for now so it's not too bad
    
    // TODO(bSalmon): Can only support 1 font currently
    AddToAssets_Font(&assets, "HyperspaceBold.ttf", ' ', '~');
    WriteAssetFile(assets, "fonts.aaf");
    
    return 0;
}