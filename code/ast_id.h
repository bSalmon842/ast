/*
Project: Asteroids
File: ast_id.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_ID_H

enum IDPtrType : u8
{
    IDPtrType_Null,
    IDPtrType_Entity,
};

struct UniversalID
{
    char string[32];
    u16 number;
    IDPtrType type;
    void *ptr;
};

#define MAX_ID_COUNT U16_MAX
struct UniversalIDList
{
    UniversalID ids[MAX_ID_COUNT];
    u32 firstAvailIndex;
};

#define AST_ID_H
#endif //AST_ID_H
