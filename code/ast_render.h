/*
Project: Asteroids
File: ast_render.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_RENDER_H

struct Camera
{
    Rect3f rect;
    f32 focalLength;
    f32 nearClip;
    s32 linkedEntityIndex;
    v2f worldToPixelConversion;
    v2f screenCenterPixels;
    b32 orthographic;
};

enum RenderEntryType
{
    RenderEntryType_RenderEntry_Null,
    RenderEntryType_RenderEntry_Clear,
    RenderEntryType_RenderEntry_Bitmap,
    RenderEntryType_RenderEntry_Rect,
    RenderEntryType_RenderEntry_Text,
};

struct RenderEntry_Header
{
    RenderEntryType type;
    f32 zPos;
    s32 zLayer;
    usize entrySize;
};

struct RenderEntryPositioning
{
    v3f pos;
    v2f dims;
    b32 valid;
};

struct RenderEntry_Bitmap
{
    LoadedAssetHeader *assetHeader;
    
    RenderEntryPositioning positioning;
    
    f32 angle;
    v4f colour;
};

struct RenderEntry_Rect
{
    RenderEntryPositioning positioning;
    
    f32 angle;
    v4f colour;
};

struct RenderEntry_Clear
{
    v4f colour;
};

struct RenderEntry_Text
{
    LoadedAssetHeader *assetHeaders[128];
    
    char string[128];
    KerningTable kerningTable;
    FontMetadata metadata;
    
    RenderEntryPositioning positioning;
    
    f32 scale;
    v4f colour;
};

#define AST_RENDER_H
#endif //AST_RENDER_H
