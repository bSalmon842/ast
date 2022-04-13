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

enum DebugEventType
{
    DebugEvent_Start,
    DebugEvent_Finish,
};

struct DebugEvent
{
    u64 clock;
    u16 thread;
    u16 core;
    u32 recordIndex;
    u8 recordArrayIndex;
    u8 type;
};

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
    DebugFrameInfo frames[DEBUG_DATUM_COUNT];
};

DebugRecord DEBUG_RECORD_ARRAY[];

#define MAX_DEBUG_EVENTS 65536
extern u64 globalDebugEventIndices; // Array Index | Event Index
extern DebugEvent globalDebugEventArray[2][MAX_DEBUG_EVENTS];

#define RecordDebugEvent(c, t) \
u64 eventIndices = AtomicIncrement(&globalDebugEventIndices); \
u32 eventIndex = (u32)(eventIndices & 0xFFFFFFFF); \
ASSERT(eventIndex < MAX_DEBUG_EVENTS); \
DebugEvent *event = &globalDebugEventArray[(u32)(eventIndices >> 32)][eventIndex]; \
event->thread = (u16)GetThreadID(); \
event->core = 0; \
event->recordIndex = (u16)(c); \
event->recordArrayIndex = DEBUG_RECORD_ARRAY_INDEX; \
event->type = t; \
event->clock = __rdtsc(); \

struct DebugTiming
{
    DebugRecord *record;
    u64 startCycleCount;
    u32 hitCount;
    u32 counter;
    
    DebugTiming(u32 counter, char *file, s32 line, char *func, u32 hits = 1)
    {
        this->record = DEBUG_RECORD_ARRAY + counter;
        this->record->fileName = file;
        this->record->functionName = func;
        this->record->lineNumber = line;
        this->hitCount = hits;
        this->counter = counter;
        
        this->startCycleCount = __rdtsc();
        
        //
        RecordDebugEvent(this->counter, DebugEvent_Start);
    }
    
    ~DebugTiming()
    {
        u64 delta = ((u64)this->hitCount << 32) | (__rdtsc() - this->startCycleCount);
        AtomicAdd(&this->record->counts, delta);
        
        RecordDebugEvent(this->counter, DebugEvent_Finish);
    }
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
