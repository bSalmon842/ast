/*
Project: Asteroids
File: ast_render.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#include "ast_render.h"

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

inline void PushBitmap(Game_RenderCommands *commands, v2f worldToPixelConversion, BitmapID id, v2f offset, v2f dims, f32 angle, v4f colour = V4F(1.0f))
{
    RenderEntry_Bitmap *entry = (RenderEntry_Bitmap *)PushRenderEntry(commands, RenderEntry_Bitmap);
    if (entry)
    {
        entry->bitmapID = id;
        entry->offset = offset * worldToPixelConversion;
        entry->dims = dims * worldToPixelConversion;
        entry->angle = angle + TAU * 0.25f;
        entry->colour = colour;
    }
}

inline void PushRect(Game_RenderCommands *commands, v2f worldToPixelConversion, v2f offset, v2f dims, f32 angle, v4f colour)
{
    RenderEntry_Rect *entry = (RenderEntry_Rect *)PushRenderEntry(commands, RenderEntry_Rect);
    if (entry)
    {
        entry->offset = offset * worldToPixelConversion;
        entry->dims = dims * worldToPixelConversion;
        entry->angle = angle;
        entry->colour = colour;
    }
}

inline void PushClear(Game_RenderCommands *commands, v4f colour)
{
    RenderEntry_Clear *entry = (RenderEntry_Clear *)PushRenderEntry(commands, RenderEntry_Clear);
    if (entry)
    {
        entry->colour = colour;
    }
}

inline void PushText(Game_RenderCommands *commands, v2f worldToPixelConversion, RenderString string, v2f offset, f32 scale, TextAlign align, v4f colour)
{
    RenderEntry_Text *entry = (RenderEntry_Text *)PushRenderEntry(commands, RenderEntry_Text);
    if (entry)
    {
        entry->string = string;
        entry->offset = offset * worldToPixelConversion;
        entry->offset.y = (f32)commands->height - offset.y;
        entry->scale = scale;
        entry->align = align;
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
