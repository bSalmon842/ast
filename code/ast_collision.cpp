/*
Project: Asteroids
File: ast_collision.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

// TODO(bSalmon): Updated to proper AABB, but needs to be search in P GJK collision to prevent collision skipping through thin objects like it can do now
inline b32 BoundingBoxCollision(Collider a, Collider b)
{
    b32 result = false;
    
    v2f aMin = a.origin - (a.dims / 2.0f);
    v2f aMax = a.origin + (a.dims / 2.0f);
    v2f bMin = b.origin - (b.dims / 2.0f);
    v2f bMax = b.origin + (b.dims / 2.0f);
    
    result = ((aMin >= bMin && aMin <= bMax) ||
              (aMax >= bMin && aMax <= bMax));
    
    return result;
}

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

function void HandleCollisions(Game_State *gameState, Entity *entity, PlatformAPI platform)
{
    Collider entityCollider = entity->collider;
    for (s32 entityIndex = 1; entityIndex < ARRAY_COUNT(gameState->entities) && entity->index != 0; ++entityIndex)
    {
        if (entityIndex != entity->index)
        {
            Entity *other = &gameState->entities[entityIndex];
            Collider otherCollider = other->collider;
            
            if (BoundingBoxCollision(entityCollider, otherCollider) && entityCollider.collisions[otherCollider.type].collisionRule != 0)
            {
                CollisionInfo collisionInfo = entityCollider.collisions[otherCollider.type];
                if (collisionInfo.solid)
                {
                    v2f reflectDir = V2F(0.0f);
                    if (other->type == Entity_Debug_Wall)
                    {
                        v2f otherMin = otherCollider.origin - (otherCollider.dims / 2.0f);
                        v2f otherMax = otherCollider.origin + (otherCollider.dims / 2.0f);
                        
                        // TODO(bSalmon): Figure out how to better calc reflect dir
                        if (entity->pos.x >= otherMin.x && entity->pos.x <= otherMax.x)
                        {
                            reflectDir = V2F(0.0f, 1.0f);
                        }
                        else if (entity->pos.y >= otherMin.y && entity->pos.y <= otherMax.y)
                        {
                            reflectDir = V2F(1.0f, 0.0f);
                        }
                    }
                    
                    entity->dP -= Dot(entity->dP, reflectDir) * reflectDir;
                }
                if (collisionInfo.trigger)
                {
                    if (collisionInfo.onTrigger)
                    {
                        DefaultCollisionTriggerData triggerData = {};
                        triggerData.entity = entity;
                        triggerData.other = other;
                        triggerData.gameState = gameState;
                        triggerData.platform = platform;
                        collisionInfo.onTrigger(&triggerData);
                    }
                }
            }
        }
    }
}
