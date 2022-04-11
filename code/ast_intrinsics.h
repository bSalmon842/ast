/*
Project: Asteroids
File: ast_intrinsics.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_INTRINSICS_H

#ifdef _MSC_VER
#include <intrin.h>

#define WriteBarrier _WriteBarrier()
#define ReadBarrier _ReadBarrier()
#define ReadWriteBarrier _ReadWriteBarrier()

inline u32 AtomicCompareExchange(u32 volatile *dest, u32 ex, u32 comp)
{
    u32 result = _InterlockedCompareExchange((long *)dest, ex, comp);
    return result;
}

inline u64 AtomicExchange(u64 volatile *dest, u64 value)
{
    u64 result = _InterlockedExchange64((s64 *)dest, value);
    return result;
}

inline u32 AtomicIncrement(u32 volatile *dest)
{
    u32 result = _InterlockedIncrement((long *)dest);
    return result;
}

inline u64 AtomicAdd(u64 volatile *dest, u64 value)
{
    u64 result = _InterlockedExchangeAdd64((s64 *)dest, value);
    return result;
}

inline u32 GetThreadID()
{
    u32 result = *(u32 *)((u8 *)__readgsqword(0x30) + 0x48);
    return result;
}
#endif // MSC_VER

#define mElem(reg, index) ((f32 *)&(reg))[index]
#define miElem(reg, index) ((s32 *)&(reg))[index]

#define AST_INTRINSICS_H
#endif // AST_INTRINSICS_H
