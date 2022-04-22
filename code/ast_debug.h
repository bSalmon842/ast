/*
Project: Asteroids
File: ast_debug.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_DEBUG_H

global b32 debug_info = false;
global b32 debug_cam = false;

#define TRANSLATION_UNIT_COUNT 2
#define MAX_DEBUG_TRANSLATION_UNIT_INFOS 256
#define MAX_DEBUG_FRAMES 64

struct DebugFrame
{
    u64 startClock;
    u64 totalClock;
    f32 frameTime;
};

struct DebugBlockInfo
{
    u64 cycles;
    u32 hits;
    
    char *name;
};

struct DebugBlockStats
{
    u64 minCycles;
    u64 maxCycles;
    
    u32 minHits;
    u32 maxHits;
};

struct DebugTable
{
    u32 frameIndex;
    DebugFrame frames[MAX_DEBUG_FRAMES];
    
    DebugBlockInfo blockInfos[TRANSLATION_UNIT_COUNT][MAX_DEBUG_TRANSLATION_UNIT_INFOS];
    DebugBlockInfo lastBlockInfos[TRANSLATION_UNIT_COUNT][MAX_DEBUG_TRANSLATION_UNIT_INFOS];
    DebugBlockStats lastBlockStats[TRANSLATION_UNIT_COUNT][MAX_DEBUG_TRANSLATION_UNIT_INFOS];
};

struct DebugSettings
{
    b32 timers;
    b32 colliders;
    b32 regions;
    b32 camMove;
    
    char options[4][32];
};

struct DebugState
{
    b32 inFrame;
    
    MemoryRegion dataRegion;
    DebugTable *table;
    
    DebugSettings settings;
    
    b32 memInitialised;
};

struct DebugAutoBlock
{
    u32 counter;
    
    DebugAutoBlock(u32 ctr, char *blockName)
    {
        this->counter = ctr;
        DEBUG_BLOCK_OPEN_(blockName, this->counter);
    }
    
    ~DebugAutoBlock()
    {
        DEBUG_BLOCK_CLOSE_(this->counter);
    }
};

#define AST_DEBUG_H
#endif //AST_DEBUG_H
