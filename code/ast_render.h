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

typedef struct
{
    u32 a;
    u32 b;
    u32 c;
    u32 d;
} BilinearSampleResult;

enum RenderEntryType
{
    RenderEntryType_RenderEntry_Null,
    RenderEntryType_RenderEntry_Clear,
    RenderEntryType_RenderEntry_Bitmap,
    RenderEntryType_RenderEntry_Rect,
    RenderEntryType_RenderEntry_Text,
};

typedef struct
{
    RenderEntryType type;
    f32 zPos;
    s32 zLayer;
    usize entrySize;
} RenderEntry_Header;

struct RenderEntryPositioning
{
    v3f pos;
    v2f dims;
    b32 valid;
};

typedef struct
{
    BitmapID bitmapID;
    
    RenderEntryPositioning positioning;
    
    f32 angle;
    v4f colour;
} RenderEntry_Bitmap;

typedef struct
{
    RenderEntryPositioning positioning;
    
    f32 angle;
    v4f colour;
} RenderEntry_Rect;

typedef struct
{
    v4f colour;
} RenderEntry_Clear;

typedef struct
{
    char string[256];
    char font[32];
    KerningTable kerningTable;
    FontMetadata metadata;
    
    RenderEntryPositioning positioning;
    
    f32 scale;
    v4f colour;
} RenderEntry_Text;

#define AST_RENDER_H
#endif //AST_RENDER_H
