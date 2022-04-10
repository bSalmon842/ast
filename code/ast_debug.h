/*
Project: Asteroids
File: ast_debug.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_DEBUG_H

#error "Build currently broken during debug refactoring"

#define DEBUG_TIMED_SCOPE(...) DebugTiming _debugBlock_##__LINE__(__FILE__, __LINE__, __FUNCTION__)

struct DebugRecord
{
    u64 cycleCount;
    
    char *fileName;
    char *functionName;
    
    u32 lineNumber;
    u32 hitCount;
};

struct DebugTiming
{
    DebugRecord *record;
    
    DebugTiming(u32 counter, char *file, s32 line, char *func, u32 hits = 1)
    {
        this->record = DEBUG_RECORD_ARRAY + counter;
        this->record->fileName = file;
        this->record->functionName = func;
        this->record->lineNumber = line;
        this->record->hitCount += hits;
        this->record->cycleCount -= __rdtsc();
    }
    
    ~DebugTiming()
    {
        this->record->cycleCount += __rdtsc();
    }
};

#define AST_DEBUG_H
#endif //AST_DEBUG_H
