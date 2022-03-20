/*
Project: Asteroids
File: ast_memory.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_MEMORY_H

typedef struct
{
    u8 tempCount;
    u8 *base;
    usize size;
    usize used;
} MemoryRegion;

typedef struct
{
    MemoryRegion *memRegion;
    usize used;
} TempMemory;

struct ParallelMemory
{
    b32 inUse;
    MemoryRegion memRegion;
    TempMemory memFlush;
};

#define AST_MEMORY_H
#endif //AST_MEMORY_H
