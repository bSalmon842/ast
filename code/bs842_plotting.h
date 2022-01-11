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
    
#ifndef BS842_MEMALLOC
#define BS842_MEMALLOC(size) calloc(1, size)
#define BS842_MEMFREE(mem) free(mem)
#endif
    
#ifndef BS842_BPP
#define BS842_BPP 4
#endif
    
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
        bool enabled;
        BS842_Plotting_Internal_BackBuffer backBuffer;
        
        void *optionValues; // 32bit values
        
        float *xData;
        float *yData;
        int datumCount;
    } BS842_Plot;
    
    typedef struct
    {
        bool initialised;
        BS842_Plot plots[8];
    } BS842_Plotting_Internal_Info;
    
    static BS842_Plotting_Internal_Info bs842_plIntInfo = {};
    
    BSDEF void BS842_Plotting_Init(int plotCount, int plotWidth, int plotHeight);
    
    BSDEF void BS842_Plotting_ChangePlotOption(int plotIndex, BS842_PlotOption option);
    BSDEF void BS842_Plotting_GetPlotOption(void *result, int plotIndex, BS842_PlotOption option);
    
    BSDEF void BS842_Plotting_PlotData(int plotIndex, float *xData, float *yData, int datumCount);
    
    BSDEF void BS842_Plotting_UpdatePlot(int plotIndex);
    BSDEF void BS842_Plotting_ResizePlot(int plotIndex, int width, int height);
    BSDEF void BS842_Plotting_GetPlotMemory(int plotIndex, void *destMem, int destPitch);
    
#ifdef __cplusplus
}
#endif

#define BS842_INCLUDE_PLOTTING_H
#endif

#ifdef BS842_PLOTTING_IMPLEMENTATION

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

BSDEF void BS842_Internal_DrawHorizontalLine(BS842_Plotting_Internal_BackBuffer *backBuffer, int xLeft, int xRight, int y, int colour)
{
    if (y >= 0 && y < backBuffer->height)
    {
        unsigned char *row = (unsigned char *)backBuffer->memory + (y * backBuffer->pitch);
        int *pixel = (int *)row + xLeft;
        for (int x = xLeft; x < xRight; ++x)
        {
            *pixel++ = colour;
        }
    }
}

BSDEF void BS842_Internal_DrawVerticalLine(BS842_Plotting_Internal_BackBuffer *backBuffer, int yTop, int yBottom, int x, int colour)
{
    if (x >= 0 && x < backBuffer->width)
    {
        unsigned char *row = (unsigned char *)backBuffer->memory + (yTop * backBuffer->pitch);
        for (s32 y = yTop; y < yBottom; ++y)
        {
            int *pixel = (int *)row + x;
            *pixel = colour;
            
            row += backBuffer->pitch;
        }
    }
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

BSDEF void BS842_Plotting_Init(int plotCount, int plotWidth, int plotHeight)
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
        
        currPlot->optionValues = BS842_MEMALLOC(sizeof(int) * PlotOpt_Count);
        int *intOptions = (int *)currPlot->optionValues;
        float *floatOptions = (float *)currPlot->optionValues;
        floatOptions[PlotOpt_MinValX] = 0.0f;
        floatOptions[PlotOpt_MaxValX] = 100.0f;
        floatOptions[PlotOpt_MinValY] = 0.0f;
        floatOptions[PlotOpt_MaxValY] = 100.0f;
        
        intOptions[PlotOpt_Colour1] = 0xFFFF0000;
        intOptions[PlotOpt_Colour2] = 0xFF00FF00;
        intOptions[PlotOpt_ColourBack] = 0xFFEEEEEE;
        intOptions[PlotOpt_ColourSub] = 0xFF888888;
        intOptions[PlotOpt_ColourMargin] = 0xFFCCCCCC;
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

BSDEF void BS842_Plotting_PlotData(int plotIndex, float *xData, float *yData, int datumCount)
{
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    if (plot->xData) { BS842_MEMFREE(plot->xData); }
    if (plot->yData) { BS842_MEMFREE(plot->yData); }
    
    plot->xData = (float *)BS842_MEMALLOC(datumCount * sizeof(float));
    plot->yData = (float *)BS842_MEMALLOC(datumCount * sizeof(float));
    plot->datumCount = datumCount;
    
    float maxValY = ((float *)plot->optionValues)[PlotOpt_MaxValY];
    for (int datumIndex = 0; datumIndex < datumCount; ++datumIndex)
    {
        plot->xData[datumIndex] = xData[datumIndex];
        plot->yData[datumIndex] = maxValY - yData[datumIndex];
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
    int colour1 = ((int *)plot->optionValues)[PlotOpt_Colour1];
    int colour2 = ((int *)plot->optionValues)[PlotOpt_Colour2];
    
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
    
    int plotMinX = marginLeft + 1;
    int plotMinY = marginTop + 1;
    int plotMaxX = rightEdge - marginRight;
    int plotMaxY = bottomEdge - marginBottom;
    
    float plotHeight = (float)(plotMaxY - plotMinY);
    
    float minValX = ((float *)plot->optionValues)[PlotOpt_MinValX];
    float maxValX = ((float *)plot->optionValues)[PlotOpt_MaxValX];
    float minValY = ((float *)plot->optionValues)[PlotOpt_MinValY];
    float maxValY = ((float *)plot->optionValues)[PlotOpt_MaxValY];
    
    float minorX = ((float *)plot->optionValues)[PlotOpt_MinorX];
    float majorX = ((float *)plot->optionValues)[PlotOpt_MajorX];
    float minorY = ((float *)plot->optionValues)[PlotOpt_MinorY];
    float majorY = ((float *)plot->optionValues)[PlotOpt_MajorY];
    
    int minorMarksXCount = 0;
    float *minorMarksX = 0;
    BS842_Internal_GetMarks(&minorMarksXCount, &minorMarksX, minValX, maxValX, minorX);
    
    int majorMarksXCount = 0;
    float *majorMarksX = 0;
    BS842_Internal_GetMarks(&majorMarksXCount, &majorMarksX, minValX, maxValX, majorX);
    
    int minorMarksYCount = 0;
    float *minorMarksY = 0;
    BS842_Internal_GetMarks(&minorMarksYCount, &minorMarksY, minValY, maxValY, minorY);
    
    int majorMarksYCount = 0;
    float *majorMarksY = 0;
    BS842_Internal_GetMarks(&majorMarksYCount, &majorMarksY, minValY, maxValY, majorY);
    
    for (int y = plotMinY; y < plotMaxY; ++y)
    {
        for (int minor = 0; minor < minorMarksYCount; ++minor)
        {
            if (BS842_Internal_GetValueInMappedRanges(minorMarksY[minor], minValY, maxValY, plotMinY, plotMaxY) == y)
            {
                BS842_Internal_DrawDottedHorizontalLine(&plot->backBuffer, plotMinX, plotMaxX, y, subColour);
            }
        }
        
        for (int major = 0; major < majorMarksYCount; ++major)
        {
            if (BS842_Internal_GetValueInMappedRanges(majorMarksY[major], minValY, maxValY, plotMinY, plotMaxY) == y)
            {
                BS842_Internal_DrawHorizontalLine(&plot->backBuffer, plotMinX, plotMaxX, y, subColour);
            }
        }
    }
    
    for (int x = plotMinX; x < plotMaxX; ++x)
    {
        for (int minor = 0; minor < minorMarksXCount; ++minor)
        {
            if (BS842_Internal_GetValueInMappedRanges(minorMarksX[minor], minValX, maxValX, plotMinX, plotMaxX) == x)
            {
                BS842_Internal_DrawDottedVerticalLine(&plot->backBuffer, plotMinY, plotMaxY, x, subColour);
            }
        }
        
        for (int major = 0; major < majorMarksXCount; ++major)
        {
            if (BS842_Internal_GetValueInMappedRanges(majorMarksX[major], minValX, maxValX, plotMinX, plotMaxX) == x)
            {
                BS842_Internal_DrawVerticalLine(&plot->backBuffer, plotMinY, plotMaxY, x, subColour);
            }
        }
    }
    
    if (minorMarksX) { BS842_MEMFREE(minorMarksX); }
    if (majorMarksX) { BS842_MEMFREE(majorMarksX); }
    if (minorMarksY) { BS842_MEMFREE(minorMarksY); }
    if (majorMarksY) { BS842_MEMFREE(majorMarksY); }
    
    // TODO(bSalmon): Draw Plot points
    for (int point = 0; point < plot->datumCount; ++point)
    {
        int xMapped = BS842_Internal_GetValueInMappedRanges(plot->xData[point], minValX, maxValX, plotMinX, plotMaxX);
        int yMapped = BS842_Internal_GetValueInMappedRanges(plot->yData[point], minValY, maxValY, plotMinY, plotMaxY);
        
        row = (unsigned char *)plot->backBuffer.memory + ((yMapped - 1) * plot->backBuffer.pitch);
        for (int y = yMapped - 1; y <= yMapped + 1; ++y)
        {
            int *pixel = (int *)row + (xMapped - 1);
            for (int x = xMapped - 1; x <= xMapped + 1; ++x)
            {
                *pixel++ = colour1;
            }
            
            row += plot->backBuffer.pitch;
        }
    }
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