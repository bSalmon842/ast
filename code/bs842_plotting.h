/*
File: bs842_plotting.h
Author: Brock Salmon
Notice: Public Domain

// This software is dual-licensed to the public domain and under the following
// license: you are granted a perpetual, irrevocable license to copy, modify,
// publish, and distribute this file as you see fit.
*/

// TODO(bSalmon): Mouse Input
// TODO(bSalmon): Highlighting
// TODO(bSalmon): Hover

#ifndef BS842_INCLUDE_PLOTTING_H

#ifdef __cplusplus
extern "C"
{
#endif
    
#ifndef BS842_FMOD
#include <math.h>
#define BS842_FMOD(val, modVal) fmod(val, modVal)
#endif
    
#ifndef BS842_STRLEN
#include <string.h>
#define BS842_STRLEN(string) strlen(string)
#endif
    
    // TODO(bSalmon): FOR TESTING ONLY, GUT THE FUNCTIONS FROM STB_TRUETYPE NEEDED INTO THIS FILE
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
    
#ifndef BSDEF
#ifdef BS842_PLOTTING_STATIC
#define BSDEF static
#else
#define BSDEF extern
#endif
#endif
    
#ifndef BS842_ASSERT
#define BS842_ASSERT(check) if(!(check)) {*(int *)0 = 0;}
#endif
    
#ifndef BS842_ARRAY_COUNT
#define BS842_ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#endif
    
#ifndef BS842_MEMALLOC
#define BS842_MEMALLOC(size) calloc(1, size)
#define BS842_MEMFREE(mem) free(mem)
#endif
    
#ifndef BS842_BPP
#define BS842_BPP 4
#endif
    
    struct BS842_Internal_V2F
    {
        union
        {
            float e[2];
            struct {float x; float y;};
        };
    };
    
    struct BS842_Internal_V2I
    {
        union
        {
            int e[2];
            struct {int x; int y;};
        };
    };
    
    enum BS842_PlotOption
    {
        PlotOpt_MinValX,
        PlotOpt_MaxValX,
        PlotOpt_MinValY,
        PlotOpt_MaxValY,
        
        PlotOpt_Colour1,
        PlotOpt_Colour2,
        PlotOpt_ColourBack,
        PlotOpt_ColourSub,
        PlotOpt_ColourMargin,
        PlotOpt_ColourBorders,
        
        PlotOpt_MarginLeft,
        PlotOpt_MarginRight,
        PlotOpt_MarginBottom,
        PlotOpt_MarginTop,
        
        PlotOpt_MinorX,
        PlotOpt_MajorX,
        PlotOpt_MinorY,
        PlotOpt_MajorY,
        
        PlotOpt_Count,
    };
    
    typedef struct
    {
        int width;
        int height;
        int pitch;
        void *memory;
    } BS842_Plotting_Internal_BackBuffer;
    
    typedef struct
    {
        float *xData;
        float *yData;
        int datumCount;
    } BS842_DataSet;
    
    typedef struct
    {
        bool enabled;
        BS842_Plotting_Internal_BackBuffer backBuffer;
        
        void *optionValues; // 32bit values
        
        stbtt_fontinfo fontInfo;
        
        BS842_DataSet data[2];
    } BS842_Plot;
    
    typedef struct
    {
        bool initialised;
        BS842_Plot plots[8];
    } BS842_Plotting_Internal_Info;
    
    static BS842_Plotting_Internal_Info bs842_plIntInfo = {};
    
    BSDEF void BS842_Plotting_Init(int plotCount, int plotWidth, int plotHeight, char *fontPath);
    
    BSDEF void BS842_Plotting_ChangePlotOption(int plotIndex, BS842_PlotOption option);
    BSDEF void BS842_Plotting_GetPlotOption(void *result, int plotIndex, BS842_PlotOption option);
    
    BSDEF void BS842_Plotting_PlotData(int plotIndex, s32 dataSetIndex, float *xData, float *yData, int datumCount);
    
    BSDEF void BS842_Plotting_UpdatePlot(int plotIndex);
    BSDEF void BS842_Plotting_ResizePlot(int plotIndex, int width, int height);
    BSDEF void BS842_Plotting_GetPlotMemory(int plotIndex, void *destMem, int destPitch);
    
#ifdef __cplusplus
}
#endif

#define BS842_INCLUDE_PLOTTING_H
#endif

#ifdef BS842_PLOTTING_IMPLEMENTATION

BSDEF unsigned char BS842_Internal_Lerp(unsigned char a, unsigned char b, float t)
{
    unsigned char result = (unsigned char)((1.0f - t) * a + t * b);
    return result;
}

BSDEF void BS842_Internal_DrawText(BS842_Plotting_Internal_BackBuffer *backBuffer, stbtt_fontinfo fontInfo, char *text, BS842_Internal_V2I pos, float lineHeight, int allowedWidth, int colour)
{
    // NOTE(bSalmon): pos is of the center of the height of the text, and on the far right of the 
    pos.y -= (int)(lineHeight / 2.0f + 0.5f);
    
    int bitmapHeight = (int)(lineHeight * 1.5f);
    int bitmapWidth = allowedWidth;
    unsigned char *textBitmap = (unsigned char *)BS842_MEMALLOC(bitmapWidth * bitmapHeight);
    
    float scale = stbtt_ScaleForPixelHeight(&fontInfo, lineHeight);
    
    int textX = 0;
    
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
    
    ascent = (int)((float)ascent * scale + 0.5f);
    descent = (int)((float)descent * scale + 0.5f);
    
    for (int i = 0; i < BS842_STRLEN(text); ++i)
    {
        int charWidth;
        int lsb;
        stbtt_GetCodepointHMetrics(&fontInfo, text[i], &charWidth, &lsb);
        /* (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].) */
        
        /* get bounding box for character (may be offset to account for chars that dip above or below the line */
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&fontInfo, text[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
        
        /* compute y (different characters have different heights */
        int y = ascent + c_y1;
        
        /* render character (stride and offset is important here) */
        int byteOffset = textX + int(lsb * scale + 0.5f) + (y * bitmapWidth);
        stbtt_MakeCodepointBitmap(&fontInfo, textBitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, bitmapWidth, scale, scale, text[i]);
        
        /* advance x */
        textX += (int)(charWidth * scale + 0.5f);
        
        /* add kerning */
        int kern;
        kern = stbtt_GetCodepointKernAdvance(&fontInfo, text[i], text[i + 1]);
        textX += (int)(kern * scale + 0.5f);
    }
    
    pos.x -= (textX / 2);
    
    // TODO(bSalmon): Anti-aliasing for readability
    unsigned char *row = (unsigned char *)backBuffer->memory + (pos.y * backBuffer->pitch);
    for (s32 y = 0; y < bitmapHeight; ++y)
    {
        int *pixel = (int *)row + pos.x;
        for (s32 x = 0; x < bitmapWidth; ++x)
        {
            int textBitmapOffset = (y * bitmapWidth) + x;
            if (textBitmap[textBitmapOffset])
            {
                unsigned char pixelR = (*pixel >> 16) & 0xFF;
                unsigned char pixelG = (*pixel >> 8) & 0xFF;
                unsigned char pixelB = *pixel & 0xFF;
                
                unsigned char colourR = (colour >> 16) & 0xFF;
                unsigned char colourG = (colour >> 8) & 0xFF;
                unsigned char colourB = colour & 0xFF;
                
                unsigned char newR = BS842_Internal_Lerp(pixelR, colourR, ((float)textBitmap[textBitmapOffset] / 255.0f));
                unsigned char newG = BS842_Internal_Lerp(pixelG, colourG, ((float)textBitmap[textBitmapOffset] / 255.0f));
                unsigned char newB = BS842_Internal_Lerp(pixelB, colourB, ((float)textBitmap[textBitmapOffset] / 255.0f));
                
                *pixel++ = (0xFF << 24) | (newR << 16) | (newG << 8) | newB;
            }
            else
            {
                pixel++;
            }
        }
        
        row += backBuffer->pitch;
    }
}

BSDEF void BS842_Internal_GetMarks(int *marksCount, float **marks, float minVal, float maxVal, float modVal)
{
    for (float val = minVal; val < maxVal; val += 1.0f)
    {
        double check = BS842_FMOD(val, modVal);
        if (check == 0.000)
        {
            (*marksCount)++;
        }
    }
    
    if (*marksCount > 0)
    {
        *marks = (float *)BS842_MEMALLOC(sizeof(float) * *marksCount);
    }
    
    int markIndex = 0;
    for (float val = minVal; val < maxVal; val += 1.0f)
    {
        double check = BS842_FMOD(val, modVal);
        if (check == 0.000)
        {
            (*marks)[markIndex++] = val;
        }
    }
}

BSDEF int BS842_Internal_GetValueInMappedRanges(float inValue, float inMin, float inMax, int mapMin, int mapMax)
{
    int result = 0;
    
    double slope = 1.0 * ((double)mapMax - (double)mapMin) / (inMax - inMin);
    result = (int)(((double)mapMin + slope * (inValue - inMin)) + 0.5);
    
    return result;
}

BSDEF void BS842_Internal_DrawDottedHorizontalLine(BS842_Plotting_Internal_BackBuffer *backBuffer, int xLeft, int xRight, int y, int colour)
{
    if (y >= 0 && y < backBuffer->height)
    {
        unsigned char *row = (unsigned char *)backBuffer->memory + (y * backBuffer->pitch);
        int *pixel = (int *)row + xLeft;
        for (int x = xLeft; x < xRight; ++x)
        {
            if ((x % 10) < 6)
            {
                *pixel++ = colour;
            }
            else
            {
                pixel++;
            }
        }
    }
}

BSDEF void BS842_Internal_DrawDottedVerticalLine(BS842_Plotting_Internal_BackBuffer *backBuffer, int yTop, int yBottom, int x, int colour)
{
    if (x >= 0 && x < backBuffer->width)
    {
        unsigned char *row = (unsigned char *)backBuffer->memory + (yTop * backBuffer->pitch);
        for (int y = yTop; y < yBottom; ++y)
        {
            if ((y % 10) < 6)
            {
                int *pixel = (int *)row + x;
                *pixel = colour;
            }
            
            row += backBuffer->pitch;
        }
    }
}

BSDEF void BS842_Internal_DrawBresenhamLine(BS842_Plotting_Internal_BackBuffer *backBuffer, BS842_Internal_V2I start, BS842_Internal_V2I end, BS842_Internal_V2I minBounds, BS842_Internal_V2I maxBounds, float width, int colour)
{
    // NOTE(bSalmon): http://members.chello.at/~easyfilter/bresenham.html
    int dx = abs(end.x - start.x), sx = start.x < end.x ? 1 : -1; 
    int dy = abs(end.y - start.y), sy = start.y < end.y ? 1 : -1; 
    int err = dx - dy, e2, x2, y2;                          /* error value e_xy */
    float ed = dx + dy == 0 ? 1 : (float)sqrt((float)dx*dx+(float)dy*dy);
    
    int *pixel = (int *)backBuffer->memory;
    int x = 0, y = 0;
    for (width = (width + 1) / 2; ; )
    {
        x = start.x;
        y = start.y;
        if (x >= minBounds.x && y >= minBounds.y && x <= maxBounds.x && y <= maxBounds.y)
        {
            pixel = (int *)backBuffer->memory + (y * backBuffer->width) + x;
            *pixel = colour;
        }
        
        e2 = err; x2 = start.x;
        if (2 * e2 >= -dx)
        {
            for (e2 += dy, y2 = start.y; e2 < ed * width && (end.y != y2 || dx > dy); e2 += dx)
            {
                x = start.x;
                y = (y2 += sy);
                if (x >= minBounds.x && y >= minBounds.y && x <= maxBounds.x && y <= maxBounds.y)
                {
                    pixel = (int *)backBuffer->memory + (y * backBuffer->width) + x;
                    *pixel = colour;
                }
            }
            if (start.x == end.x) break;
            e2 = err; err -= dy; start.x += sx; 
        }
        if (2 * e2 <= dy)
        {
            for (e2 = dx - e2; e2 < ed * width && (end.x != x2 || dx < dy); e2 += dy)
            {
                x = (x2 += sx);
                y = start.y;
                if (x >= minBounds.x && y >= minBounds.y && x <= maxBounds.x && y <= maxBounds.y)
                {
                    pixel = (int *)backBuffer->memory + (y * backBuffer->width) + x;
                    *pixel = colour;
                }
            }
            if (start.y == end.y) break;
            err += dx; start.y += sy; 
        }
    }
}

BSDEF void BS842_Plotting_Init(int plotCount, int plotWidth, int plotHeight, char *fontPath)
{
    BS842_ASSERT(!bs842_plIntInfo.initialised);
    
    for (int plotIndex = 0; plotIndex < plotCount; ++plotIndex)
    {
        BS842_Plot *currPlot = &bs842_plIntInfo.plots[plotIndex];
        
        currPlot->enabled = true;
        
        currPlot->backBuffer.width = plotWidth;
        currPlot->backBuffer.height = plotHeight;
        currPlot->backBuffer.pitch = plotWidth * BS842_BPP;
        currPlot->backBuffer.memory = BS842_MEMALLOC(currPlot->backBuffer.height * currPlot->backBuffer.pitch);
        
        unsigned char* fontBuffer;
        int fontSize = 0;
        
        FILE* fontFile = fopen(fontPath, "rb");
        fseek(fontFile, 0, SEEK_END);
        fontSize = ftell(fontFile); /* how long is the file ? */
        fseek(fontFile, 0, SEEK_SET); /* reset */
        
        fontBuffer = (unsigned char *)BS842_MEMALLOC(fontSize);
        fread(fontBuffer, fontSize, 1, fontFile);
        fclose(fontFile);
        
        /* prepare font */
        bool fontInitResult = false;
        fontInitResult = stbtt_InitFont(&currPlot->fontInfo, fontBuffer, 0);
        BS842_ASSERT(fontInitResult);
        
        currPlot->optionValues = BS842_MEMALLOC(sizeof(int) * PlotOpt_Count);
        int *intOptions = (int *)currPlot->optionValues;
        float *floatOptions = (float *)currPlot->optionValues;
        floatOptions[PlotOpt_MinValX] = 0.0f;
        floatOptions[PlotOpt_MaxValX] = 100.0f;
        floatOptions[PlotOpt_MinValY] = 0.0f;
        floatOptions[PlotOpt_MaxValY] = 100.0f;
        
        intOptions[PlotOpt_Colour1] = 0xFFFF0000;
        intOptions[PlotOpt_Colour2] = 0xFF00FF00;
        intOptions[PlotOpt_ColourBack] = 0xFFDDDDDD;
        intOptions[PlotOpt_ColourSub] = 0xFF888888;
        intOptions[PlotOpt_ColourMargin] = 0xFFBBBBBB;
        intOptions[PlotOpt_ColourBorders] = 0xFF000000;
        
        intOptions[PlotOpt_MarginLeft] = 75;
        intOptions[PlotOpt_MarginRight] = 15;
        intOptions[PlotOpt_MarginBottom] = 30;
        intOptions[PlotOpt_MarginTop] = 15;
        
        // TODO(bSalmon): Should this be percentage or value based? (Value based for now)
        floatOptions[PlotOpt_MinorX] = 5.0f;
        floatOptions[PlotOpt_MajorX] = 10.0f;
        floatOptions[PlotOpt_MinorY] = 5.0f;
        floatOptions[PlotOpt_MajorY] = 25.0f;
        
    }
    
    bs842_plIntInfo.initialised = true;
}

BSDEF void BS842_Plotting_ChangePlotOption(int plotIndex, BS842_PlotOption option, int intVal, float fltVal)
{
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    switch (option)
    {
        case PlotOpt_MinValX:
        case PlotOpt_MaxValX:
        case PlotOpt_MinValY:
        case PlotOpt_MaxValY:
        case PlotOpt_MinorX:
        case PlotOpt_MajorX:
        case PlotOpt_MinorY:
        case PlotOpt_MajorY:
        {
            float *floatOptions = (float *)plot->optionValues;
            floatOptions[option] = fltVal;
        } break;
        
        default:
        {
            int *intOptions = (int *)plot->optionValues;
            intOptions[option] = intVal;
        } break;
    }
}

BSDEF void BS842_Plotting_GetPlotOption(void *result, int plotIndex, BS842_PlotOption option)
{
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    switch (option)
    {
        case PlotOpt_MinValX:
        case PlotOpt_MaxValX:
        case PlotOpt_MinValY:
        case PlotOpt_MaxValY:
        case PlotOpt_MinorX:
        case PlotOpt_MajorX:
        case PlotOpt_MinorY:
        case PlotOpt_MajorY:
        {
            float *floatOptions = (float *)plot->optionValues;
            *(float *)result = floatOptions[option];
        } break;
        
        default:
        {
            int *intOptions = (int *)plot->optionValues;
            *(int *)result = intOptions[option];
        } break;
    }
    
}

BSDEF void BS842_Plotting_PlotData(int plotIndex, s32 dataSetIndex, float *xData, float *yData, int datumCount)
{
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    if (plot->data[dataSetIndex].xData) { BS842_MEMFREE(plot->data[dataSetIndex].xData); }
    if (plot->data[dataSetIndex].yData) { BS842_MEMFREE(plot->data[dataSetIndex].yData); }
    
    plot->data[dataSetIndex].xData = (float *)BS842_MEMALLOC(datumCount * sizeof(float));
    plot->data[dataSetIndex].yData = (float *)BS842_MEMALLOC(datumCount * sizeof(float));
    plot->data[dataSetIndex].datumCount = datumCount;
    
    float maxValY = ((float *)plot->optionValues)[PlotOpt_MaxValY];
    for (int datumIndex = 0; datumIndex < datumCount; ++datumIndex)
    {
        plot->data[dataSetIndex].xData[datumIndex] = xData[datumIndex];
        plot->data[dataSetIndex].yData[datumIndex] = maxValY - yData[datumIndex];
    }
}

BSDEF void BS842_Plotting_UpdatePlot(int plotIndex)
{
    // TODO(bSalmon): SIMD
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    int rightEdge = plot->backBuffer.width - 1;
    int bottomEdge = plot->backBuffer.height - 1;
    
    int borderColour = ((int *)plot->optionValues)[PlotOpt_ColourBorders];
    int backgroundColour = ((int *)plot->optionValues)[PlotOpt_ColourBack];
    int marginColour = ((int *)plot->optionValues)[PlotOpt_ColourMargin];
    int subColour = ((int *)plot->optionValues)[PlotOpt_ColourSub];
    
    int colour[2] = {((int *)plot->optionValues)[PlotOpt_Colour1], ((int *)plot->optionValues)[PlotOpt_Colour2]};
    
    int marginLeft = ((int *)plot->optionValues)[PlotOpt_MarginLeft];
    int marginRight = ((int *)plot->optionValues)[PlotOpt_MarginRight];
    int marginTop = ((int *)plot->optionValues)[PlotOpt_MarginTop];
    int marginBottom = ((int *)plot->optionValues)[PlotOpt_MarginBottom];
    
    unsigned char *row = (unsigned char *)plot->backBuffer.memory;
    for (int y = 0; y < plot->backBuffer.height; ++y)
    {
        int *pixel = (int *)row;
        for (int x = 0; x < plot->backBuffer.width; ++x)
        {
            if (x == 0 || y == 0 ||
                x == rightEdge || y == bottomEdge ||
                (x == marginLeft && y >= marginTop && y <= bottomEdge - marginBottom) ||
                (x == rightEdge - marginRight && y >= marginTop && y <= bottomEdge - marginBottom) ||
                (y == marginTop && x >= marginLeft && x <= rightEdge - marginRight) ||
                (y == bottomEdge - marginBottom && x >= marginLeft && x <= rightEdge - marginRight))
            {
                *pixel++ = borderColour;
            }
            else if (x < marginLeft || y < marginTop ||
                     x > rightEdge - marginRight || y > bottomEdge - marginBottom)
            {
                *pixel++ = marginColour;
            }
            else
            {
                *pixel++ = backgroundColour;
            }
            
        }
        
        row += plot->backBuffer.pitch;
        
    }
    
    BS842_Internal_V2I plotMin = {marginLeft + 1, marginTop + 1};
    BS842_Internal_V2I plotMax = {rightEdge - marginRight, bottomEdge - marginBottom};
    
    float plotHeight = (float)(plotMax.y - plotMin.y);
    
    BS842_Internal_V2F minVal = {((float *)plot->optionValues)[PlotOpt_MinValX], ((float *)plot->optionValues)[PlotOpt_MinValY]};
    BS842_Internal_V2F maxVal = {((float *)plot->optionValues)[PlotOpt_MaxValX], ((float *)plot->optionValues)[PlotOpt_MaxValY]};
    
    BS842_Internal_V2F minor = {((float *)plot->optionValues)[PlotOpt_MinorX], ((float *)plot->optionValues)[PlotOpt_MinorY]};
    BS842_Internal_V2F major = {((float *)plot->optionValues)[PlotOpt_MajorX], ((float *)plot->optionValues)[PlotOpt_MajorY]};
    
    BS842_Internal_V2I minorMarksCount = {0, 0};
    BS842_Internal_V2I majorMarksCount = {0, 0};
    
    float *minorMarksX = 0;
    BS842_Internal_GetMarks(&minorMarksCount.x, &minorMarksX, minVal.x, maxVal.x, minor.x);
    
    float *majorMarksX = 0;
    BS842_Internal_GetMarks(&majorMarksCount.x, &majorMarksX, minVal.x, maxVal.x, major.x);
    
    float *minorMarksY = 0;
    BS842_Internal_GetMarks(&minorMarksCount.y, &minorMarksY, minVal.y, maxVal.y, minor.y);
    
    float *majorMarksY = 0;
    BS842_Internal_GetMarks(&majorMarksCount.y, &majorMarksY, minVal.y, maxVal.y, major.y);
    
    for (int y = plotMin.y; y < plotMax.y; ++y)
    {
        for (int minorIndex = 0; minorIndex < minorMarksCount.y; ++minorIndex)
        {
            if (BS842_Internal_GetValueInMappedRanges(minorMarksY[minorIndex], minVal.y, maxVal.y, plotMin.y, plotMax.y) == y)
            {
                BS842_Internal_DrawDottedHorizontalLine(&plot->backBuffer, plotMin.x, plotMax.x, y, subColour);
            }
        }
        
        for (int majorIndex = 0; majorIndex < majorMarksCount.y; ++majorIndex)
        {
            if (BS842_Internal_GetValueInMappedRanges(majorMarksY[majorIndex], minVal.y, maxVal.y, plotMin.y, plotMax.y) == y)
            {
                BS842_Internal_DrawBresenhamLine(&plot->backBuffer, {plotMin.x, y}, {plotMax.x, y}, plotMin, plotMax, 1.0f, subColour);
                
                // TODO(bSalmon): Calc x gap from line using string length
                float textLineHeight = 13.0f;
                char majorText[16] = {};
                sprintf(majorText, "%.01f\0", majorMarksY[(majorMarksCount.y - 1) - majorIndex] + major.y);
                BS842_Internal_DrawText(&plot->backBuffer, plot->fontInfo, majorText, {plotMin.x - (int)textLineHeight * 2, y},
                                        textLineHeight, (int)(marginLeft * 0.75f), 0xFF000000);
            }
        }
    }
    
    for (int x = plotMin.x; x < plotMax.x; ++x)
    {
        for (int minorIndex = 0; minorIndex < minorMarksCount.x; ++minorIndex)
        {
            if (BS842_Internal_GetValueInMappedRanges(minorMarksX[minorIndex], minVal.x, maxVal.x, plotMin.x, plotMax.x) == x)
            {
                BS842_Internal_DrawDottedVerticalLine(&plot->backBuffer, plotMin.y, plotMax.y, x, subColour);
            }
        }
        
        for (int majorIndex = 0; majorIndex < majorMarksCount.x; ++majorIndex)
        {
            if (BS842_Internal_GetValueInMappedRanges(majorMarksX[majorIndex], minVal.x, maxVal.x, plotMin.x, plotMax.x) == x)
            {
                BS842_Internal_DrawBresenhamLine(&plot->backBuffer, {x, plotMin.y}, {x, plotMax.y}, plotMin, plotMax, 1.0f, subColour);
                
                float textLineHeight = 13.0f;
                char majorText[16] = {};
                sprintf(majorText, "%.01f\0", majorMarksX[majorIndex]);
                BS842_Internal_DrawText(&plot->backBuffer, plot->fontInfo, majorText, {x, plotMax.y + (int)textLineHeight},
                                        textLineHeight, (int)(marginLeft * 0.75f), 0xFF000000);
            }
        }
    }
    
    if (minorMarksX) { BS842_MEMFREE(minorMarksX); }
    if (majorMarksX) { BS842_MEMFREE(majorMarksX); }
    if (minorMarksY) { BS842_MEMFREE(minorMarksY); }
    if (majorMarksY) { BS842_MEMFREE(majorMarksY); }
    
    for (int dataSetIndex = 0; dataSetIndex < BS842_ARRAY_COUNT(plot->data); ++dataSetIndex)
    {
        BS842_Internal_V2I prevMapped = {0, 0};
        for (int point = 0; point < plot->data[dataSetIndex].datumCount; ++point)
        {
            BS842_Internal_V2I mapped =
            {BS842_Internal_GetValueInMappedRanges(plot->data[dataSetIndex].xData[point], minVal.x, maxVal.x, plotMin.x, plotMax.x),
                BS842_Internal_GetValueInMappedRanges(plot->data[dataSetIndex].yData[point], minVal.y, maxVal.y, plotMin.y, plotMax.y)};
            
            if (point > 0)
            {
                BS842_Internal_DrawBresenhamLine(&plot->backBuffer, prevMapped, mapped, plotMin, plotMax, 1.0f, colour[dataSetIndex]);
            }
            
            prevMapped = mapped;
        }
    }
    
#if 0    
    // TODO(bSalmon): Calc x gap from line using string length
    float textLineHeight = 13.0f;
    
    char yMaxText[16] = {};
    sprintf(yMaxText, "%.01f\0", maxVal.y);
    BS842_Internal_DrawText(&plot->backBuffer, plot->fontInfo, yMaxText, {plotMin.x - (int)textLineHeight * 2, plotMin.y},
                            textLineHeight, (int)(marginLeft * 0.75f), 0xFF000000);
    
    char yMinText[16] = {};
    sprintf(yMinText, "%.01f\0", minVal.y);
    BS842_Internal_DrawText(&plot->backBuffer, plot->fontInfo, yMinText, {plotMin.x - (int)textLineHeight * 2, plotMax.y},
                            textLineHeight, (int)(marginLeft * 0.75f), 0xFF000000);
    
    char xMinText[16] = {};
    sprintf(xMinText, "%.01f\0", minVal.x);
    BS842_Internal_DrawText(&plot->backBuffer, plot->fontInfo, xMinText, {plotMin.x, plotMax.y + (int)textLineHeight},
                            textLineHeight, (int)(marginLeft * 0.75f), 0xFF000000);
    
    char xMaxText[16] = {};
    sprintf(xMaxText, "%.01f\0", maxVal.x);
    BS842_Internal_DrawText(&plot->backBuffer, plot->fontInfo, xMaxText, {plotMax.x, plotMax.y + (int)textLineHeight},
                            textLineHeight, (int)(marginLeft * 0.75f), 0xFF000000);
#endif
}

BSDEF void BS842_Plotting_ResizePlot(int plotIndex, int width, int height)
{
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    BS842_MEMFREE(plot->backBuffer.memory);
    
    plot->backBuffer.width = width;
    plot->backBuffer.height = height;
    plot->backBuffer.pitch = width * BS842_BPP;
    plot->backBuffer.memory = BS842_MEMALLOC(plot->backBuffer.height * plot->backBuffer.pitch);
}

BSDEF void BS842_Plotting_GetPlotMemory(int plotIndex, void *destMem, int destPitch)
{
    // TODO(bSalmon): SIMD
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    unsigned char *srcRow = (unsigned char *)plot->backBuffer.memory;
    unsigned char *destRow = (unsigned char *)destMem;
    for (int y = 0; y < plot->backBuffer.height; ++y)
    {
        int *srcPixel = (int *)srcRow;
        int *destPixel = (int *)destRow;
        for (int x = 0; x < plot->backBuffer.width; ++x)
        {
            *destPixel++ = *srcPixel++;
        }
        
        srcRow += plot->backBuffer.pitch;
        destRow += destPitch;
    }
}

#endif