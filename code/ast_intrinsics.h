/*
Project: Asteroids
File: ast_intrinsics.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifdef _MSC_VER
#include <intrin.h>
#endif // MSC_VER

#ifndef AST_INTRINSICS_H

#define mElem(reg, index) ((f32 *)&(reg))[index]
#define miElem(reg, index) ((s32 *)&(reg))[index]

#define AST_INTRINSICS_H
#endif // AST_INTRINSICS_H
