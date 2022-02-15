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
        CASE_SETRESULT(AsteroidSize_Large, V2F(15.0f));
        CASE_SETRESULT(AsteroidSize_Medium, V2F(10.0f));
        CASE_SETRESULT(AsteroidSize_Small, V2F(5.0f));
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

function AsteroidSize GetAsteroidSizeFromDims(v2f dims)
{
    AsteroidSize result = AsteroidSize_Large;
    
    if (dims == GetAsteroidDims(AsteroidSize_Large)) { result = AsteroidSize_Large; }
    else if (dims == GetAsteroidDims(AsteroidSize_Medium)) { result = AsteroidSize_Medium; }
    else if (dims == GetAsteroidDims(AsteroidSize_Small)) { result = AsteroidSize_Small; }
    
    return result;
}

function void MoveEntity(Game_Input *input, Entity *entity)
{
    entity->pos += entity->dP * input->deltaTime;
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
    result.pos = initialPos;
    result.dP = V2F();
    result.dims = dims;
    
    return result;
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
