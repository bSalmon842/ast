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
#ifndef InterlockedCompareExchange
#define InterlockedCompareExchange(a, b, c) _InterlockedCompareExchange(a, b, c)
#endif

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
