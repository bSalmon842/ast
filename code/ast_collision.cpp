/*
Project: Asteroids
File: ast_collision.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

function Collider MakeCollider(Game_State *gameState, ColliderType type, v2f origin, v2f dims)
{
    Collider result = {};
    
    result.type = type;
    
    result.origin = origin;
    result.dims = dims;
    
    for (s32 i = 0; i < ColliderType_Count; ++i)
    {
        result.collisions[i].collisionRule = gameState->collisionRules[type][i];
        result.collisions[i].solid = ((result.collisions[i].collisionRule & Collision_Solid) != 0);
        result.collisions[i].trigger = ((result.collisions[i].collisionRule & Collision_Trigger) != 0);
        
        if (gameState->collisionActions[type][i].action)
        {
            result.collisions[i].onTrigger = gameState->collisionActions[type][i].action;
        }
    }
    
    return result;
}

inline NearbyEntities NearbyEntitiesStart(Game_State *gameState, v2f origin, f32 distance, PlatformAPI platform)
{
    NearbyEntities result = {};
    
    for (s32 entityIndex = 0; entityIndex < ARRAY_COUNT(gameState->entities); ++entityIndex)
    {
        Entity entity = gameState->entities[entityIndex];
        if (entity.type != Entity_Null && entity.active)
        {
            // TODO(bSalmon): Check against closest corner of rectangles or closest point of circle
            f32 distanceBetween = VectorDistance(entity.pos.xy, origin);
            if (distanceBetween <= distance)
            {
                ++result.count;
            }
        }
    }
    
    result.list = (Entity *)platform.MemAlloc(result.count * sizeof(Entity));
    result.triggers = (b32 *)platform.MemAlloc(result.count * sizeof(b32));
    
    s32 listIndex = 0;
    for (s32 entityIndex = 0; entityIndex < ARRAY_COUNT(gameState->entities); ++entityIndex)
    {
        Entity entity = gameState->entities[entityIndex];
        if (entity.type != Entity_Null && entity.active)
        {
            f32 distanceBetween = VectorDistance(entity.pos.xy, origin);
            if (distanceBetween <= distance)
            {
                result.list[listIndex++] = entity;
            }
        }
    }
    
    return result;
}

inline void NearbyEntitiesFinish(NearbyEntities *nearby, PlatformAPI platform)
{
    platform.MemFree(nearby->list);
    platform.MemFree(nearby->triggers);
    nearby->count = 0;
}

inline TestCollisionResult TestCollision(CollisionInfo collisionInfo, f32 deltaMain, f32 deltaAlt, f32 collidedStart, f32 relPosMain, f32 relPosAlt, f32 minAlt, f32 maxAlt, f32 *tMin)
{
    TestCollisionResult result = {};
    
    f32 epsilon = 0.0001f;
    if (deltaMain != 0.0f)
    {
        f32 tResult = (collidedStart - relPosMain) / deltaMain;
        f32 alt = relPosAlt + tResult * deltaAlt;
        if ((tResult >= 0.0f) && (*tMin > tResult))
        {
            if ((alt >= minAlt) && (alt <= maxAlt))
            {
                if (collisionInfo.solid)
                {
                    *tMin = MAX(0.0f, tResult - epsilon);
                    result.collided = true;
                }
                if (collisionInfo.trigger)
                {
                    result.trigger = true;
                }
            }
        }
    }
    
    return result;
}

#define COLLISION_ITERATION_COUNT 4
function void HandleCollisions(Game_State *gameState, Entity *entity, PlatformAPI platform)
{
    v2f entityDelta = entity->newPos.xy - entity->pos.xy;
    // TODO(bSalmon): For when entities are found via their sides/corners NearbyEntities nearby = NearbyEntitiesStart(gameState, entity->pos, VectorLength(entityDelta) * 2.0f, platform);
    NearbyEntities nearby = NearbyEntitiesStart(gameState, entity->pos.xy, 20.0f, platform);
    
    f32 tRemain = 1.0f;
    for (s32 i = 0; i < COLLISION_ITERATION_COUNT; ++i)
    {
        f32 tMin = 1.0f;
        v2f collisionNormal = V2F();
        for (u32 entityIndex = 0; entityIndex < nearby.count; ++entityIndex)
        {
            Entity *other = &nearby.list[entityIndex];
            CollisionInfo collisionInfo = entity->collider.collisions[other->collider.type];
            
            v2f entityMinkowskiDims = entity->collider.dims + other->collider.dims;
            v2f relOriginalPos = entity->pos.xy - other->pos.xy;
            v2f minCorner = -0.5f * entityMinkowskiDims;
            v2f maxCorner = 0.5f * entityMinkowskiDims;
            
            TestCollisionResult left = TestCollision(collisionInfo, entityDelta.x, entityDelta.y, minCorner.x, relOriginalPos.x, relOriginalPos.y, minCorner.y, maxCorner.y, &tMin);
            TestCollisionResult right = TestCollision(collisionInfo, entityDelta.x, entityDelta.y, maxCorner.x, relOriginalPos.x, relOriginalPos.y, minCorner.y, maxCorner.y, &tMin);
            TestCollisionResult down = TestCollision(collisionInfo, entityDelta.y, entityDelta.x, minCorner.y, relOriginalPos.y, relOriginalPos.x, minCorner.x, maxCorner.x, &tMin);
            TestCollisionResult up = TestCollision(collisionInfo, entityDelta.y, entityDelta.x, maxCorner.y, relOriginalPos.y, relOriginalPos.x, minCorner.x, maxCorner.x, &tMin);
            
            if (left.collided)
            {
                collisionNormal = V2F(-1, 0);
            }
            if (right.collided)
            {
                collisionNormal = V2F(1, 0);
            }
            if (down.collided)
            {
                collisionNormal = V2F(0, -1);
            }
            if (up.collided)
            {
                collisionNormal = V2F(0, 1);
            }
            
            if (left.trigger || right.trigger || down.trigger || up.trigger)
            {
                nearby.triggers[entityIndex] = true;
            }
        }
        
        entity->newPos.xy = entity->pos.xy + (tMin * entityDelta);
        entity->dP -= Dot(entity->dP, collisionNormal) * collisionNormal;
        entityDelta -= Dot(entityDelta, collisionNormal) * collisionNormal;
        tRemain -= tMin * tRemain;
    }
    
    for (u32 entityIndex = 0; entityIndex < nearby.count; ++entityIndex)
    {
        Entity *other = &gameState->entities[nearby.list[entityIndex].index];
        CollisionInfo collisionInfo = entity->collider.collisions[other->collider.type];
        
        if (nearby.triggers[entityIndex])
        {
            DefaultCollisionTriggerData data = {};
            data.entity = entity;
            data.other = other;
            data.gameState = gameState;
            data.platform = platform;
            if (collisionInfo.onTrigger)
            {
                collisionInfo.onTrigger(&data);
            }
        }
    }
    
    NearbyEntitiesFinish(&nearby, platform);
}
