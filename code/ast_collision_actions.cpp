/*
Project: Asteroids
File: ast_collision_actions.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

function COLLIDER_TRIGGER_ACTION(CollisionTrigger_PlayerShot_Asteroid)
{
    DefaultCollisionTriggerData *triggerData = (DefaultCollisionTriggerData *)data;
    Game_State *gameState = triggerData->gameState;
    PlatformAPI platform = triggerData->platform;
    Entity *entity = triggerData->entity;
    Entity *other = triggerData->other;
    
    v3f oldPos = other->pos;
    v2f oldDims = other->dims;
    AsteroidSize oldSize = ((EntityInfo_Asteroid *)other->extraInfo)->size;
    
    ClearEntity(other, platform);
    gameState->asteroidCount--;
    gameState->score += GetScoreForAsteroidSize(oldSize);
    
    EmitterProgressionInfo emitProgress = {EmitterLife_Wait, 0.5f, 2.0f, V4F(0.0f, 1.0f, 1.0f, 1.0f), V4F(0.0f, 1.0f, 1.0f, 1.0f), 0.0f, 10.0f};
    BitmapID emitBitmaps[4] = {BitmapID_Asteroid0, BitmapID_Asteroid1, BitmapID_Asteroid2, BitmapID_Asteroid3};
    AddEmitter(gameState, platform, oldPos, true, 4, emitProgress, {}, oldDims / 6.0f, false, emitBitmaps, 4);
    
    if (oldSize > AsteroidSize_Small)
    {
        s32 asteroidsToCreate = 2;
        for (s32 newAstIndex = 0; newAstIndex < ARRAY_COUNT(gameState->entities) && asteroidsToCreate > 0; ++newAstIndex)
        {
            FindFirstNullEntityResult newAst = FindFirstNullEntity(gameState->entities, ARRAY_COUNT(gameState->entities));
            if (newAst.entity)
            {
                AsteroidSize newSize = (AsteroidSize)(oldSize - 1);
                u8 newAstBitmapIndex = (u8)rnd_pcg_range(&gameState->pcg, 0, 3);
                f32 newAstRotationDir = (f32)rnd_pcg_range(&gameState->pcg, -1, 1);
                f32 astDA = newAstRotationDir * gameState->asteroidRotationRate;
                
                v2f astDP = {};
                while (astDP.x == 0.0f) { astDP.x = (f32)rnd_pcg_range(&gameState->pcg, -1, 1); }
                while (astDP.y == 0.0f) { astDP.y = (f32)rnd_pcg_range(&gameState->pcg, -1, 1); }
                astDP *= GetAsteroidBaseSpeed(newSize) * (V2F(rnd_pcg_nextf(&gameState->pcg), rnd_pcg_nextf(&gameState->pcg)) + 0.5f);
                
                *newAst.entity = MakeEntity_Asteroid(gameState, newAst.index, true, oldPos, astDP,
                                                     astDA, newSize, newAstBitmapIndex,
                                                     platform);
                --asteroidsToCreate;
            }
        }
    }
    
    ClearEntity(entity, platform);
}

function COLLIDER_TRIGGER_ACTION(CollisionTrigger_PlayerShot_UFO)
{
    DefaultCollisionTriggerData *triggerData = (DefaultCollisionTriggerData *)data;
    Game_State *gameState = triggerData->gameState;
    Entity *entity = triggerData->entity;
    Entity *other = triggerData->other;
    
    ClearEntity(entity, triggerData->platform);
    ClearEntity(other, triggerData->platform);
    gameState->score += 200;
    ResetTimer(&gameState->ufoSpawnTimer);
    gameState->ufoSpawned = false;
}
