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
#else
#define ASSERT(check)
#define INVALID_CODE_PATH
#endif

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define SWAP(a, b) {decltype(a) temp = a; a = b; b = temp;}

#define KILOBYTE(value) ((value) * 1024LL)
#define MEGABYTE(value) (KILOBYTE(value) * 1024LL)
#define GIGABYTE(value) (MEGABYTE(value) * 1024LL)
#define TERABYTE(value) (GIGABYTE(value) * 1024LL)

#define AST_UTILITY_H
#endif //AST_UTILITY_H
