/*
Project: BS842_Tools
File: bs842_vector.h
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved
*/

#ifndef BS842_VECTOR_H

#define Make2DStruct(type, name, capital_name) \
struct name \
{ \
union \
{ \
type e[2]; \
struct { type x, y; }; \
struct { type w, h; }; \
struct { type u, v; }; \
}; \
}; \
\
inline name capital_name(type x, type y) \
{ \
name result = {};\
\
result.x = x; \
result.y = y; \
\
return result; \
} \
\
inline name capital_name(type value = 0) \
{ \
name result = {};\
\
result.x = value; \
result.y = value; \
\
return result; \
} \
\
inline name operator-(name a, name b) \
{ \
name result = {}; \
\
result.x = a.x - b.x; \
result.y = a.y - b.y; \
\
return result; \
} \
\
inline name operator-(name a, type b) \
{ \
name result = {}; \
\
result.x = a.x - b; \
result.y = a.y - b; \
\
return result; \
} \
\
inline name operator-(name a) \
{ \
name result = {}; \
\
result.x = -a.x; \
result.y = -a.y; \
\
return result; \
} \
\
inline name operator+(name a, name b) \
{ \
name result = {}; \
\
result.x = a.x + b.x; \
result.y = a.y + b.y; \
\
return result; \
} \
inline name operator+(name a, type b) \
{ \
name result = {}; \
\
result.x = a.x + b; \
result.y = a.y + b; \
\
return result; \
} \
\
inline name operator*(name a, name b) \
{ \
name result = {}; \
\
result.x = a.x * b.x; \
result.y = a.y * b.y; \
\
return result; \
} \
\
inline name operator*(name a, type b) \
{ \
name result = {}; \
\
result.x = a.x * b; \
result.y = a.y * b; \
\
return result; \
} \
\
inline name operator*(type a, name b) \
{ \
name result = {}; \
\
result.x = a * b.x; \
result.y = a * b.y; \
\
return result; \
} \
\
inline name operator/(name a, type b) \
{ \
name result = {}; \
\
result.x = a.x / b; \
result.y = a.y / b; \
\
return result; \
} \
\
inline name operator/(type b, name a) \
{ \
name result = {}; \
\
result.x = b / a.x; \
result.y = b / a.y; \
\
return result; \
} \
\
inline name operator/(name a, name b) \
{ \
name result = {}; \
\
result.x = a.x / b.x; \
result.y = a.y / b.y; \
\
return result; \
} \
\
inline name &operator+=(name &a, name b) \
{ \
a = a + b; \
\
return a; \
} \
\
inline name &operator+=(name &a, type b) \
{ \
a = a + b; \
\
return a; \
} \
\
inline name &operator-=(name &a, name b) \
{ \
a = a - b; \
\
return a; \
} \
\
inline name &operator-=(name &a, type b) \
{ \
a = a - b; \
\
return a; \
} \
\
inline name &operator*=(name &a, type b) \
{ \
a = a * b; \
\
return a; \
} \
\
inline name &operator*=(name &a, name b) \
{ \
a = a * b; \
\
return a; \
} \
\
inline bool operator==(name a, name b) \
{ \
return (a.x == b.x && a.y == b.y); \
} \
\
\
inline bool operator!=(name a, name b) \
{ \
return (a.x != b.x || a.y != b.y); \
} \
\
inline bool operator<(name a, name b) \
{ \
return (a.x < b.x && a.y < b.y); \
} \
\
inline bool operator>(name a, name b) \
{ \
return (a.x > b.x && a.y > b.y); \
} \
\
inline bool operator<=(name a, name b) \
{ \
return (a < b || a == b); \
} \
\
inline bool operator>=(name a, name b) \
{ \
return (a > b || a == b); \
} \
\
inline name &operator--(name &a) \
{ \
a.x = a.x - 1; \
a.y = a.y - 1; \
\
return a; \
} \
\
inline name &operator++(name &a) \
{ \
a.x = a.x + 1; \
a.y = a.y + 1; \
\
return a; \
} \
\

#define Make3DStruct(type, name, capital_name, lower_type) \
struct name \
{ \
union \
{ \
type e[3]; \
struct { type x, y, z; }; \
struct { type w, h, d; }; \
struct { type r, g, b; }; \
struct { lower_type xy; type z; }; \
}; \
}; \
\
inline name capital_name(type x, type y, type z) \
{ \
name result = {};\
\
result.x = x; \
result.y = y; \
result.z = z; \
\
return result; \
} \
\
inline name capital_name(lower_type xy, type z) \
{ \
name result = {};\
\
result.xy = xy; \
result.z = z; \
\
return result; \
} \
\
inline name capital_name(type value = 0) \
{ \
name result = {};\
\
result.x = value; \
result.y = value; \
result.z = value; \
\
return result; \
} \
\
inline name operator+(name a, name b) \
{ \
name result = {}; \
\
result.x = a.x + b.x; \
result.y = a.y + b.y; \
result.z = a.z + b.z; \
\
return result; \
} \
\
inline name operator-(name a, name b) \
{ \
name result = {}; \
\
result.x = a.x - b.x; \
result.y = a.y - b.y; \
result.z = a.z - b.z; \
\
return result; \
} \
\
inline name operator*(name a, type b) \
{ \
name result = {}; \
\
result.x = a.x * b; \
result.y = a.y * b; \
result.z = a.z * b; \
\
return result; \
} \
\
inline name operator*(type b, name a) \
{ \
name result = {}; \
\
result.x = b * a.x; \
result.y = b * a.y; \
result.z = b * a.z; \
\
return result; \
} \
\
inline name operator/(name a, type b) \
{ \
name result = {}; \
\
result.x = a.x / b; \
result.y = a.y / b; \
result.z = a.z / b; \
\
return result; \
} \
\
inline name operator/(type b, name a) \
{ \
name result = {}; \
\
result.x = b / a.x; \
result.y = b / a.y; \
result.z = b / a.z; \
\
return result; \
} \
\
inline name &operator+=(name &a, name b) \
{ \
a = a + b; \
\
return a; \
} \
\
inline name &operator*=(name &a, type b) \
{ \
a = a * b; \
\
return a; \
} \
\

#define Make4DStruct(type, name, capital_name, lower_type) \
struct name \
{ \
union \
{ \
type e[4]; \
struct { type x, y, z, w; }; \
struct { type r, g, b, a; }; \
struct { lower_type xyz; type w; }; \
struct { lower_type rgb; type a; }; \
}; \
}; \
\
inline name capital_name(type x, type y, type z, type w) \
{ \
name result = {};\
\
result.x = x; \
result.y = y; \
result.z = z; \
result.w = w; \
\
return result; \
} \
\
inline name capital_name(lower_type xyz, type w) \
{ \
name result = {};\
\
result.xyz = xyz; \
result.w = w; \
\
return result; \
} \
\
inline name capital_name(type value = 0) \
{ \
name result = {};\
\
result.x = value; \
result.y = value; \
result.z = value; \
result.w = value; \
\
return result; \
} \
\
inline name operator+(name a, name b) \
{ \
name result = {}; \
\
result.x = a.x + b.x; \
result.y = a.y + b.y; \
result.z = a.z + b.z; \
result.w = a.w + b.w; \
\
return result; \
} \
\
inline name operator*(name a, name b) \
{ \
name result = {}; \
\
result.x = a.x * b.x; \
result.y = a.y * b.y; \
result.z = a.z * b.z; \
result.w = a.w * b.w; \
\
return result; \
} \
\
inline name operator*(name a, type b) \
{ \
name result = {}; \
\
result.x = a.x * b; \
result.y = a.y * b; \
result.z = a.z * b; \
result.w = a.w * b; \
\
return result; \
} \
\
inline name operator*(type b, name a) \
{ \
name result = {}; \
\
result.x = a.x * b; \
result.y = a.y * b; \
result.z = a.z * b; \
result.w = a.w * b; \
\
return result; \
} \
\
inline bool operator==(name a, name b) \
{ \
return (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w); \
} \
\
inline bool operator!=(name a, name b) \
{ \
return (a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w); \
} \
\

#define MakeRectStruct(type, name) \
struct name \
{ \
type min; \
type max; \
type dims; \
type center; \
}; \
\
inline name Create##name##_MinMax(type min, type max) \
{ \
name result = {}; \
\
result.min = min; \
result.max = max; \
result.dims = max - min; \
result.center = min + (result.dims / 2); \
\
return result; \
} \
\
inline name Create##name##_MinDims(type min, type dims) \
{ \
name result = {}; \
\
result.min = min; \
result.max = min + dims; \
result.dims = dims; \
result.center = min + (dims / 2); \
\
return result; \
} \
\
inline name Create##name##_CenterHalfDims(type center, type halfDims) \
{ \
name result = {}; \
\
result.min = center - halfDims; \
result.max = center + halfDims; \
result.dims = halfDims * 2; \
result.center = center; \
\
return result; \
} \
\
inline name Create##name##_CenterDims(type center, type dims) \
{ \
name result = Create##name##_CenterHalfDims(center, dims / 2); \
\
return result; \
}

#ifdef BS842_MAKE_STRUCTS
#pragma warning(disable:4146)
Make2DStruct(f32, v2f, V2F);
Make2DStruct(u32, v2u, V2U);
Make2DStruct(s32, v2s, V2S);

Make3DStruct(f32, v3f, V3F, v2f);
Make3DStruct(u32, v3u, V3U, v2u);
Make3DStruct(s32, v3s, V3S, v2s);

Make4DStruct(f32, v4f, V4F, v3f);

MakeRectStruct(v2f, Rect2f);
MakeRectStruct(v2u, Rect2u);
MakeRectStruct(v2s, Rect2s);

MakeRectStruct(v3f, Rect3f);
#pragma warning(default:4146)
#endif //BS842_MAKE_STRUCTS

#define BS842_VECTOR_H
#endif //BS842_VECTOR_H