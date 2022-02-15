/*
Project: Asteroids
File: ast_utility.h
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_UTILITY_H

#define function static
#define persist static
#define global static

typedef __int8 s8;
typedef __int16 s16;
typedef __int32 s32;
typedef __int64 s64;

typedef unsigned __int8 u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef float f32;
typedef double f64;

typedef s32 b32;
typedef bool b8;

typedef s64 ssize;
typedef u64 usize;

#if AST_SLOW
#define ASSERT(check) if(!(check)) {*(s32 *)0 = 0;}
#define INVALID_CODE_PATH ASSERT(false)
#define INVALID_DEFAULT default: { INVALID_CODE_PATH; } break;
#else
#define ASSERT(check)
#define INVALID_CODE_PATH
#define INVALID_DEFAULT
#endif

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define SWAP(a, b) {decltype(a) temp = a; a = b; b = temp;}

#define Align8(value) ((value + 7) & ~7)

#define KILOBYTE(value) ((value) * 1024LL)
#define MEGABYTE(value) (KILOBYTE(value) * 1024LL)
#define GIGABYTE(value) (MEGABYTE(value) * 1024LL)
#define TERABYTE(value) (GIGABYTE(value) * 1024LL)

#define TAU 6.28318530717958647692f

#define BITMAP_BYTES_PER_PIXEL 4

inline s32 StringLength(char *string)
{
    s32 result = 0;
    char *c = string;
    while (*c++)
    {
        ++result;
    }
    
    return result;
}

inline void ConcatStrings(char *dest, char *a, usize aSize, char *b, usize bSize)
{
    for (s32 i = 0; i < aSize; ++i)
    {
        *dest++ = *a++;
    }
    
    for (s32 i = 0; i < bSize; ++i)
    {
        *dest++ = *b++;
    }
    
    *dest++ = 0;
}

inline u32 SafeTruncateU64(u64 value)
{
    ASSERT(value <= 0xFFFFFFFF);
    u32 result = (u32)value;
    return result;
}

#define AST_UTILITY_H
#endif //AST_UTILITY_H
