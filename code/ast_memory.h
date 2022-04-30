/*
Project: Asteroids
File: ast_memory.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_MEMORY_H

struct MemoryRegion
{
    u8 tempCount;
    u8 *base;
    usize size;
    usize used;
};

struct TempMemory
{
    MemoryRegion *memRegion;
    usize used;
};

struct ParallelMemory
{
    b32 inUse;
    MemoryRegion memRegion;
    TempMemory memFlush;
};

//////////////////////// CPP IMPL ////////////////////////////

inline void InitMemRegion(MemoryRegion *memRegion, usize size, void *base)
{
    memRegion->base = (u8 *)base;
    memRegion->size = size;
    memRegion->used = 0;
    memRegion->tempCount = 0;
}

inline usize GetAlignmentOffset(MemoryRegion *memRegion, usize alignment)
{
    usize result = 0;
    usize resultPtr = (usize)memRegion->base + memRegion->used;
    usize alignmentMask = alignment - 1;
    
    if (resultPtr & alignmentMask)
    {
        result = alignment - (resultPtr & alignmentMask);
    }
    
    return result;
}

#include <string.h>

#define Align16(val) (((val) + 15) & ~15)

#define PushStruct(region, type, ...) (type *)PushSize(region, sizeof(type), ## __VA_ARGS__)
#define PushArray(region, count, type, ...) (type *)PushSize(region, (count) * sizeof(type), ## __VA_ARGS__)
#define PushSize(region, size, ...) PushSize_(region, size, ## __VA_ARGS__)
inline void *PushSize_(MemoryRegion *memRegion, usize size, usize alignment = 4)
{
    usize alignmentOffset = GetAlignmentOffset(memRegion, alignment);
    size += alignmentOffset;
    
    ASSERT((memRegion->used + size) <= memRegion->size);
    
    void *result = memRegion->base + memRegion->used + alignmentOffset;
    memRegion->used += size;
    
#if 0
    // TODO(bSalmon): Manual memset
#else
    memset(result, 0, size);
#endif
    
    return result;
}

inline usize GetMemoryRegionSizeRemaining(MemoryRegion *memRegion, usize alignment = 4)
{
    usize result = memRegion->size - (memRegion->used + GetAlignmentOffset(memRegion, alignment));
    return result;
}

inline TempMemory StartTempMemory(MemoryRegion *memRegion)
{
    TempMemory result = {};
    
    result.memRegion = memRegion;
    result.used = memRegion->used;
    
    ++memRegion->tempCount;
    
    return result;
}

inline void FinishTempMemory(TempMemory tempMem)
{
    MemoryRegion *memRegion = tempMem.memRegion;
    
    ASSERT(memRegion->used >= tempMem.used);
    memRegion->used = tempMem.used;
    
    ASSERT(memRegion->tempCount > 0);
    --memRegion->tempCount;
}

inline MemoryRegion CreateMemorySubRegion(MemoryRegion *baseRegion, usize size, usize alignment = 16)
{
    MemoryRegion result = {};
    
    ASSERT(GetMemoryRegionSizeRemaining(baseRegion) >= size);
    result.size = size;
    result.base = (u8 *)PushSize(baseRegion, size, alignment);
    
    return result;
}

#define AST_MEMORY_H
#endif //AST_MEMORY_H
