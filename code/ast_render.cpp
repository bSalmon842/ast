/*
Project: Asteroids
File: ast_render.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#define PushRenderEntry(group, type, sort, layer) PushRenderEntry_(group, sizeof(type), RenderEntryType_##type, sort, layer)
inline void *PushRenderEntry_(Game_RenderCommands *commands, usize size, RenderEntryType type, f32 zPos, s32 zLayer)
{
    DEBUG_BLOCK_FUNC;
    
    void *result = 0;
    
    size += sizeof(RenderEntry_Header);
    
    if (commands->pushBufferSize + size <= commands->maxPushBufferSize)
    {
        RenderEntry_Header *header = (RenderEntry_Header *)(commands->pushBufferBase + commands->pushBufferSize);
        header->type = type;
        header->zPos = zPos;
        header->zLayer = zLayer;
        header->entrySize = size;
        result = header + 1;
        
        commands->pushBufferSize += size;
        commands->headers[commands->entryCount++] = header;
        
        for (s32 entryIndex = commands->entryCount - 1; entryIndex >= 0; --entryIndex)
        {
            RenderEntry_Header *a = commands->headers[entryIndex];
            if ((entryIndex - 1) >= 0)
            {
                RenderEntry_Header *b = commands->headers[entryIndex - 1];
                if (a->zPos < b->zPos || (a->zPos == b->zPos && a->zLayer < b->zLayer))
                {
                    RenderEntry_Header *temp = commands->headers[entryIndex];
                    commands->headers[entryIndex] = commands->headers[entryIndex - 1];
                    commands->headers[entryIndex - 1] = temp;
                }
                else
                {
                    break;
                }
            }
        }
    }
    
    return result;
}

inline RenderEntryPositioning GetRenderScreenPositioning(Game_RenderCommands *commands, Camera camera, v3f offset, v2f dims)
{
    RenderEntryPositioning result = {};
    
    if (camera.orthographic)
    {
        f32 pzDist = camera.rect.center.z - offset.z;
        if (pzDist > camera.nearClip)
        {
            result.pos = offset;
            result.dims = dims;
            result.valid = true;
        }
    }
    else
    {
        offset.xy -= camera.rect.center.xy;
        
        f32 pzDist = camera.rect.center.z - offset.z;
        v3f rawXYZ = V3F(offset.xy, 1.0f);
        
        if (pzDist > camera.nearClip)
        {
            v3f projXYZ = (camera.focalLength * rawXYZ) / pzDist;
            
            result.pos.xy = camera.screenCenterPixels + (projXYZ.xy * camera.worldToPixelConversion);
            result.pos.z = offset.z;
            result.dims = (projXYZ.z * dims) * camera.worldToPixelConversion;
            result.valid = true;
        }
    }
    
    return result;
}

inline void PushBitmap(Game_RenderCommands *commands, Camera camera, BitmapID id, v3f offset, v2f dims, f32 angle, s32 zLayer, v4f colour = V4F(1.0f))
{
    RenderEntryPositioning positioning = GetRenderScreenPositioning(commands, camera, offset, dims);
    RenderEntry_Bitmap *entry = (RenderEntry_Bitmap *)PushRenderEntry(commands, RenderEntry_Bitmap, positioning.pos.z, zLayer);
    if (entry && positioning.valid)
    {
        entry->assetHeader = GetAsset(commands->loadedAssets, AssetType_Bitmap, &id, false);
        
        entry->positioning = positioning;
        
        entry->angle = angle + TAU * 0.25f;
        entry->colour = colour;
    }
}

inline void PushRect(Game_RenderCommands *commands, Camera camera, v3f offset, v2f dims, f32 angle, s32 zLayer, v4f colour)
{
    RenderEntryPositioning positioning = GetRenderScreenPositioning(commands, camera, offset, dims);
    RenderEntry_Rect *entry = (RenderEntry_Rect *)PushRenderEntry(commands, RenderEntry_Rect, positioning.pos.z, zLayer);
    if (entry && positioning.valid)
    {
        entry->positioning = positioning;
        
        entry->angle = angle;
        entry->colour = colour;
    }
}

inline void PushRect(Game_RenderCommands *commands, Camera camera, v2f min, v2f max, f32 z, f32 angle, s32 zLayer, v4f colour)
{
    v3f offset = V3F(((max - min) / 2.0f) + min, z);
    v2f dims = max - min;
    PushRect(commands, camera, offset, dims, angle, zLayer, colour);
}

inline void PushHollowRect(Game_RenderCommands *commands, Camera camera, v3f offset, v2f dims, f32 angle, f32 thickness, s32 zLayer, v4f colour)
{
    PushRect(commands, camera, V3F(offset.x, offset.y + (dims.y / 2.0f), offset.z), {dims.x, thickness}, angle, zLayer, colour); // Top
    PushRect(commands, camera, V3F(offset.x, offset.y - (dims.y / 2.0f), offset.z), {dims.x, thickness}, angle, zLayer, colour); // Bottom
    PushRect(commands, camera, V3F(offset.x - (dims.x / 2.0f), offset.y, offset.z), {thickness, dims.y}, angle, zLayer, colour); // Left
    PushRect(commands, camera, V3F(offset.x + (dims.x / 2.0f), offset.y, offset.z), {thickness, dims.y}, angle, zLayer, colour); // Right
}

inline void PushClear(Game_RenderCommands *commands, v4f colour)
{
    RenderEntry_Clear *entry = (RenderEntry_Clear *)PushRenderEntry(commands, RenderEntry_Clear, -FLT_MAX, 0);
    if (entry)
    {
        entry->colour = colour;
    }
}

inline f32 GetStringWidth(Game_RenderCommands *commands, char *font, char *string)
{
    // NOTE(bSalmon): Only suitable for monospace fonts
    
    f32 result = 0.0f;
    
    GlyphIdentifier id = {'0', font};
    LoadedAssetHeader *loadedAsset = GetAsset(commands->loadedAssets, AssetType_Glyph, &id, false);
    GlyphInfo info = loadedAsset->glyph;
    
    result = (f32)info.dims.x * StringLength(string);
    
    return result;
}

inline void PushText(Game_RenderCommands *commands, Camera camera, char *string, char *font, v3f offset, f32 scale, s32 zLayer, v4f colour)
{
    RenderEntryPositioning positioning = GetRenderScreenPositioning(commands, camera, offset, V2F());
    RenderEntry_Text *entry = (RenderEntry_Text *)PushRenderEntry(commands, RenderEntry_Text, positioning.pos.z, zLayer);
    if (entry && positioning.valid)
    {
        stbsp_sprintf(entry->string, "%s", string);
        
        char *c = entry->string;
        for (s32 i = 0; *c; ++i)
        {
            if (*c != '\n' && *c != '\t')
            {
                GlyphIdentifier id = {*c, font};
                entry->assetHeaders[i] = GetAsset(commands->loadedAssets, AssetType_Glyph, &id, false);
            }
            c++;
        }
        
        LoadedAssetHeader *metadataHeader = GetAsset(commands->loadedAssets, AssetType_FontMetadata, font, true);
        entry->metadata = metadataHeader->metadata;
        if (entry->metadata.allCapital)
        {
            StringToUpper(entry->string);
        }
        
        LoadedAssetHeader *kerningHeader = GetAsset(commands->loadedAssets, AssetType_KerningTable, font, true);
        entry->kerningTable = GetKerningTableFromAssetHeader(kerningHeader);
        
        offset.y = (f32)commands->height - offset.y;
        entry->positioning = positioning;
        
        entry->scale = scale;
        entry->colour = colour;
    }
}

#define DEFAULT_CHAR_WIDTH 13.5f
inline void PushTooltip(Game_RenderCommands *commands, Camera camera, char *string, char *font, f32 textScale, v2f mousePos, s32 zLayer)
{
    u8 lineCount = 1;
    char *c = string;
    s32 longestLineLength = 0;
    s32 currLineLength = 0;
    while (*c)
    {
        if (*c == '\n')
        {
            lineCount++;
            if (currLineLength > longestLineLength)
            {
                longestLineLength = currLineLength;
                currLineLength = 0;
            }
        }
        else
        {
            currLineLength++;
        }
        c++;
    }
    
    LoadedAssetHeader *metadataHeader = GetAsset(commands->loadedAssets, AssetType_FontMetadata, font, true);
    FontMetadata metadata = metadataHeader->metadata;
    f32 lineHeight = metadata.lineGap * textScale;
    
    v2f tooltipDims = V2F();
    if (metadata.monospace)
    {
        tooltipDims = V2F((f32)(longestLineLength * metadata.charWidth) * textScale + 20.0f, (lineHeight * lineCount) + 5.0f);
    }
    else
    {
        tooltipDims = V2F((f32)longestLineLength * (DEFAULT_CHAR_WIDTH * textScale) + 20.0f, (lineHeight * lineCount) + 5.0f);
    }
    
    v2f tooltipMax = V2F();
    v2f tooltipMin = V2F();
    if (mousePos.x <= commands->width / 2 &&
        mousePos.y > commands->height / 2)
    {
        tooltipMin = V2F(mousePos.x + 10.0f, mousePos.y - tooltipDims.y);
        tooltipMax = tooltipMin + tooltipDims;
    }
    else if (mousePos.x > commands->width / 2 &&
             mousePos.y > commands->height / 2)
    {
        tooltipMax = (mousePos - V2F(10.0f));
        tooltipMin = tooltipMax - tooltipDims;
    }
    else if (mousePos.x <= commands->width / 2 &&
             mousePos.y <= commands->height / 2)
    {
        tooltipMin = mousePos + V2F(10.0f);
        tooltipMax = tooltipMin + tooltipDims;
    }
    else if (mousePos.x > commands->width / 2 &&
             mousePos.y <= commands->height / 2)
    {
        tooltipMin = V2F(mousePos.x - (tooltipDims.x + 10.0f), mousePos.y + 10.0f);
        tooltipMax = tooltipMin + tooltipDims;
    }
    
    PushRect(commands, camera, tooltipMin, tooltipMax, 0.0f, 0.0f, zLayer, V4F(V3F(0.1f), 0.66f));
    
    v2f hoverLineOffset = V2F(tooltipMin.x + 10.0f, tooltipMax.y - lineHeight);
    PushText(commands, camera, string, font, V3F(hoverLineOffset, 0.0f), textScale, zLayer, V4F(1.0f));
}
