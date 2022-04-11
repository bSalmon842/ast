/*
Project: Asteroids
File: ast_debug.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_DEBUG_H

#define DEBUG_TIMED_SCOPE__(lineNo, ...) DebugTiming _debugBlock_##lineNo(__COUNTER__, __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__)
#define DEBUG_TIMED_SCOPE_(lineNo, ...) DEBUG_TIMED_SCOPE__(lineNo, ## __VA_ARGS__)
#define DEBUG_TIMED_SCOPE(...) DEBUG_TIMED_SCOPE_(__LINE__, ## __VA_ARGS__)

#define DEBUG_TEXT_SCALE 0.75f
#define DEBUG_LAYER 200
#define DEBUG_DATUM_COUNT 240

global b32 debug_info = false;
global b32 debug_colliders = false;
global b32 debug_cam = false;
global b32 debug_regions = false;
global b32 debug_camMove = false;

struct DebugRecord
{
    char *fileName;
    char *functionName;
    u32 lineNumber;
    
    u64 counts;
};

DebugRecord DEBUG_RECORD_ARRAY[];

struct DebugCounterDatum
{
    u32 hits;
    u32 cycles;
};

struct DebugCounter
{
    char *fileName;
    char *functionName;
    u32 lineNumber;
    
    DebugCounterDatum data[DEBUG_DATUM_COUNT];
};

struct DebugState
{
    u32 datumIndex;
    u32 counterCount;
    DebugCounter counters[512];
};

struct DebugTiming
{
    DebugRecord *record;
    u64 startCycleCount;
    u32 hitCount;
    
    DebugTiming(u32 counter, char *file, s32 line, char *func, u32 hits = 1)
    {
        this->record = DEBUG_RECORD_ARRAY + counter;
        this->record->fileName = file;
        this->record->functionName = func;
        this->record->lineNumber = line;
        this->hitCount = hits;
        
        this->startCycleCount = __rdtsc();
    }
    
    ~DebugTiming()
    {
        u64 delta = ((u64)this->hitCount << 32) | (__rdtsc() - this->startCycleCount);
        AtomicAdd(&this->record->counts, delta);
    }
};

struct DebugStat
{
    f64 min;
    f64 max;
    f64 ave;
    u32 count;
};

inline void StartDebugStat(DebugStat *stat)
{
    stat->min = FLT_MAX;
    stat->max = -FLT_MAX;
    stat->ave = 0.0;
    stat->count = 0;
}

inline void CalcDebugStat(DebugStat *stat, f64 value)
{
    ++stat->count;
    
    if (stat->min > value)
    {
        stat->min = value;
    }
    
    if (stat->max < value)
    {
        stat->max = value;
    }
    
    stat->ave += value;
}

inline void FinishDebugStat(DebugStat *stat)
{
    if (stat->count)
    {
        stat->ave /= (f64)stat->count;
    }
    else
    {
        stat->min = 0.0;
        stat->max = 0.0;
    }
}

#define AST_DEBUG_H
#endif //AST_DEBUG_H
