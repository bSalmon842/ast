/*
Project: Asteroids
File: ast_render.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#define PushRenderEntry(group, type) PushRenderEntry_(group, sizeof(type), RenderEntryType_##type)
inline void *PushRenderEntry_(Game_RenderCommands *commands, usize size, RenderEntryType type)
{
    void *result = 0;
    
    size += sizeof(RenderEntry_Header);
    
    if (commands->pushBufferSize + size <= commands->maxPushBufferSize)
    {
        RenderEntry_Header *header = (RenderEntry_Header *)(commands->pushBufferBase + commands->pushBufferSize);
        header->type = type;
        result = header + 1;
        commands->pushBufferSize += size;
    }
    
    return result;
}

inline RenderEntryPositioning GetRenderScreenPositioning(Game_RenderCommands *commands, Camera camera, v2f worldToPixelConversion, v3f offset, v2f dims)
{
    RenderEntryPositioning result = {};
    
    v2f screenCenter = V2F((f32)commands->width, (f32)commands->height) / 2.0f;
    offset.xy -= camera.pos.xy;
    
    f32 pzDist = camera.pos.z - offset.z;
    v3f rawXYZ = V3F(offset.xy, 1.0f);
    
    if (pzDist > camera.nearClip)
    {
        v3f projXYZ = (camera.focalLength * rawXYZ) / pzDist;
        
        result.pos = screenCenter + (projXYZ.xy * worldToPixelConversion);
        result.dims = (projXYZ.z * dims) * worldToPixelConversion;
        result.valid = true;
    }
    
    return result;
}

inline void PushBitmap(Game_RenderCommands *commands, v2f worldToPixelConversion, Camera camera, BitmapID id, v3f offset, v2f dims, f32 angle, v4f colour = V4F(1.0f))
{
    RenderEntryPositioning positioning = GetRenderScreenPositioning(commands, camera, worldToPixelConversion, offset, dims);
    RenderEntry_Bitmap *entry = (RenderEntry_Bitmap *)PushRenderEntry(commands, RenderEntry_Bitmap);
    if (entry && positioning.valid)
    {
        entry->bitmapID = id;
        
        entry->positioning = positioning;
        
        entry->angle = angle + TAU * 0.25f;
        entry->colour = colour;
    }
}

inline void PushRect(Game_RenderCommands *commands, v2f worldToPixelConversion, Camera camera, v3f offset, v2f dims, f32 angle, v4f colour)
{
    RenderEntryPositioning positioning = GetRenderScreenPositioning(commands, camera, worldToPixelConversion, offset, dims);
    RenderEntry_Rect *entry = (RenderEntry_Rect *)PushRenderEntry(commands, RenderEntry_Rect);
    if (entry && positioning.valid)
    {
        entry->positioning = positioning;
        
        entry->angle = angle;
        entry->colour = colour;
    }
}

inline void PushHollowRect(Game_RenderCommands *commands, v2f worldToPixelConversion, Camera camera, v3f offset, v2f dims, f32 angle, f32 thickness, v4f colour)
{
    PushRect(commands, worldToPixelConversion, camera, V3F(offset.x, offset.y + (dims.y / 2.0f), offset.z), {dims.x, thickness}, angle, colour); // Top
    PushRect(commands, worldToPixelConversion, camera, V3F(offset.x, offset.y - (dims.y / 2.0f), offset.z), {dims.x, thickness}, angle, colour); // Bottom
    PushRect(commands, worldToPixelConversion, camera, V3F(offset.x - (dims.x / 2.0f), offset.y, offset.z), {thickness, dims.y}, angle, colour); // Left
    PushRect(commands, worldToPixelConversion, camera, V3F(offset.x + (dims.x / 2.0f), offset.y, offset.z), {thickness, dims.y}, angle, colour); // Right
}

inline void PushClear(Game_RenderCommands *commands, v4f colour)
{
    RenderEntry_Clear *entry = (RenderEntry_Clear *)PushRenderEntry(commands, RenderEntry_Clear);
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

inline void PushText(Game_RenderCommands *commands, v2f worldToPixelConversion, RenderString string, char *font, Camera camera, v3f offset, f32 scale, v4f colour)
{
    RenderEntryPositioning positioning = GetRenderScreenPositioning(commands, camera, worldToPixelConversion, offset, V2F());
    RenderEntry_Text *entry = (RenderEntry_Text *)PushRenderEntry(commands, RenderEntry_Text);
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

function RenderGroup *AllocateRenderGroup(MemoryRegion *memRegion, Game_State *gameState, Game_Memory *memory, v2f worldToPixelConversion)
{
    RenderGroup *result = PushStruct(memRegion, RenderGroup);
    
    result->worldToPixelConversion = worldToPixelConversion;
    result->gameState = gameState;
    result->memory = memory;
    
    return result;
}
