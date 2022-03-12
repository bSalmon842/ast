/*
Project: Asteroids
File: ast_asset.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#define BEGIN_SEARCHING_ASSET_FILES \
Platform_FileGroup fileGroup = platform.GetAllFilesOfExtBegin(PlatformFileType_Asset); \
\
for (u32 i = 0; i < fileGroup.fileCount; ++i) \
{ \
Platform_FileHandle fileHandle = platform.OpenNextFile(&fileGroup); \
AAFHeader fileHeader; \
platform.ReadDataFromFile(&fileHandle, 0, sizeof(AAFHeader), &fileHeader); \
ASSERT(fileHeader.magicValue == AAF_CODE('B', 'A', 'A', 'F')); \
\
usize offset = sizeof(AAFHeader); \
for (u32 assetIndex = 0; assetIndex < fileHeader.assetCount; ++assetIndex) \
{ \
AssetHeader assetHeader; \
platform.ReadDataFromFile(&fileHandle, offset, sizeof(AssetHeader), &assetHeader); \
offset += sizeof(AssetHeader)

#define FINISH_SEARCHING_ASSET_FILES \
offset += assetHeader.assetSize; \
} \
} \
\
platform.GetAllFilesOfExtEnd(&fileGroup)

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
        BEGIN_SEARCHING_ASSET_FILES;
        
        if (assetHeader.type == AssetType_Bitmap &&
            assetHeader.bitmap.id == searchID)
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
        
        FINISH_SEARCHING_ASSET_FILES;
    }
    
    return result;
}

function Glyph *GetGlyphAsset(Glyph **cachedGlyphs, u32 *cachedGlyphCount, PlatformAPI platform, char *searchFont, char searchChar)
{
    Glyph *result = 0;
    
    for (u32 cacheIndex = 0; cacheIndex < *cachedGlyphCount; ++cacheIndex)
    {
        GlyphInfo info = (*cachedGlyphs)[cacheIndex].info;
        if (info.glyph == searchChar &&
            StringsAreSame(searchFont, info.font, StringLength(info.font)))
        {
            result = &(*cachedGlyphs)[cacheIndex];
            break;
        }
    }
    
    if (!result)
    {
        BEGIN_SEARCHING_ASSET_FILES;
        
        if (assetHeader.type == AssetType_Glyph &&
            assetHeader.glyph.glyph == searchChar && StringsAreSame(assetHeader.glyph.font, searchFont, StringLength(assetHeader.glyph.font)))
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
        
        FINISH_SEARCHING_ASSET_FILES;
    }
    
    return result;
}

function KerningTable GetKerningTableForFont(Game_RenderCommands *commands, char *font)
{
    KerningTable result = {};
    
    for (u32 i = 0; i < commands->kerningTableCount; ++i)
    {
        KerningTable table = commands->kerningTables[i];
        if (StringsAreSame(table.font, font, StringLength(table.font)))
        {
            result = table;
        }
    }
    
    return result;
}

function KernInfo GetKerningInfo(KerningTable kerningTable, char searchChar0, char searchChar1)
{
    KernInfo result = {};
    
    for (u32 index = 0; index < kerningTable.infoCount; ++index)
    {
        KernInfo info = kerningTable.table[index];
        if (info.codepoint0 == searchChar0 &&
            info.codepoint1 == searchChar1)
        {
            result = info;
            break;
        }
    }
    
    return result;
}

function FontMetadata GetMetadataForFont(Game_RenderCommands *commands, char *font)
{
    FontMetadata result = {};
    
    for (u32 index = 0; index < commands->metadataCount; ++index)
    {
        FontMetadata metadata = commands->metadatas[index];
        if (StringsAreSame(metadata.font, font, StringLength(metadata.font)))
        {
            result = metadata;
            break;
        }
    }
    
    return result;
}