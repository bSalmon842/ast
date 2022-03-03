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

inline void CopyMem(void *dest, void *src, usize size)
{
    u8 *destPtr = (u8 *)dest;
    u8 *srcPtr = (u8 *)src;
    for (usize i = 0; i < size; ++i)
    {
        *destPtr++ = *srcPtr++;
    }
}

#define AST_MEMORY_H
#endif //AST_MEMORY_H
