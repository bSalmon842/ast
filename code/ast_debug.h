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
    
    DebugCounterDatum data[DEBUG_DATUM_COUNT];
};

struct DebugState
{
    u32 datumIndex;
    u32 counterCount;
    
    DebugCounter counters[512];
    DebugFrameInfo frames[DEBUG_DATUM_COUNT];
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
