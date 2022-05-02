/*
Project: Asteroids
File: ast_debug.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_DEBUG_H

#define TRANSLATION_UNIT_COUNT 2
#define MAX_DEBUG_TRANSLATION_UNIT_INFOS 256
#define MAX_DEBUG_FRAMES 64

#define DEBUG_TEXT_SCALE 0.75f
#define DEBUG_LAYER 200

struct DebugBlockInfo
{
    u64 cycles;
    u32 hits;
    b32 isMarker;
    
    char *name;
};

struct DebugBlockStats
{
    u64 minCycles;
    u64 maxCycles;
    
    u32 minHits;
    u32 maxHits;
};

struct DebugFrame
{
    u64 startClock;
    u64 totalClock;
    f32 frameTime;
    
    DebugBlockInfo blockInfos[TRANSLATION_UNIT_COUNT][MAX_DEBUG_TRANSLATION_UNIT_INFOS];
};

struct DebugTable
{
    u32 frameIndex;
    DebugFrame frames[MAX_DEBUG_FRAMES];
    
    DebugBlockInfo blockInfos[TRANSLATION_UNIT_COUNT][MAX_DEBUG_TRANSLATION_UNIT_INFOS];
    DebugBlockStats blockStats[TRANSLATION_UNIT_COUNT][MAX_DEBUG_TRANSLATION_UNIT_INFOS];
};

struct DebugConfig
{
    b32 funcTimers;
    b32 frameTimers;
    b32 renderTiming;
    b32 entityColliders;
    b32 particleColliders;
    b32 regions;
    b32 camMove;
    b32 camZoom;
    b32 mouseInfo;
    b32 picking;
};

enum DebugMenuFunctionType
{
    DebugMenuFuncType_None,
    DebugMenuFuncType_b32,
};

struct DebugMenuItem
{
    DebugMenuItem *next;
    DebugMenuItem *child;
    b32 isOpen;
    
    char name[32];
    
    DebugMenuFunctionType funcType;
    void *use;
};

struct DebugSettings
{
    DebugConfig config;
    b32 configChanged;
    
    b32 movingTimerWindow;
    f32 timerWindowPosY;
    
    s32 pickedEntityIndex;
    
    DebugMenuItem menuSentinel;
};

struct DebugState
{
    b32 inFrame;
    
    MemoryRegion dataRegion;
    DebugTable *table;
    
    b32 openMenu;
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
