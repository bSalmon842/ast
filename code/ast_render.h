/* date = February 14th 2022 2:58 pm */

#ifndef AST_RENDER_H

enum TextAlign
{
    TextAlign_Center,
    TextAlign_TopLeft,
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
} RenderEntry_Header;

typedef struct
{
    Bitmap *bitmap;
    v2f offset;
    v2f dims;
    f32 angle;
    v4f colour;
} RenderEntry_Bitmap;

typedef struct
{
    v2f offset;
    v2f dims;
    f32 angle;
    v4f colour;
} RenderEntry_Rect;

typedef struct
{
    v4f colour;
} RenderEntry_Clear;

typedef struct
{
    stbtt_fontinfo *fontInfo;
    v2f offset;
    char *text;
    
    TextAlign align;
    
    f32 lineHeight;
    v4f colour;
    
    PlatformAPI platform;
} RenderEntry_Text;

typedef struct
{
    v2f worldToPixelConversion;
    
    u8 *pushBufferBase;
    usize pushBufferSize;
    usize maxPushBufferSize;
} RenderGroup;

#define AST_RENDER_H
#endif //AST_RENDER_H
