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

enum EntityType
{
    Entity_Null,
    
    Entity_Player,
    Entity_Shot_Player,
    Entity_Shot_UFO,
    Entity_Asteroids,
    Entity_UFO,
    
    Entity_Debug_Wall,
    
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
    b32 smallUFO;
    Timer timer;
    Timer shotTimer;
    s32 vMoveDir;
    f32 nextShotAngle;
};

introspect(category:"null") struct Entity
{
    EntityType type;
    b32 active;
    s32 index;
    
    Collider collider;
    
    f32 angle;
    f32 dA;
    
    v3f pos;
    v2f dP;
    v3f newPos;
    
    v2f dims;
    
    void *extraInfo;
};

#define AST_ENTITY_H
#endif //AST_ENTITY_H
