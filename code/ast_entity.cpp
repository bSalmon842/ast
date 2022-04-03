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

function v2f MoveEntity(Game_Input *input, Entity entity, v2f worldDims, b32 loop)
{
    v2f result = entity.pos.xy + (entity.dP * input->deltaTime);
    
    if (loop)
    {
        if (result.x + (entity.dims.w / 2.0f) < 0.0f)
        {
            result.x = worldDims.w + (entity.dims.w / 2.0f);
        }
        if (result.x - (entity.dims.w / 2.0f) > worldDims.w)
        {
            result.x = 0.0f - (entity.dims.w / 2.0f);
        }
        if (result.y + (entity.dims.h / 2.0f) < 0.0f)
        {
            result.y = worldDims.h + (entity.dims.h / 2.0f);
        }
        if (result.y - (entity.dims.h / 2.0f) > worldDims.h)
        {
            result.y = 0.0f - (entity.dims.h / 2.0f);
        }
    }
    
    return result;
}

#define NullEntity MakeEntity(Entity_Null, {}, 0, false, V3F(), V2F(), false)
function Entity MakeEntity(EntityType type, Collider collider, s32 index, b32 startActive, v3f initialPos, v2f dims, b32 loop, f32 initialAngle = 0.0f)
{
    Entity result = {};
    
    result.type = type;
    result.collider = collider;
    result.index = index;
    result.active = startActive;
    result.angle = initialAngle;
    result.dA = 0.0f;
    result.pos = initialPos;
    result.dP = V2F();
    result.dims = dims;
    
    result.loop = loop;
    result.extraInfo = 0;
    
    return result;
}

function Entity MakeEntity_Asteroid(Game_State *gameState, s32 index, b32 startActive, v3f initialPos, v2f dP, f32 dA, AsteroidSize size, u8 bitmapIndex, PlatformAPI platform)
{
    Entity result = {};
    
    result.type = Entity_Asteroids;
    result.index = index;
    result.active = startActive;
    result.angle = 0.0f;
    result.dA = dA;
    result.pos = initialPos;
    result.dP = dP;
    result.dims = GetAsteroidDims(size);
    result.collider = MakeCollider(gameState, ColliderType_Asteroid, result.pos.xy, result.dims);
    
    result.loop = true;
    result.extraInfo = platform.MemAlloc(sizeof(EntityInfo_Asteroid));
    EntityInfo_Asteroid *astInfo = (EntityInfo_Asteroid *)result.extraInfo;
    astInfo->size = size;
    astInfo->bitmapIndex = bitmapIndex;
    
    return result;
}

function Entity MakeEntity_Shot(Game_State *gameState, s32 index, v3f initialPos, v2f dP, f32 lifetime, PlatformAPI platform)
{
    Entity result = {};
    
    result.type = Entity_Shot;
    result.index = index;
    result.active = true;
    result.angle = 0.0f;
    result.pos = initialPos;
    result.dP = dP;
    result.dims = V2F(1.0f);
    result.collider = MakeCollider(gameState, ColliderType_Shot_Player, result.pos.xy, result.dims);
    
    result.loop = true;
    result.extraInfo = platform.MemAlloc(sizeof(EntityInfo_Shot));
    EntityInfo_Shot *shotInfo = (EntityInfo_Shot *)result.extraInfo;
    shotInfo->timer = InitialiseTimer(0.0f, lifetime);
    
    return result;
}

function Entity MakeEntity_UFO(Game_State *gameState, s32 index, v3f initialPos, v2f dims, s32 vMoveDir, PlatformAPI platform)
{
    Entity result = {};
    
    result.type = Entity_UFO;
    result.index = index;
    result.active = true;
    result.angle = TAU * 0.25f;
    result.pos = initialPos;
    result.dP = V2F(10.0f, 0.0f);
    result.dims = dims;
    result.collider = MakeCollider(gameState, ColliderType_UFO, result.pos.xy, result.dims);
    
    result.loop = true;
    result.extraInfo = platform.MemAlloc(sizeof(EntityInfo_UFO));
    EntityInfo_UFO *ufoInfo = (EntityInfo_UFO *)result.extraInfo;
    ufoInfo->timer = InitialiseTimer(0.0f, 10.0f);
    ufoInfo->vMoveDir = vMoveDir;
    
    return result;
}

function void ClearEntity(Entity *entity, PlatformAPI platform)
{
    platform.MemFree(entity->extraInfo);
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
