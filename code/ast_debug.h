/*
Project: Asteroids
File: ast_debug.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_DEBUG_H

global b32 debug_info = false;
global b32 debug_colliders = false;
global b32 debug_cam = false;
global b32 debug_regions = false;
global b32 debug_camMove = false;

struct DebugCounterDatum
{
    u32 hits;
    u64 cycles;
};

struct DebugCounter
{
    char *fileName;
    char *blockName;
    u32 lineNumber;
};

struct DebugFrameSegment
{
    u32 laneIndex;
    f32 minT;
    f32 maxT;
};

struct DebugFrame
{
    u64 startClock;
    u64 finishClock;
    u32 segmentCount;
    DebugFrameSegment *segments;
};

struct DebugState
{
    
#if 0    
    u32 counterCount;
    DebugCounter counters[512];
#endif
    
    u32 frameCount;
    DebugFrame *frames;
    
    u32 visBarLaneCount;
    f32 visBarScale;
    
    MemoryRegion dataRegion;
    TempMemory dataTemp;
    b32 initialised;
};

struct DebugStat
{
    f64 min;
    f64 max;
    f64 ave;
    u32 count;
};

#define AST_DEBUG_H
#endif //AST_DEBUG_H
