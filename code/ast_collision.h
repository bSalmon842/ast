/*
Project: Asteroids
File: ast_collision.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_COLLISION_H

enum CollisionRule : u32
{
    Collision_Solid = 0x1,
    Collision_Trigger = 0x2,
};

enum ColliderType
{
    ColliderType_None,
    ColliderType_Player,
    ColliderType_Asteroid,
    ColliderType_UFO,
    ColliderType_Shot_Player,
    ColliderType_Shot_UFO,
    ColliderType_Debug_Wall,
    ColliderType_Debug_Particle,
    
    ColliderType_Count,
};

#define COLLIDER_TRIGGER_ACTION(funcName) void funcName(void *data)
typedef COLLIDER_TRIGGER_ACTION(colliderAction);

struct CollisionInfo
{
    u32 collisionRule;
    b32 solid;
    b32 trigger;
    
    colliderAction *onTrigger;
};

introspect(category:"null") struct Collider
{
    ColliderType type;
    
    v3f origin;
    v2f dims;
    
    CollisionInfo collisions[ColliderType_Count];
};

struct Entity;
struct Game_State;
struct DefaultCollisionTriggerData
{
    Entity *entity;
    Entity *other;
    
    Game_State *gameState;
    PlatformAPI platform;
};

struct CollisionTriggerAction
{
    ColliderType targetType;
    colliderAction *action;
};

struct TestCollisionResult
{
    b32 collided;
    b32 trigger;
};

struct NearbyEntities
{
    Entity *list;
    b32 *triggers;
    u32 count;
};

#define AST_COLLISION_H
#endif //AST_COLLISION_H
