/*
Project: Asteroids
File: ast_asset.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

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

function Bitmap *GetBitmapAsset(Bitmap **cachedBitmaps, u32 *cachedBitmapCount, PlatformAPI platform, u32 searchID)
{
    Bitmap *result = 0;
    
    for (u32 cacheIndex = 0; cacheIndex < *cachedBitmapCount; ++cacheIndex)
    {
        if ((*cachedBitmaps)[cacheIndex].info.id == searchID)
        {
            result = &(*cachedBitmaps)[cacheIndex];
            break;
        }
    }
    
    if (!result)
    {
        Platform_FileGroup fileGroup = platform.GetAllFilesOfExtBegin(PlatformFileType_Asset);
        
        for (u32 i = 0; i < fileGroup.fileCount; ++i)
        {
            Platform_FileHandle fileHandle = platform.OpenNextFile(&fileGroup);
            AAFHeader fileHeader;
            platform.ReadDataFromFile(&fileHandle, 0, sizeof(AAFHeader), &fileHeader);
            ASSERT(fileHeader.magicValue == AAF_CODE('B', 'A', 'A', 'F'));
            
            usize offset = sizeof(AAFHeader);
            for (u32 assetIndex = 0; assetIndex < fileHeader.assetCount; ++assetIndex)
            {
                AssetHeader assetHeader;
                platform.ReadDataFromFile(&fileHandle, offset, sizeof(AssetHeader), &assetHeader);
                offset += sizeof(AssetHeader);
                
                if (assetHeader.type == AssetType_Bitmap && assetHeader.bitmap.id == searchID)
                {
                    (*cachedBitmapCount)++;
                    *cachedBitmaps = (Bitmap *)realloc(*cachedBitmaps, *cachedBitmapCount * sizeof(Bitmap));
                    Bitmap *bitmap = &(*cachedBitmaps)[*cachedBitmapCount - 1];
                    bitmap->info = assetHeader.bitmap;
                    bitmap->memory = platform.MemAlloc(assetHeader.assetSize);
                    platform.ReadDataFromFile(&fileHandle, offset, assetHeader.assetSize, bitmap->memory);
                    result = bitmap;
                    break;
                }
                
                offset += assetHeader.assetSize;
            }
        }
        
        platform.GetAllFilesOfExtEnd(&fileGroup);
    }
    
    return result;
}

function Glyph *GetGlyphAsset(Glyph **cachedGlyphs, u32 *cachedGlyphCount, PlatformAPI platform, char searchChar)
{
    Glyph *result = 0;
    
    for (u32 cacheIndex = 0; cacheIndex < *cachedGlyphCount; ++cacheIndex)
    {
        if ((*cachedGlyphs)[cacheIndex].info.glyph == searchChar)
        {
            result = &(*cachedGlyphs)[cacheIndex];
            break;
        }
    }
    
    if (!result)
    {
        Platform_FileGroup fileGroup = platform.GetAllFilesOfExtBegin(PlatformFileType_Asset);
        
        for (u32 i = 0; i < fileGroup.fileCount; ++i)
        {
            Platform_FileHandle fileHandle = platform.OpenNextFile(&fileGroup);
            AAFHeader fileHeader;
            platform.ReadDataFromFile(&fileHandle, 0, sizeof(AAFHeader), &fileHeader);
            ASSERT(fileHeader.magicValue == AAF_CODE('B', 'A', 'A', 'F'));
            
            usize offset = sizeof(AAFHeader);
            for (u32 assetIndex = 0; assetIndex < fileHeader.assetCount; ++assetIndex)
            {
                AssetHeader assetHeader;
                platform.ReadDataFromFile(&fileHandle, offset, sizeof(AssetHeader), &assetHeader);
                offset += sizeof(AssetHeader);
                
                if (assetHeader.type == AssetType_Glyph && assetHeader.glyph.glyph == searchChar)
                {
                    (*cachedGlyphCount)++;
                    *cachedGlyphs = (Glyph *)realloc(*cachedGlyphs, *cachedGlyphCount * sizeof(Glyph));
                    Glyph *glyph = &(*cachedGlyphs)[*cachedGlyphCount - 1];
                    glyph->info = assetHeader.glyph;
                    glyph->memory = platform.MemAlloc(assetHeader.assetSize);
                    platform.ReadDataFromFile(&fileHandle, offset, assetHeader.assetSize, glyph->memory);
                    result = glyph;
                    break;
                }
                
                offset += assetHeader.assetSize;
            }
        }
        
        platform.GetAllFilesOfExtEnd(&fileGroup);
    }
    
    return result;
}

inline void GetRequiredGlyphsForString(Game_RenderCommands *commands, PlatformAPI platform, char *text)
{
    char *c = text;
    while (*c)
    {
        GetGlyphAsset(&commands->cachedGlyphs, &commands->cachedGlyphCount, platform, *c);
        c++;
    }
}

// NOTE(bSalmon): Assumes all required glyphs are already cached
inline v2f GetStringDims(Game_RenderCommands *commands, f32 scale, char *text)
{
    v2f result = V2F();
    
    char *c = text;
    while (*c)
    {
        Glyph *glyph = 0;
        for (u32 cacheIndex = 0; cacheIndex < commands->cachedGlyphCount; ++cacheIndex)
        {
            glyph = &commands->cachedGlyphs[cacheIndex];
            if (glyph->info.glyph == *c)
            {
                result.x += (glyph->info.dims.w * scale);
                
            }
        }
        
        c++;
    }
    
    return result;
}