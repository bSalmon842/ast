/*
Project: Asteroids
File: ast_render.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#include "ast_render.h"

#define PushRenderEntry(group, type) PushRenderEntry_(group, sizeof(type), RenderEntryType_##type)
inline void *PushRenderEntry_(RenderGroup *renderGroup, usize size, RenderEntryType type)
{
    void *result = 0;
    
    size += sizeof(RenderEntry_Header);
    
    if (renderGroup->pushBufferSize + size <= renderGroup->maxPushBufferSize)
    {
        RenderEntry_Header *header = (RenderEntry_Header *)(renderGroup->pushBufferBase + renderGroup->pushBufferSize);
        header->type = type;
        result = header + 1;
        renderGroup->pushBufferSize += size;
    }
    
    return result;
}

inline void PushBitmap(RenderGroup *renderGroup, Bitmap *bitmap, v2f offset, v2f dims, f32 angle, v4f colour = V4F(1.0f))
{
    RenderEntry_Bitmap *entry = (RenderEntry_Bitmap *)PushRenderEntry(renderGroup, RenderEntry_Bitmap);
    if (entry)
    {
        entry->bitmap = bitmap;
        entry->offset = offset * renderGroup->worldToPixelConversion;
        entry->dims = dims * renderGroup->worldToPixelConversion;
        entry->angle = angle;
        entry->colour = colour;
    }
}

inline void PushRect(RenderGroup *renderGroup, v2f offset, v2f dims, f32 angle, v4f colour)
{
    RenderEntry_Rect *entry = (RenderEntry_Rect *)PushRenderEntry(renderGroup, RenderEntry_Rect);
    if (entry)
    {
        entry->offset = offset * renderGroup->worldToPixelConversion;
        entry->dims = dims * renderGroup->worldToPixelConversion;
        entry->angle = angle;
        entry->colour = colour;
    }
}

inline void PushClear(RenderGroup *renderGroup, v4f colour)
{
    RenderEntry_Clear *entry = (RenderEntry_Clear *)PushRenderEntry(renderGroup, RenderEntry_Clear);
    if (entry)
    {
        entry->colour = colour;
    }
}

inline void PushText(RenderGroup *renderGroup, char *text, stbtt_fontinfo *fontInfo, v2f offset, TextAlign align, f32 lineHeight, v4f colour, PlatformAPI platform)
{
    RenderEntry_Text *entry = (RenderEntry_Text *)PushRenderEntry(renderGroup, RenderEntry_Text);
    if (entry)
    {
        entry->fontInfo = fontInfo;
        entry->offset = offset * renderGroup->worldToPixelConversion;
        entry->text = text;
        entry->align = align;
        entry->lineHeight = lineHeight;
        entry->colour = colour;
        entry->platform = platform;
    }
}

inline v4f ARGB_U32ToV4F(u32 value)
{
    v4f result = V4F((f32)((value >> 16) & 0xFF),
                     (f32)((value >> 8) & 0xFF),
                     (f32)(value & 0xFF),
                     (f32)((value >> 24) & 0xFF));
    return result;
}

inline u32 ARGB_V4FToU32(v4f value)
{
    u32 result = ((u32)value.a << 24 |
                  (u32)value.r << 16 | 
                  (u32)value.g << 8 | 
                  (u32)value.b);
    return result;
}

inline u32 ARGB_V4FToU32Rounded(v4f value)
{
    u32 result = (RoundF32ToU32(value.a) << 24 |
                  RoundF32ToU32(value.r) << 16 | 
                  RoundF32ToU32(value.g) << 8 | 
                  RoundF32ToU32(value.b));
    return result;
}

inline v4f SRGBToLinear(v4f value)
{
    v4f result = V4F(Sq(value.r / 255.0f),
                     Sq(value.g / 255.0f),
                     Sq(value.b / 255.0f),
                     value.a / 255.0f);
    return result;
}

inline v4f LinearToSRGB(v4f value)
{
    v4f result = V4F(SqRt(value.r) * 255.0f,
                     SqRt(value.g) * 255.0f,
                     SqRt(value.b) * 255.0f,
                     value.a * 255.0f);
    return result;
}

inline v4f MultiplyAlpha(v4f value)
{
    v4f result = value;
    
    result.rgb *= value.a; 
    
    return result;
}

inline BilinearSampleResult BilinearSample(Bitmap *texture, v2s point)
{
    BilinearSampleResult result = {};
    
    u8 *texelPtr = (u8 *)texture->memory + (point.y * texture->pitch) + (point.x * BITMAP_BYTES_PER_PIXEL);
    result.a = *(u32 *)(texelPtr);
    result.b = *(u32 *)(texelPtr + BITMAP_BYTES_PER_PIXEL);
    result.c = *(u32 *)(texelPtr + texture->pitch);
    result.d = *(u32 *)(texelPtr + texture->pitch + BITMAP_BYTES_PER_PIXEL);
    
    return result;
}

inline v4f BilinearBlend_SRGB(BilinearSampleResult sample, v2f offset)
{
    v4f result = {};
    
    v4f texelA = ARGB_U32ToV4F(sample.a);
    v4f texelB = ARGB_U32ToV4F(sample.b);
    v4f texelC = ARGB_U32ToV4F(sample.c);
    v4f texelD = ARGB_U32ToV4F(sample.d);
    
    texelA = SRGBToLinear(texelA);
    texelB = SRGBToLinear(texelB);
    texelC = SRGBToLinear(texelC);
    texelD = SRGBToLinear(texelD);
    
    result = Lerp(Lerp(texelA, texelB, offset.x), Lerp(texelC, texelD, offset.x), offset.y);
    
    return result;
}

inline b32 CheckPixelIsOnBackBuffer(u32 *pixelLoc, Game_BackBuffer *backBuffer)
{
    b32 result = pixelLoc >= (u32 *)backBuffer->memory && pixelLoc < (u32 *)backBuffer->memory + (backBuffer->height * backBuffer->width);
    return result;
}

function void RenderClear(Game_BackBuffer *backBuffer, v4f colour)
{
    u8 *row = (u8 *)backBuffer->memory;
    for (s32 y = 0; y < backBuffer->height; ++y)
    {
        u32 *pixel = (u32 *)row;
        for (s32 x = 0; x < backBuffer->width; ++x)
        {
            *pixel++ = ARGB_V4FToU32(colour * 255.0f);
        }
        
        row += backBuffer->pitch;
    }
}

function void RenderRect(Game_BackBuffer *backBuffer, v2f origin, v2f dims, f32 angle, v4f colour)
{
    v2f backBufferF = V2F((f32)backBuffer->width, (f32)backBuffer->height);
    v2s originS = ToV2S(origin);
    v2s dimsS = ToV2S(dims);
    v2s min = originS - (dimsS / 2);
    v2s max = originS + (dimsS / 2);
    
    min = ClampV2S(min, V2S(0), V2S(backBuffer->width - 1, backBuffer->height - 1));
    max = ClampV2S(max, V2S(0), V2S(backBuffer->width - 1, backBuffer->height - 1));
    
    u8 *row = (u8 *)backBuffer->memory + (min.y * backBuffer->pitch);
    for (s32 y = min.y; y < max.y; ++y)
    {
        u32 *pixel = (u32 *)row + min.x;
        for (s32 x = min.x; x < max.x; ++x)
        {
            if (CheckPixelIsOnBackBuffer(pixel, backBuffer))
            {
                *pixel++ = ARGB_V4FToU32(colour * 255.0f);
            }
        }
        
        row += backBuffer->pitch;
    }
}

function void RenderBitmap(Game_BackBuffer *backBuffer, Bitmap *bitmap, v2f origin, v2f dims, f32 angle, v4f colour = V4F(1.0f))
{
    BEGIN_TIMED_BLOCK(RenderBitmap_Slow);
    
    v2f backBufferF = V2F((f32)backBuffer->width, (f32)backBuffer->height);
    
    angle -= TAU * 0.75f;
    v2f xAxis = dims * V2F(Cos(angle), Sin(angle));
    v2f yAxis = Perp(xAxis);
    
    v2s originS = ToV2S(origin);
    origin = ToV2F(originS) - (0.5f * xAxis) - (0.5f * yAxis);
    
    v2f invAxisLengthSq = 1.0f / V2F(LengthSq(xAxis), LengthSq(yAxis));
    
    v2s min = V2S(backBuffer->width, backBuffer->height);
    v2s max = V2S();
    
    v2f points[4] = {origin, origin + xAxis, origin + xAxis + yAxis, origin + yAxis};
    for (s32 pointIndex = 0; pointIndex < ARRAY_COUNT(points); ++pointIndex)
    {
        v2f testPoint = points[pointIndex];
        v2s floor = V2S(FloorF32ToS32(testPoint.x), FloorF32ToS32(testPoint.y));
        v2s ceil = V2S(CeilF32ToS32(testPoint.x), CeilF32ToS32(testPoint.y));
        
        min = MinElementsV2S(min, floor);
        max = MaxElementsV2S(max, ceil);
    }
    
    min = MaxElementsV2S(min, V2S());
    max = MinElementsV2S(max, V2S(backBuffer->width, backBuffer->height));
    
    colour = MultiplyAlpha(colour);
    
    
    if (colour.a != 0.0f)
    {
        u8 *row = (u8 *)backBuffer->memory + (min.y * backBuffer->pitch) + (min.x * BITMAP_BYTES_PER_PIXEL);
        for (s32 y = min.y; y < max.y; ++y)
        {
            u32 *pixel = (u32 *)row;
            for (s32 x = min.x; x < max.x; ++x)
            {
                v2f pixelPoint = V2F((f32)x, (f32)y);
                v2f pixelDiff = pixelPoint - origin;
                
                if (Dot(pixelDiff, -Perp(xAxis)) < 0.0f &&
                    Dot(pixelDiff - xAxis, -Perp(yAxis)) < 0.0f &&
                    Dot(pixelDiff - xAxis - yAxis, Perp(xAxis)) < 0.0f &&
                    Dot(pixelDiff - yAxis, Perp(yAxis)) < 0.0f)
                {
                    v2f uv = invAxisLengthSq * V2F(Dot(pixelDiff, xAxis), Dot(pixelDiff, yAxis));
                    uv = ClampV2F(uv, V2F(0.0f), V2F(1.0f));
                    
                    v2f texelPoint = uv * ToV2F(bitmap->dims - 2);
                    v2s basedPoint = ToV2S(texelPoint);
                    v2f floatPoint = texelPoint - ToV2F(basedPoint);
                    
                    BilinearSampleResult sample = BilinearSample(bitmap, basedPoint);
                    v4f texel = BilinearBlend_SRGB(sample, floatPoint);
                    texel = Hadamard(texel, colour);
                    
                    v4f dest = SRGBToLinear(ARGB_U32ToV4F(*pixel));
                    v4f destLinear = (1.0f - texel.a) * dest + texel;
                    v4f destSRGB = LinearToSRGB(destLinear);
                    
                    *pixel = ARGB_V4FToU32Rounded(destSRGB);
                }
                
                ++pixel;
            }
            
            row += backBuffer->pitch;
        }
    }
    
    END_TIMED_BLOCK(RenderBitmap_Slow);
}

// TODO(bSalmon): Eventually get rid of this for instead using Bitmaps for each glyph
// NOTE(bSalmon): Will crash if it goes past screen bounds, this is acceptable for now
function void RenderText(Game_BackBuffer *backBuffer, stbtt_fontinfo *fontInfo, char *text, TextAlign align, v2f origin, f32 lineHeight, v4f colour, PlatformAPI platform)
{
    v2s pos = ToV2S(origin);
    
    if (align == TextAlign_Center)
    {
        pos.y -= RoundF32ToS32(lineHeight / 2.0f);
    }
    
    s32 bitmapWidth = 0;
    s32 bitmapHeight = RoundF32ToS32(lineHeight);
    u8 *textBitmap = 0;
    
    f32 scale = stbtt_ScaleForPixelHeight(fontInfo, lineHeight);
    s32 ascent = ttSHORT(fontInfo->data + fontInfo->hhea + 4);
    
    ascent = RoundF32ToS32((f32)ascent * scale);
    
    s32 textX = 0;
    s32 pass = 0;
    while (pass < 2)
    {
        textX = 0;
        for (s32 i = 0; i < StringLength(text); ++i)
        {
            s32 charWidth, lsb;
            stbtt_GetCodepointHMetrics(fontInfo, text[i], &charWidth, &lsb);
            /* (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].) */
            
            if (pass > 0)
            {
                /* get bounding box for character (may be offset to account for chars that dip above or below the line */
                s32 c_x1, c_y1, c_x2, c_y2;
                stbtt_GetGlyphBitmapBoxSubpixel(fontInfo, stbtt_FindGlyphIndex(fontInfo, text[i]), scale, scale, 0.0f, 0.0f, &c_x1, &c_y1, &c_x2, &c_y2);
                
                /* compute y (different characters have different heights */
                s32 y = ascent + c_y1;
                
                /* render character (stride and offset is important here) */
                s32 byteOffset = textX + RoundF32ToS32(lsb * scale) + (y * bitmapWidth);
                stbtt_MakeCodepointBitmap(fontInfo, textBitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, bitmapWidth, scale, scale, text[i]);
            }
            
            /* advance x */
            textX += RoundF32ToS32(charWidth * scale);
            
            /* add kerning */
            s32 kern;
            kern = stbtt_GetCodepointKernAdvance(fontInfo, text[i], text[i + 1]);
            textX += RoundF32ToS32(kern * scale);
        }
        
        if (pass == 0)
        {
            bitmapWidth = textX;
            textBitmap = (u8 *)platform.MemAlloc(bitmapWidth * bitmapHeight);
        }
        
        pass++;
    }
    
    if (align == TextAlign_Center)
    {
        pos.x -= (textX / 2);
    }
    
    u32 colourU = ARGB_V4FToU32Rounded(colour * 255.0f);
    u8 *row = (u8 *)backBuffer->memory + (pos.y * backBuffer->pitch);
    for (s32 y = 0; y < bitmapHeight; ++y)
    {
        u32 *pixel = (u32 *)row + pos.x;
        for (s32 x = 0; x < bitmapWidth; ++x)
        {
            s32 textBitmapOffset = (y * bitmapWidth) + x;
            if (textBitmap[textBitmapOffset])
            {
                u8 pixelR = (*pixel >> 16) & 0xFF;
                u8 pixelG = (*pixel >> 8) & 0xFF;
                u8 pixelB = *pixel & 0xFF;
                
                u8 colourR = (colourU >> 16) & 0xFF;
                u8 colourG = (colourU >> 8) & 0xFF;
                u8 colourB = colourU & 0xFF;
                
                u8 newR = Lerp(pixelR, colourR, ((float)textBitmap[textBitmapOffset] / 255.0f));
                u8 newG = Lerp(pixelG, colourG, ((float)textBitmap[textBitmapOffset] / 255.0f));
                u8 newB = Lerp(pixelB, colourB, ((float)textBitmap[textBitmapOffset] / 255.0f));
                
                *pixel++ = (0xFF << 24) | (newR << 16) | (newG << 8) | newB;
            }
            else
            {
                pixel++;
            }
        }
        
        row += backBuffer->pitch;
    }
    
    platform.MemFree(textBitmap);
}

function RenderGroup *AllocateRenderGroup(MemoryRegion *memRegion, usize maxPushBufferSize, v2f worldToPixelConversion)
{
    RenderGroup *result = PushStruct(memRegion, RenderGroup);
    result->pushBufferBase = (u8 *)PushSize(memRegion, maxPushBufferSize);
    
    result->pushBufferSize = 0;
    result->maxPushBufferSize = maxPushBufferSize;
    
    result->worldToPixelConversion = worldToPixelConversion;
    
    return result;
}

function void RenderToBuffer(Game_BackBuffer *backBuffer, RenderGroup *renderGroup)
{
    for (usize baseAddress = 0; baseAddress < renderGroup->pushBufferSize;)
    {
        RenderEntry_Header *header = (RenderEntry_Header *)(renderGroup->pushBufferBase + baseAddress);
        baseAddress += sizeof(RenderEntry_Header);
        
        switch (header->type)
        {
            case RenderEntryType_RenderEntry_Bitmap:
            {
                RenderEntry_Bitmap *entry = (RenderEntry_Bitmap *)(renderGroup->pushBufferBase + baseAddress);
                
                RenderBitmap(backBuffer, entry->bitmap, entry->offset, entry->dims, entry->angle, entry->colour);
                
                baseAddress += sizeof(RenderEntry_Bitmap);
            } break;
            
            case RenderEntryType_RenderEntry_Rect:
            {
                RenderEntry_Rect *entry = (RenderEntry_Rect *)(renderGroup->pushBufferBase + baseAddress);
                
                RenderRect(backBuffer, entry->offset, entry->dims, entry->angle, entry->colour);
                
                baseAddress += sizeof(RenderEntry_Rect);
            } break;
            
            case RenderEntryType_RenderEntry_Clear:
            {
                RenderEntry_Clear *entry = (RenderEntry_Clear *)(renderGroup->pushBufferBase + baseAddress);
                
                RenderClear(backBuffer, entry->colour);
                
                baseAddress += sizeof(RenderEntry_Clear);
            } break;
            
            case RenderEntryType_RenderEntry_Text:
            {
                RenderEntry_Text *entry = (RenderEntry_Text *)(renderGroup->pushBufferBase + baseAddress);
                
                RenderText(backBuffer, entry->fontInfo, entry->text, entry->align, entry->offset, entry->lineHeight, entry->colour, entry->platform);
                
                baseAddress += sizeof(RenderEntry_Text);
            } break;
            
            INVALID_DEFAULT;
        }
    }
}
