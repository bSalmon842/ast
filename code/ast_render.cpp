/*
Project: Asteroids
File: ast_render.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

inline void *FillRenderEntry(Game_RenderCommands *commands, usize size, u8 *baseAddress, RenderEntryType type, f32 zPos, s32 zLayer)
{
    void *result = 0;
    
    RenderEntry_Header *header = (RenderEntry_Header *)baseAddress;
    header->type = type;
    header->zPos = zPos;
    header->zLayer = zLayer;
    header->entrySize = size;
    result = header + 1;
    
    commands->pushBufferSize += size;
    commands->entryCount++;
    
    return result;
}

#define PushRenderEntry(group, type, sort, layer, platform) PushRenderEntry_(group, sizeof(type), RenderEntryType_##type, sort, layer, platform)
inline void *PushRenderEntry_(Game_RenderCommands *commands, usize size, RenderEntryType type, f32 zPos, s32 zLayer, PlatformAPI platform)
{
    void *result = 0;
    
    size += sizeof(RenderEntry_Header);
    
    BEGIN_TIMED_BLOCK(PushRenderEntry);
    
    if (commands->pushBufferSize + size <= commands->maxPushBufferSize)
    {
        u8 *baseAddress = commands->pushBufferBase;
        for (u32 entryIndex = 0; entryIndex <= commands->entryCount; ++entryIndex)
        {
            RenderEntry_Header *testHeader = (RenderEntry_Header *)baseAddress;
            if (zPos < testHeader->zPos || (zPos == testHeader->zPos && zLayer < testHeader->zLayer) || entryIndex == commands->entryCount)
            {
                usize shiftSize = commands->pushBufferSize - (baseAddress - commands->pushBufferBase);
                void *temp = platform.MemAlloc(shiftSize);
                CopyMem(temp, baseAddress, shiftSize);
                CopyMem(baseAddress + size, temp, shiftSize);
                
                result = FillRenderEntry(commands, size, baseAddress, type, zPos, zLayer);
                
                platform.MemFree(temp);
                break;
            }
            
            baseAddress += testHeader->entrySize;
        }
    }
    
    END_TIMED_BLOCK(PushRenderEntry);
    
    return result;
}

inline RenderEntryPositioning GetRenderScreenPositioning(Game_RenderCommands *commands, Camera camera, v3f offset, v2f dims)
{
    RenderEntryPositioning result = {};
    
    v2f screenCenter = V2F((f32)commands->width, (f32)commands->height) / 2.0f;
    offset.xy -= camera.pos.xy;
    
    f32 pzDist = camera.pos.z - offset.z;
    v3f rawXYZ = V3F(offset.xy, 1.0f);
    
    if (pzDist > camera.nearClip)
    {
        v3f projXYZ = (camera.focalLength * rawXYZ) / pzDist;
        
        result.pos.xy = screenCenter + (projXYZ.xy * camera.worldToPixelConversion);
        result.pos.z = offset.z;
        result.dims = (projXYZ.z * dims) * camera.worldToPixelConversion;
        result.valid = true;
    }
    
    return result;
}

inline void PushBitmap(Game_RenderCommands *commands, PlatformAPI platform, Camera camera, BitmapID id, v3f offset, v2f dims, f32 angle, s32 zLayer, v4f colour = V4F(1.0f))
{
    RenderEntryPositioning positioning = GetRenderScreenPositioning(commands, camera, offset, dims);
    RenderEntry_Bitmap *entry = (RenderEntry_Bitmap *)PushRenderEntry(commands, RenderEntry_Bitmap, positioning.pos.z, zLayer, platform);
    if (entry && positioning.valid)
    {
        entry->bitmapID = id;
        
        entry->positioning = positioning;
        
        entry->angle = angle + TAU * 0.25f;
        entry->colour = colour;
    }
}

inline void PushRect(Game_RenderCommands *commands, PlatformAPI platform, Camera camera, v3f offset, v2f dims, f32 angle, s32 zLayer, v4f colour)
{
    RenderEntryPositioning positioning = GetRenderScreenPositioning(commands, camera, offset, dims);
    RenderEntry_Rect *entry = (RenderEntry_Rect *)PushRenderEntry(commands, RenderEntry_Rect, positioning.pos.z, zLayer, platform);
    if (entry && positioning.valid)
    {
        entry->positioning = positioning;
        
        entry->angle = angle;
        entry->colour = colour;
    }
}

inline void PushHollowRect(Game_RenderCommands *commands, PlatformAPI platform, Camera camera, v3f offset, v2f dims, f32 angle, f32 thickness, s32 zLayer, v4f colour)
{
    PushRect(commands, platform, camera, V3F(offset.x, offset.y + (dims.y / 2.0f), offset.z), {dims.x, thickness}, angle, zLayer, colour); // Top
    PushRect(commands, platform, camera, V3F(offset.x, offset.y - (dims.y / 2.0f), offset.z), {dims.x, thickness}, angle, zLayer, colour); // Bottom
    PushRect(commands, platform, camera, V3F(offset.x - (dims.x / 2.0f), offset.y, offset.z), {thickness, dims.y}, angle, zLayer, colour); // Left
    PushRect(commands, platform, camera, V3F(offset.x + (dims.x / 2.0f), offset.y, offset.z), {thickness, dims.y}, angle, zLayer, colour); // Right
}

inline void PushClear(Game_RenderCommands *commands, PlatformAPI platform, v4f colour)
{
    RenderEntry_Clear *entry = (RenderEntry_Clear *)PushRenderEntry(commands, RenderEntry_Clear, -FLT_MAX, 0, platform);
    if (entry)
    {
        entry->colour = colour;
    }
}

inline void RenderStringToUpper(RenderString *string)
{
    for (s32 i = 0; i < string->length; ++i)
    {
        if (string->text[i] >= 'a' && string->text[i] <= 'z')
        {
            string->text[i] -= 32;
        }
    }
}

inline void PushText(Game_RenderCommands *commands, PlatformAPI platform, Camera camera, RenderString string, char *font, v3f offset, f32 scale, s32 zLayer, v4f colour)
{
    RenderEntryPositioning positioning = GetRenderScreenPositioning(commands, camera, offset, V2F());
    RenderEntry_Text *entry = (RenderEntry_Text *)PushRenderEntry(commands, RenderEntry_Text, positioning.pos.z, zLayer, platform);
    if (entry && positioning.valid)
    {
        entry->string = string;
        sprintf(entry->font, "%s", font);
        
        LoadedAssetHeader *metadataHeader = GetAsset(commands->loadedAssets, AssetType_FontMetadata, font, true);
        entry->metadata = metadataHeader->metadata;
        if (entry->metadata.allCapital)
        {
            RenderStringToUpper(&entry->string);
        }
        
        LoadedAssetHeader *kerningHeader = GetAsset(commands->loadedAssets, AssetType_KerningTable, font, true);
        entry->kerningTable = GetKerningTableFromAssetHeader(kerningHeader);
        
        offset.y = (f32)commands->height - offset.y;
        entry->positioning = positioning;
        
        entry->scale = scale;
        entry->colour = colour;
    }
}

#include <stdarg.h>
inline RenderString MakeRenderString(PlatformAPI platform, char *fmt, ...)
{
    RenderString result = {};
    
    va_list args;
    va_start(args, fmt);
    
    char string[256] = {};
    result.length = (u8)vsprintf(string, fmt, args);
    result.text = (char *)platform.MemAlloc(result.length);
    for (u8 i = 0; i < result.length; ++i) { result.text[i] = string[i]; }
    
    va_end(args);
    
    return result;
}
