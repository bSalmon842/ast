/*
Project: Asteroids
File: ast_entity.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#define CASE_SETRESULT(caseValue, resultValue) case caseValue: {result = resultValue;} break
function f32 GetAsteroidBaseSpeed(AsteroidSize size)
{
    f32 result = {};
    switch (size)
    {
        CASE_SETRESULT(AsteroidSize_Large, 5.0f);
        CASE_SETRESULT(AsteroidSize_Medium, 7.5f);
        CASE_SETRESULT(AsteroidSize_Small, 10.0f);
        INVALID_DEFAULT;
    }
    return result;
}

function v2f GetAsteroidDims(AsteroidSize size)
{
    v2f result = {};
    switch (size)
    {
        CASE_SETRESULT(AsteroidSize_Large, V2F(10.0f));
        CASE_SETRESULT(AsteroidSize_Medium, V2F(5.0f));
        CASE_SETRESULT(AsteroidSize_Small, V2F(2.5f));
        INVALID_DEFAULT;
    }
    return result;
}

function u32 GetScoreForAsteroidSize(AsteroidSize size)
{
    u32 result = 0;
    switch (size)
    {
        CASE_SETRESULT(AsteroidSize_Large, 20);
        CASE_SETRESULT(AsteroidSize_Medium, 50);
        CASE_SETRESULT(AsteroidSize_Small, 100);
        INVALID_DEFAULT;
    }
    return result;
}

function f32 RotateEntity(Game_Input *input, Entity entity)
{
    f32 result = entity.angle + entity.dA * input->deltaTime;
    return result;
}

function v2f MoveEntity(Game_Input *input, Entity entity, World world)
{
    v2f result = entity.pos.xy + (entity.dP * input->deltaTime);
    CheckWorldPosition(world, entity, &result);
    
    return result;
}

function Entity *MakeEntity(Entity *entityList, EntityType type, b32 startActive, s32 index, Collider collider, f32 initialAngle, f32 dAngle, v3f initialPos, v2f dPos, v2f dims)
{
    Entity *entity = &entityList[index];
    
    UniversalID *universalID = RegisterUniversalID("Entity", IDPtrType_Entity, entity);
    stbsp_sprintf(entity->id, "%s", universalID->string);
    
    entity->type = type;
    entity->active = startActive;
    entity->index = index;
    
    entity->collider = collider;
    
    entity->angle = initialAngle;
    entity->dA = dAngle;
    
    entity->pos = initialPos;
    entity->dP = dPos;
    
    entity->dims = dims;
    
    entity->extraInfo = 0;
    
    return entity;
}

function void MakeEntity_Asteroid(Game_State *gameState, s32 index, b32 startActive, v3f initialPos, v2f dP, f32 dA, AsteroidSize size, u8 bitmapIndex, PlatformAPI platform)
{
    v2f dims = GetAsteroidDims(size);
    Entity *entity = MakeEntity(gameState->entities, Entity_Asteroids, startActive, index,
                                MakeCollider(gameState, ColliderType_Asteroid, initialPos, dims),
                                0.0f, dA,
                                initialPos, dP,
                                dims);
    
    entity->extraInfo = platform.MemAlloc(sizeof(EntityInfo_Asteroid));
    EntityInfo_Asteroid *astInfo = (EntityInfo_Asteroid *)entity->extraInfo;
    astInfo->size = size;
    astInfo->bitmapIndex = bitmapIndex;
}

// TODO(bSalmon): Compress the 2 shot functions, similar enough
function void MakeEntity_Shot_UFO(Game_State *gameState, s32 index, v3f initialPos, v2f dP, f32 lifetime, PlatformAPI platform)
{
    v2f dims = V2F(1.0f);
    Entity *entity = MakeEntity(gameState->entities, Entity_Shot_UFO, true, index,
                                MakeCollider(gameState, ColliderType_Shot_UFO, initialPos, dims),
                                0.0f, 0.0f,
                                initialPos, dP,
                                dims);
    
    entity->extraInfo = platform.MemAlloc(sizeof(EntityInfo_Shot));
    EntityInfo_Shot *shotInfo = (EntityInfo_Shot *)entity->extraInfo;
    shotInfo->timer = InitialiseTimer(0.0f, lifetime);
}

function void MakeEntity_Shot_Player(Game_State *gameState, s32 index, v3f initialPos, v2f dP, f32 lifetime, PlatformAPI platform)
{
    v2f dims = V2F(1.0f);
    Entity *entity = MakeEntity(gameState->entities, Entity_Shot_Player, true, index,
                                MakeCollider(gameState, ColliderType_Shot_Player, initialPos, dims),
                                0.0f, 0.0f,
                                initialPos, dP,
                                dims);
    
    entity->extraInfo = platform.MemAlloc(sizeof(EntityInfo_Shot));
    EntityInfo_Shot *shotInfo = (EntityInfo_Shot *)entity->extraInfo;
    shotInfo->timer = InitialiseTimer(0.0f, lifetime);
}

function void MakeEntity_UFO(Game_State *gameState, s32 index, v3f initialPos, v2f dims, b32 smallUFO, s32 vMoveDir, PlatformAPI platform)
{
    Entity *entity = MakeEntity(gameState->entities, Entity_UFO, true, index,
                                MakeCollider(gameState, ColliderType_UFO, initialPos, dims),
                                TAU * 0.25f, 0.0f,
                                initialPos, V2F(10.0f, 0.0f),
                                dims);
    
    entity->extraInfo = platform.MemAlloc(sizeof(EntityInfo_UFO));
    EntityInfo_UFO *ufoInfo = (EntityInfo_UFO *)entity->extraInfo;
    ufoInfo->smallUFO = smallUFO;
    ufoInfo->timer = InitialiseTimer(0.0f, 10.0f);
    ufoInfo->shotTimer = InitialiseTimer(0.0f, 1.5f);
    ufoInfo->vMoveDir = vMoveDir;
}

function void ClearEntity(Entity *entity, PlatformAPI platform)
{
    platform.MemFree(entity->extraInfo);
    DeregisterUniversalID(entity->id);
    *entity = {};
}

struct FindFirstNullEntityResult
{
    Entity *entity;
    s32 index;
};

function FindFirstNullEntityResult FindFirstNullEntity(Entity *entityList, s32 entityListSize)
{
    FindFirstNullEntityResult result = {};
    for (s32 entityIndex = 1; entityIndex < entityListSize; ++entityIndex)
    {
        if (entityList[entityIndex].type == Entity_Null)
        {
            result.entity = &entityList[entityIndex];
            result.index = entityIndex;
            break;
        }
    }
    
    return result;
}
