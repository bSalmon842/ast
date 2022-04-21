/*
Project: Asteroids
File: asset_build.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#include <Windows.h>
#include <stdio.h>
#include <direct.h>

#include "..\ast_utility.h"

#define BS842_MAKE_STRUCTS
#include "..\bs842_vector.h"

#include "..\ast_math.h"
#include "..\ast_asset.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "..\stb_truetype.h"

#define DEFAULT_FONT_SIZE 128

struct ReadFileResult
{
    usize contentsSize;
    void *contents;
};

struct FontInfo
{
    stbtt_fontinfo font;
    b32 monospace;
    b32 allCapital;
    
    char fontName[64];
};

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
    assets->headers = (AssetHeader *)realloc(assets->headers, sizeof(AssetHeader) * (assets->assetCount + 1));
    assets->headers[assets->assetCount++] = assetHeader;
    
    if (assetHeader.assetSize > 0 && assetData)
    {
        assets->assetData = realloc(assets->assetData, assets->assetDataSize + assetHeader.assetSize);
        memcpy((u8 *)assets->assetData + assets->assetDataSize, assetData, assetHeader.assetSize);
        assets->assetDataSize += assetHeader.assetSize;
    }
}

function AssetHeader MakeAssetHeader_Bitmap(usize assetSize, BitmapInfo bitmap)
{
    AssetHeader result = {};
    
    result.assetSize = assetSize;
    result.type = AssetType_Bitmap;
    result.bitmap = bitmap;
    
    return result;
}

function void ClearAssets(Game_Assets *assets)
{
    assets->assetCount = 0;
    free(assets->headers);
    assets->headers = 0;
    free(assets->assetData);
    assets->assetData = 0;
    assets->assetDataSize = 0;
}

function AssetHeader MakeAssetHeader_Glyph(usize assetSize, GlyphInfo glyph)
{
    AssetHeader result = {};
    
    result.assetSize = assetSize;
    result.type = AssetType_Glyph;
    result.glyph = glyph;
    
    return result;
}

function AssetHeader MakeAssetHeader_KerningTable(KerningTableInfo kerning)
{
    AssetHeader result = {};
    
    result.assetSize = kerning.infoCount * sizeof(Kerning);
    result.type = AssetType_KerningTable;
    result.kerning = kerning;
    
    return result;
}

function AssetHeader MakeAssetHeader_FontMetadata(FontMetadata metadata)
{
    AssetHeader result = {};
    
    result.assetSize = 0;
    result.type = AssetType_FontMetadata;
    result.metadata = metadata;
    
    return result;
}

function void AddToAssets_Bitmap(Game_Assets *assets, char *filename, BitmapID id, v2f align = V2F(0.5f))
{
    Bitmap bitmap = {};
    
    ReadFileResult readResult = ReadFile(filename);
    ASSERT(readResult.contents);
    
    BitmapHeader *header = (BitmapHeader *)readResult.contents;
    u32 *pixels = (u32 *)((u8 *)readResult.contents + header->bitmapOffset);
    bitmap.memory = pixels;
    
    bitmap.info.dims = V2S(header->width, header->height);
    bitmap.info.id = id;
    
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
    
    --min;
    ++max;
    bitmap.info.dims = (max - min);
    bitmap.info.align = align;
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
    AssetHeader assetHeader = MakeAssetHeader_Bitmap((usize)(bitmap.info.dims.h * pitch), bitmap.info);
    AddAsset(assets, assetHeader, bitmap.memory);
}

function void AddToAssets_Glyph(Game_Assets *assets, FontInfo font, char codepoint, f32 fontSize)
{
    Glyph glyph = {};
    
    f32 scale = stbtt_ScaleForPixelHeight(&font.font, fontSize);
    
    if (codepoint != ' ')
    {
        v2s min = V2S();
        v2s max = V2S();
        if (font.monospace)
        {
            v2s unusedMin = V2S();
            v2s unusedMax = V2S();
            
            stbtt_GetCodepointBitmapBox(&font.font, '_', scale, scale, &min.x, &unusedMin.y, &max.x, &unusedMax.y);
            stbtt_GetCodepointBitmapBox(&font.font, codepoint, scale, scale, &unusedMin.x, &min.y, &unusedMax.x, &max.y);
        }
        else
        {
            stbtt_GetCodepointBitmapBox(&font.font, codepoint, scale, scale, &min.x, &min.y, &max.x, &max.y);
        }
        
        --min;
        ++max;
        glyph.info.dims = V2S((max.x - min.x) + 1, (max.y - min.y) + 1);
        u8 *codepointBitmap = (u8 *)calloc(1, glyph.info.dims.w * glyph.info.dims.h);
        
        stbtt_MakeCodepointBitmap(&font.font, codepointBitmap, glyph.info.dims.w, glyph.info.dims.h, glyph.info.dims.w,
                                  scale, scale, codepoint);
        
        glyph.info.align = V2F(0.0f, (f32)max.y / (f32)glyph.info.dims.h);
        glyph.info.glyph = codepoint;
        sprintf(glyph.info.font, "%s", font.fontName);
        glyph.memory = calloc(1, glyph.info.dims.w * glyph.info.dims.h * BITMAP_BYTES_PER_PIXEL);
        
        u8 *srcRow = codepointBitmap;
        u32 *outPixel = (u32 *)glyph.memory;
        for (s32 y = 0; y < glyph.info.dims.h; ++y)
        {
            u8 *srcPixel = srcRow;
            for (s32 x = 0; x < glyph.info.dims.w; ++x)
            {
                u8 alpha = *srcPixel++;
                v4f texel = V4F((f32)alpha);
                
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
            
            srcRow += glyph.info.dims.w;
        }
    }
    else
    {
        if (font.monospace)
        {
            v2s min = V2S();
            v2s max = V2S();
            stbtt_GetCodepointBitmapBox(&font.font, 'W', scale, scale, &min.x, &min.y, &max.x, &max.y);
            --min;
            ++max;
            glyph.info.dims = V2S(max.x - min.x + 1, 2);
        }
        else
        {
            glyph.info.dims = V2S(32, 2);
        }
        glyph.info.align = V2F();
        glyph.info.glyph = codepoint;
        sprintf(glyph.info.font, "%s", font.fontName);
        glyph.memory = calloc(1, glyph.info.dims.w * glyph.info.dims.h * BITMAP_BYTES_PER_PIXEL);
    }
    
    AssetHeader assetHeader = MakeAssetHeader_Glyph((usize)(glyph.info.dims.h * glyph.info.dims.w * BITMAP_BYTES_PER_PIXEL), glyph.info);
    AddAsset(assets, assetHeader, glyph.memory);
}

function void AddToAssets_KerningAndMetadata(Game_Assets *assets, FontInfo font, f32 fontSize)
{
    float scale = stbtt_ScaleForPixelHeight(&font.font, fontSize);
    
    KerningTable table = {};
    sprintf(table.info.font, "%s", font.fontName);
    
    for (char codepoint0 = ' '; codepoint0 <= '~'; ++codepoint0)
    {
        for (char codepoint1 = ' '; codepoint1 <= '~'; ++codepoint1)
        {
            f32 advance = scale * stbtt_GetCodepointKernAdvance(&font.font, codepoint0, codepoint1);
            if (advance != 0.0f)
            {
                table.info.infoCount++;
            }
        }
    }
    
    table.table = (Kerning *)calloc(1, table.info.infoCount * sizeof(Kerning));
    
    u32 tableIndex = 0;
    for (char codepoint0 = ' '; codepoint0 <= '~'; ++codepoint0)
    {
        for (char codepoint1 = ' '; codepoint1 <= '~'; ++codepoint1)
        {
            Kerning kerning = {};
            kerning.codepoint0 = codepoint0;
            kerning.codepoint1 = codepoint1;
            kerning.advance = scale * stbtt_GetCodepointKernAdvance(&font.font, codepoint0, codepoint1);
            
            if (kerning.advance != 0.0f)
            {
                table.table[tableIndex++] = kerning;
            }
        }
    }
    AssetHeader kerningAssetHeader = MakeAssetHeader_KerningTable(table.info);
    AddAsset(assets, kerningAssetHeader, table.table);
    
    FontMetadata metadata = {};
    s32 ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font.font, &ascent, &descent, &lineGap);
    metadata.lineGap = scale * ((f32)ascent - (f32)descent + (f32)lineGap);
    metadata.monospace = font.monospace;
    metadata.allCapital = font.allCapital;
    sprintf(metadata.font, "%s", font.fontName);
    
    AssetHeader metaAssetHeader = MakeAssetHeader_FontMetadata(metadata);
    AddAsset(assets, metaAssetHeader, 0);
}

function void WriteAssetFile(Game_Assets assets, char *filename, FILE *logFile)
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
            
            if (logFile)
            {
                switch (assetHeader.type)
                {
                    case AssetType_Bitmap:
                    {
                        fprintf(logFile, "Saving Bitmap %d to File \"%s\", Asset Size: %lld, Total Size: %lld\n", assetHeader.bitmap.id, filename, sizeof(AssetHeader) + assetHeader.assetSize, cumulativeSize);
                    } break;
                    
                    case AssetType_Glyph:
                    {
                        fprintf(logFile, "Saving Glyph %c of %s to File \"%s\", Asset Size: %lld, Total Size: %lld\n", assetHeader.glyph.glyph, assetHeader.glyph.font, filename, sizeof(AssetHeader) + assetHeader.assetSize, cumulativeSize);
                    } break;
                    
                    case AssetType_KerningTable:
                    {
                        fprintf(logFile, "Saving Kerning Table for %s to File \"%s\", Asset Size: %lld, Total Size: %lld\n", assetHeader.kerning.font, filename, sizeof(AssetHeader) + assetHeader.assetSize, cumulativeSize);
                    } break;
                    
                    case AssetType_FontMetadata:
                    {
                        fprintf(logFile, "Saving Metadata for Font %s to File \"%s\", Asset Size: %lld, Total Size: %lld\n", assetHeader.metadata.font, filename, sizeof(AssetHeader) + assetHeader.assetSize, cumulativeSize);
                    } break;
                    
                    INVALID_DEFAULT;
                }
            }
        }
        
        fclose(assetFile);
    }
}

function void GetFontInfo(FontInfo *font, char *fontName, char *fontFileName, b32 monospace, b32 allCapital)
{
    FILE *file = fopen(fontFileName, "rb");
    if (!file)
    {
        printf("Failed to find: %s\n", fontFileName);
        ASSERT(false);
    }
    
    u8 *ttfBuffer = (u8 *)calloc(1, 1 << 25);
    fread(ttfBuffer, 1, 1 << 25, file);
    
    sprintf(font->fontName, "%s", fontName);
    s32 fontInitResult = stbtt_InitFont(&font->font, ttfBuffer, 0);
    ASSERT(fontInitResult);
    
    font->monospace = monospace;
    font->allCapital = allCapital;
}

s32 main(s32 argc, char **argv)
{
    _chdir("c:\\work\\ast\\data");
    
    Game_Assets assets = {};
    FILE *logFile = fopen(".\\logs\\asset_build_log.txt", "wb");
    
    FontInfo fonts[2] = {};
    GetFontInfo(&fonts[0], "Hyperspace", ".\\raw\\HyperspaceBold.ttf", false, true);
    GetFontInfo(&fonts[1], "Debug", "C:\\Windows\\Fonts\\Ltype.ttf", true, false);
    
    AddToAssets_Bitmap(&assets, ".\\raw\\player.bmp", BitmapID_Player_NoTrail);
    AddToAssets_Bitmap(&assets, ".\\raw\\player_trail.bmp", BitmapID_Player_Trail);
    AddToAssets_Bitmap(&assets, ".\\raw\\ast1.bmp", BitmapID_Asteroid0);
    AddToAssets_Bitmap(&assets, ".\\raw\\ast2.bmp", BitmapID_Asteroid1);
    AddToAssets_Bitmap(&assets, ".\\raw\\ast3.bmp", BitmapID_Asteroid2);
    AddToAssets_Bitmap(&assets, ".\\raw\\ast4.bmp", BitmapID_Asteroid3);
    AddToAssets_Bitmap(&assets, ".\\raw\\ufoL.bmp", BitmapID_UFO_Large);
    WriteAssetFile(assets, "graphics.aaf", logFile);
    
    ClearAssets(&assets);
    
    for (s32 fontIndex = 0; fontIndex < ARRAY_COUNT(fonts); ++fontIndex)
    {
        f32 fontSize = (strcmp(fonts[fontIndex].fontName, "Debug") == 0) ? 16.0f : DEFAULT_FONT_SIZE;
        for (char glyphCodepoint = ' '; glyphCodepoint <= '~'; ++glyphCodepoint)
        {
            AddToAssets_Glyph(&assets, fonts[fontIndex], glyphCodepoint, fontSize);
        }
        
        AddToAssets_KerningAndMetadata(&assets, fonts[fontIndex], fontSize);
    }
    
    WriteAssetFile(assets, "fonts.aaf", logFile);
    
    fclose(logFile);
    
    return 0;
}