/*
Project: Asteroids
File: ast_entity.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_ENTITY_H

enum AsteroidSize
{
    AsteroidSize_Invalid,
    
    AsteroidSize_Small,
    AsteroidSize_Medium,
    AsteroidSize_Large,
    
    AsteroidSize_Count,
};

enum Entity_Type
{
    Entity_Null,
    
    Entity_Player,
    Entity_Shot,
    Entity_Asteroids,
    Entity_UFO,
    
    Entity_Count,
};

struct EntityInfo_Asteroid
{
    AsteroidSize size;
    u8 bitmapIndex;
};

struct EntityInfo_Shot
{
    Timer timer;
};

struct EntityInfo_UFO
{
    Timer timer;
    s32 vMoveDir;
    f32 nextShotAngle;
};

struct Entity
{
    Entity_Type type;
    b32 active;
    
    f32 angle;
    f32 dA;
    
    v2f pos;
    v2f dP;
    
    v2f dims;
    
    void *extraInfo;
};

#define AST_ENTITY_H
#endif //AST_ENTITY_H
