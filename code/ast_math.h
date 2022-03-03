/*
Project: Asteroids
File: ast_math.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_MATH_H

#include <math.h>

inline f32 Sin(f32 theta)
{
    f32 result = sinf(theta);
    return result;
}

inline f32 Cos(f32 theta)
{
    f32 result = cosf(theta);
    return result;
}

inline f32 Sq(f32 value)
{
    f32 result = value * value;
    return result;
}

inline f32 SqRt(f32 value)
{
    f32 result = sqrtf(value);
    return result;
}

inline f32 Lerp(f32 a, f32 b, f32 t)
{
    f32 result = (1.0f - t) * a + (b * t);
    return result;
}

inline u8 Lerp(u8 a, u8 b, f32 t)
{
    u8 result = (u8)((1.0f - t) * (f32)a + ((f32)b * t));
    return result;
}

inline v4f Lerp(v4f a, v4f b, f32 t)
{
    v4f result = (1.0f - t) * a + (b * t);
    return result;
}

inline f32 Dot(v2f a, v2f b)
{
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}

inline v4f Hadamard(v4f a, v4f b)
{
    v4f result = V4F(a.x * b.x,
                     a.y * b.y,
                     a.z * b.z,
                     a.w * b.w);
    return result;
}

inline f32 LengthSq(v2f vec)
{
    f32 result = Dot(vec, vec);
    return result;
}

inline f32 SignOf(f32 value)
{
    f32 result = (value > 0.0f) ? 1.0f : (value == 0.0f) ? 0.0f : -1.0f;
    return result;
}

inline f32 VectorLength(v2f vec)
{
    f32 result = SqRt(Sq(vec.x) + Sq(vec.y));
    return result;
}

inline s32 RoundF32ToS32(f32 value)
{
    s32 result = (s32)(value + 0.5f);
    return result;
}

inline u32 RoundF32ToU32(f32 value)
{
    u32 result = (u32)(value + 0.5f);
    return result;
}

inline v2s RoundV2FToV2S(v2f vec)
{
    v2s result = {RoundF32ToS32(vec.x), RoundF32ToS32(vec.y)};
    return result;
}

inline s32 FloorF32ToS32(f32 value)
{
    s32 result = (s32)value;
    return result;
}

inline s32 CeilF32ToS32(f32 value)
{
    s32 result = (s32)(value + 1.0f);
    return result;
}

inline s32 ClampS32(s32 value, s32 min, s32 max)
{
    s32 result = (value < min) ? min : ((value > max) ? max : value);
    return result;
}

inline f32 ClampF32(f32 value, f32 min, f32 max)
{
    f32 result = (value < min) ? min : ((value > max) ? max : value);
    return result;
}

inline v2s ClampV2S(v2s vec, v2s min, v2s max)
{
    v2s result = {ClampS32(vec.x, min.x, max.x), ClampS32(vec.y, min.y, max.y)};
    return result;
}

inline v2f ClampV2F(v2f vec, v2f min, v2f max)
{
    v2f result = {ClampF32(vec.x, min.x, max.x), ClampF32(vec.y, min.y, max.y)};
    return result;
}

inline v2f Perp(v2f a)
{
    v2f result = V2F(-a.y, a.x);
    return result;
}

inline v2s MinElementsV2S(v2s a, v2s b)
{
    v2s result = V2S();
    result.x = a.x < b.x ? a.x : b.x;
    result.y = a.y < b.y ? a.y : b.y;
    return result;
}

inline v2s MaxElementsV2S(v2s a, v2s b)
{
    v2s result = V2S();
    result.x = a.x > b.x ? a.x : b.x;
    result.y = a.y > b.y ? a.y : b.y;
    return result;
}

inline f32 SafeRatio(f32 num, f32 div, f32 n)
{
    f32 result = (div != 0.0f) ? num / div : n;
    return result;
}

//~ NOTE(bSalmon): VECTOR CONVERSIONS
inline v2f ToV2F(v2s vec)
{
    return V2F((f32)vec.x, (f32)vec.y);
}

inline v2s ToV2S(v2f vec)
{
    return V2S((s32)vec.x, (s32)vec.y);
}

#define AST_MATH_H
#endif //AST_MATH_H
