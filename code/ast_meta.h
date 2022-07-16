/*
Project: Asteroids
File: ast_meta.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_META_H

enum MemberType
{
    MemberType_void,
    MemberType_f32,
    MemberType_s32,
    MemberType_b32,
    MemberType_v2f,
    MemberType_v3f,
    MemberType_EntityType,
    MemberType_Collider,
    MemberType_ColliderType,
    MemberType_Array_char,
    MemberType_Array_CollisionInfo,
};

struct IntrospectMemberDef
{
    MemberType type;
    char *name;
    u32 offset;
    u32 elementCount; // NOTE(bSalmon): For arrays
};

enum EnumType
{
    EnumType_u16,
    EnumType_u32,
};

struct IntrospectEnumDef
{
    EnumType type;
    char *name;
    u64 value;
};

#define AST_META_H
#endif //AST_META_H
