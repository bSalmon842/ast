/* date = February 14th 2022 2:58 pm */

#ifndef AST_RENDER_H

struct RenderString
{
    char *text;
    u8 length;
};

struct KerningTable
{
    KernInfo *table;
    u32 infoCount;
    char font[32];
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
    BitmapID bitmapID;
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
    RenderString string;
    char font[32];
    KerningTable kerningTable;
    
    v2f offset;
    f32 scale;
    v4f colour;
} RenderEntry_Text;

struct RenderGroup
{
    struct Game_State *gameState;
    Game_Memory *memory;
    
    v2f worldToPixelConversion;
};

#define AST_RENDER_H
#endif //AST_RENDER_H
