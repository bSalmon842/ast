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

function void MoveEntity(Game_Input *input, Entity *entity)
{
    entity->pos += entity->dP * input->deltaTime;
    entity->angle += entity->dA * input->deltaTime;
}

function void MoveEntityLoop(Game_Input *input, Entity *entity, v2f worldDims)
{
    MoveEntity(input, entity);
    
    if (entity->pos.x + (entity->dims.w / 2.0f) < 0.0f)
    {
        entity->pos.x = worldDims.w + (entity->dims.w / 2.0f);
    }
    if (entity->pos.x - (entity->dims.w / 2.0f) > worldDims.w)
    {
        entity->pos.x = 0.0f - (entity->dims.w / 2.0f);
    }
    if (entity->pos.y + (entity->dims.h / 2.0f) < 0.0f)
    {
        entity->pos.y = worldDims.h + (entity->dims.h / 2.0f);
    }
    if (entity->pos.y - (entity->dims.h / 2.0f) > worldDims.h)
    {
        entity->pos.y = 0.0f - (entity->dims.h / 2.0f);
    }
}

#define NullEntity MakeEntity(Entity_Null, false, V2F(), V2F())
function Entity MakeEntity(Entity_Type type, b32 startActive, v2f initialPos, v2f dims, f32 initialAngle = 0.0f)
{
    Entity result = {};
    
    result.type = type;
    result.active = startActive;
    result.angle = initialAngle;
    result.dA = 0.0f;
    result.pos = initialPos;
    result.dP = V2F();
    result.dims = dims;
    
    result.extraInfo = 0;
    
    return result;
}

function Entity MakeEntity_Asteroid(b32 startActive, v2f initialPos, v2f dP, f32 dA, AsteroidSize size, u8 bitmapIndex, PlatformAPI platform)
{
    Entity result = {};
    
    result.type = Entity_Asteroids;
    result.active = startActive;
    result.angle = 0.0f;
    result.dA = dA;
    result.pos = initialPos;
    result.dP = dP;
    result.dims = GetAsteroidDims(size);
    
    result.extraInfo = platform.MemAlloc(sizeof(EntityInfo_Asteroid));
    EntityInfo_Asteroid *astInfo = (EntityInfo_Asteroid *)result.extraInfo;
    astInfo->size = size;
    astInfo->bitmapIndex = bitmapIndex;
    
    return result;
}

function Entity MakeEntity_Shot(v2f initialPos, v2f dP, f32 lifetime, PlatformAPI platform)
{
    Entity result = {};
    
    result.type = Entity_Shot;
    result.active = true;
    result.angle = 0.0f;
    result.pos = initialPos;
    result.dP = dP;
    result.dims = V2F(1.0f);
    
    result.extraInfo = platform.MemAlloc(sizeof(EntityInfo_Shot));
    EntityInfo_Shot *shotInfo = (EntityInfo_Shot *)result.extraInfo;
    shotInfo->timer = InitialiseTimer(0.0f, lifetime);
    
    return result;
}

function Entity MakeEntity_UFO(v2f initialPos, v2f dims, s32 vMoveDir, PlatformAPI platform)
{
    Entity result = {};
    
    result.type = Entity_UFO;
    result.active = true;
    result.angle = TAU * 0.25f;
    result.pos = initialPos;
    result.dP = V2F(10.0f, 0.0f);
    result.dims = dims;
    
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

function Entity *FindFirstNullEntity(Entity *entityList, s32 entityListSize)
{
    Entity *result = 0;
    for (s32 entityIndex = 1; entityIndex < entityListSize; ++entityIndex)
    {
        if (entityList[entityIndex].type == Entity_Null)
        {
            result = &entityList[entityIndex];
            break;
        }
    }
    
    return result;
}
