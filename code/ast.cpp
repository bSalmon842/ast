/*
Project: Asteroids
File: ast.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

// TODO(bSalmon): Engine:
// TODO(bSalmon): Concept of layers within the same Z coords for sorting and maybe collision
// TODO(bSalmon): Collision based on Entity Z and maybe layers
// TODO(bSalmon): Collision double dispatch method review
// TODO(bSalmon): More concrete concept of a World vs a Level (should they be different, eg: World is persistent while a Level is the entities to put into the World, or should there just be Worlds?)
// TODO(bSalmon): Sim Regions
// TODO(bSalmon): Particles
// TODO(bSalmon): Animation (Sprite-sheets?)
// TODO(bSalmon): Better Debug outputs
// TODO(bSalmon): Audio Mixing
// TODO(bSalmon): Menues
// TODO(bSalmon): Loading Screens
// TODO(bSalmon): Proper window sizing
// TODO(bSalmon): Multiple resolutions

// TODO(bSalmon): Game:
// TODO(bSalmon): Death and lives
// TODO(bSalmon): UFO Shots

// TODO(bSalmon): Bitmap list
// TODO(bSalmon): Ship (w/ rocket trail)
// TODO(bSalmon): UFO (S: 1000)

// TODO(bSalmon): Audio list
// TODO(bSalmon): Ambient noise once every x seconds which speeds up as wave progresses
// TODO(bSalmon): Shot
// TODO(bSalmon): Propulsion
// TODO(bSalmon): Asteroid destroyed
// TODO(bSalmon): Ambient UFO active

#include "ast.h"
#include "ast_timer.cpp"
#include "ast_intrinsics.h"
#include "ast_math.h"
#include "ast_memory.cpp"
#include "ast_asset.cpp"
#include "ast_render.cpp"
#include "ast_collision.cpp"
#include "ast_entity.cpp"
#include "ast_collision_actions.cpp"
#include "ast_camera.cpp"

inline BitmapID GetAsteroidBitmapID(u32 index)
{
    BitmapID result = (BitmapID)(BitmapID_Asteroid0 + index);
    return result;
}

function void Debug_CycleCounters(Game_Memory *memory, Game_RenderCommands *commands, Camera camera, PlatformAPI platform)
{
    char *nameTable[] = 
    {
        "GameUpdateRender",
        "UpdateEntities",
    };
    
    f32 scale = 0.25f;
    char *font = "Hyperspace";
    
    RenderString title = MakeRenderString(platform, "Debug Perf Metrics");
    PushText(commands, V2F(1.0f), title, font, camera, V3F(camera.pos.xy - V2F(100.0f), 0.0f), scale, V4F(1.0f));
    
    v3f debugLineOffset = V3F(camera.pos.xy - V2F(100.0f), 0.0f);
    for (s32 counterIndex = 0; counterIndex < ARRAY_COUNT(memory->counters); ++counterIndex)
    {
        DebugCycleCounter *counter = &memory->counters[counterIndex];
        
        if (counter->hitCount)
        {
            RenderString string = MakeRenderString(platform, "%s: %I64u cycles | %u hits | %I64u cycles/hit",
                                                   nameTable[counterIndex], counter->cycleCount, counter->hitCount, counter->cycleCount / counter->hitCount);
            PushText(commands, V2F(1.0f), string, font, camera, debugLineOffset, scale, V4F(1.0f));
            
            LoadedAssetHeader *metadataHeader = GetAsset(commands->loadedAssets, AssetType_FontMetadata, font, true);
            FontMetadata metadata = metadataHeader->metadata;
            debugLineOffset.y += metadata.lineGap * scale;
        }
    }
}

function void OutputTestSineWave(Game_State *gameState, Game_AudioBuffer *audioBuffer, s32 toneHz)
{
    persist f32 tSine;
    
    s16 volume = 3000;
    s32 wavePeriod = audioBuffer->samplesPerSecond / toneHz;
    
    s16 *sampleOut = (s16 *)audioBuffer->samples;
    for (s32 sampleIndex = 0; sampleIndex < audioBuffer->sampleCount; ++sampleIndex)
    {
        f32 sineValue = Sin(tSine);
        s16 sampleValue = (s16)(sineValue * volume);
        
        //s16 sampleValue = 0;
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        
        tSine += (TAU * (1.0f / (f32)wavePeriod));
        if (tSine > TAU)
        {
            tSine -= TAU;
        }
    }
}

inline v2f RandomChoosePointInArea(Game_State *gameState, v2f min, v2f max, b32 exclusionArea)
{
    v2f result = V2F();
    
    if (exclusionArea)
    {
        f32 x0 = rnd_pcg_nextf(&gameState->pcg) * min.x;
        f32 x1 = (rnd_pcg_nextf(&gameState->pcg) * (gameState->worldDims.x - max.x)) + max.x;
        f32 x = rnd_pcg_range(&gameState->pcg, 0, 1) ? x1 : x0;
        
        f32 y0 = rnd_pcg_nextf(&gameState->pcg) * min.y;
        f32 y1 = (rnd_pcg_nextf(&gameState->pcg) * (gameState->worldDims.y - max.y)) + max.y;
        f32 y = rnd_pcg_range(&gameState->pcg, 0, 1) ? y1 : y0;
        
        result = V2F(x, y);
    }
    else
    {
        f32 x = (rnd_pcg_nextf(&gameState->pcg) * (max.x - min.x)) + min.x;
        f32 y = (rnd_pcg_nextf(&gameState->pcg) * (max.y - min.y)) + min.y;
        result = V2F(x, y);
    }
    
    return result;
}

inline b32 InputNoRepeat(Game_ButtonState buttonState)
{
    b32 result = buttonState.endedFrameDown && (buttonState.halfTransitionCount % 2 != 0);
    return result;
}

#define SetCollisionRuleForPair(a, b, rule) gameState->collisionRules[a][b] = gameState->collisionRules[b][a] = (rule)
#define SetTriggerAction(a, b, trigger) gameState->collisionActions[a][b] = {b, trigger}
function void BuildCollisionRulesAndTriggers(Game_State *gameState)
{
    SetCollisionRuleForPair(ColliderType_Player, ColliderType_Asteroid, Collision_Trigger);
    SetCollisionRuleForPair(ColliderType_Player, ColliderType_UFO, Collision_Trigger);
    SetCollisionRuleForPair(ColliderType_Player, ColliderType_Shot_UFO, Collision_Trigger);
    SetCollisionRuleForPair(ColliderType_Player, ColliderType_Debug_Wall, Collision_Solid);
    
    SetCollisionRuleForPair(ColliderType_Asteroid, ColliderType_UFO, Collision_Trigger);
    SetCollisionRuleForPair(ColliderType_Asteroid, ColliderType_Shot_Player, Collision_Trigger);
    SetCollisionRuleForPair(ColliderType_Asteroid, ColliderType_Shot_UFO, Collision_Trigger);
    
    SetCollisionRuleForPair(ColliderType_UFO, ColliderType_Shot_Player, Collision_Trigger);
    
    SetCollisionRuleForPair(ColliderType_Debug_Wall, ColliderType_Debug_Wall, Collision_Solid);
    
    SetTriggerAction(ColliderType_Shot_Player, ColliderType_Asteroid, CollisionTrigger_PlayerShot_Asteroid);
    SetTriggerAction(ColliderType_Shot_Player, ColliderType_UFO, CollisionTrigger_PlayerShot_UFO);
}

global b32 debug_info = false;
global b32 debug_colliders = false;
global b32 debug_cam = false;
global b32 debug_regions = false;

#if AST_INTERNAL
Game_Memory *debugGlobalMem;
#endif
extern "C" GAME_UPDATE_RENDER(Game_UpdateRender)
{
#if AST_INTERNAL
    debugGlobalMem = memory;
#endif
    BEGIN_TIMED_BLOCK(GameUpdateRender);
    
    ASSERT(sizeof(Game_State) <= memory->permaStorageSize);
    Game_State *gameState = (Game_State *)memory->permaStorage;
    PlatformAPI platform = memory->platform;
    if (!gameState->initialised)
    {
        u32 timeSeed = SafeTruncateU64(memory->platform.SecondsSinceEpoch());
        rnd_pcg_seed(&gameState->pcg, timeSeed);
        
        gameState->worldDims = V2F(100.0f);
        
        for (s32 i = 0; i < ARRAY_COUNT(gameState->entities); ++i)
        {
            gameState->entities[i] = NullEntity;
        }
        
        BuildCollisionRulesAndTriggers(gameState);
        
        gameState->gameCamera = InitialiseCamera(V3F(gameState->worldDims / 2.0f, 10.0f), 10.0f, 0.1f, 0.0f);
        
        gameState->entities[0] = NullEntity;
        
        v3f playerStartPos = V3F(gameState->worldDims / 2.0f, 0.0f);
        v2f playerDims = V2F(2.5f);
        gameState->entities[1] = MakeEntity(Entity_Player, MakeCollider(gameState, ColliderType_Player, playerStartPos.xy, playerDims), 1, true, playerStartPos, playerDims, true, TAU * 0.25f);
        gameState->playerEntity = &gameState->entities[1];
        LinkCameraToEntity(&gameState->gameCamera, gameState->playerEntity->index);
        
        v3f wallPos = V3F(gameState->worldDims / 4.0f, -1.0f);
        v2f wallDims = V2F(10.0f);
        gameState->entities[ARRAY_COUNT(gameState->entities) - 1] = MakeEntity(Entity_Debug_Wall, MakeCollider(gameState, ColliderType_Debug_Wall, wallPos.xy, wallDims), ARRAY_COUNT(gameState->entities) - 1, true, wallPos, wallDims, false);
        gameState->entities[ARRAY_COUNT(gameState->entities) - 2] = MakeEntity(Entity_Debug_Wall, MakeCollider(gameState, ColliderType_Debug_Wall, V2F(wallPos.x, wallPos.y - 10.0f), wallDims), ARRAY_COUNT(gameState->entities) - 1, true, V3F(wallPos.x, wallPos.y - 10.0f, 0.0f), wallDims, false);
        
        gameState->asteroidRotationRate = 0.5f;
        
        s32 initialAsteroidCount = 4;
        for (s32 i = 0; i < initialAsteroidCount; ++i)
        {
            Entity *currAst = &gameState->entities[2 + i];
            v3f astInitialPos = V3F(RandomChoosePointInArea(gameState, 0.4f * gameState->worldDims, 0.6f * gameState->worldDims, true), 0.0f);
            u8 astBitmapIndex = (u8)rnd_pcg_range(&gameState->pcg, 0, 3);
            f32 astRotationDir = (f32)rnd_pcg_range(&gameState->pcg, -1, 1);
            f32 dA = astRotationDir * gameState->asteroidRotationRate;
            
            v2f baseDP = {};
            while (baseDP.x == 0.0f) { baseDP.x = (f32)rnd_pcg_range(&gameState->pcg, -1, 1); }
            while (baseDP.y == 0.0f) { baseDP.y = (f32)rnd_pcg_range(&gameState->pcg, -1, 1); }
            baseDP *= GetAsteroidBaseSpeed(AsteroidSize_Large) * (V2F(rnd_pcg_nextf(&gameState->pcg), rnd_pcg_nextf(&gameState->pcg)) + 0.5f);
            
            *currAst = MakeEntity_Asteroid(gameState, 2 + i, true, astInitialPos, baseDP, dA, AsteroidSize_Large, astBitmapIndex, memory->platform);
        }
        gameState->asteroidCount = 4;
        
        gameState->ufoSpawnTimer = InitialiseTimer(5.0f, 0.0f);
        gameState->ufoSpawned = false;
        
        gameState->initialised = true;
    }
    
    ASSERT(sizeof(Transient_State) <= memory->transStorageSize);
    Transient_State *transState = (Transient_State *)memory->transStorage;
    if (!transState->initialised)
    {
        InitMemRegion(&transState->transRegion, memory->transStorageSize - sizeof(Transient_State), (u8 *)memory->transStorage + sizeof(Transient_State));
        
        for (u32 memIndex = 0; memIndex < ARRAY_COUNT(transState->parallelMems); ++memIndex)
        {
            ParallelMemory *mem = &transState->parallelMems[memIndex];
            
            mem->inUse = false;
            CreateMemorySubRegion(&mem->memRegion, &transState->transRegion, MEGABYTE(1));
        }
        
        transState->loadedAssets = InitialiseLoadedAssets(platform, memory->parallelQueue);
        transState->loadedAssets.transState = transState;
        renderCommands->loadedAssets = &transState->loadedAssets;
        
        transState->initialised = true;
    }
    
    v2f worldToPixelConversion = V2F((f32)renderCommands->width, (f32)renderCommands->height) / gameState->worldDims;
    
    TempMemory renderMemory = StartTempMemory(&transState->transRegion);
    RenderGroup *renderGroup = AllocateRenderGroup(&transState->transRegion, gameState, memory, worldToPixelConversion);
    
    if (gameState->paused)
    {
        input->deltaTime = 0.0f;
    }
    
    Game_Keyboard *keyboard = &input->keyboard;
    f32 playerRotateSpeed = 5.0f * input->deltaTime;
    v2f playerForward = V2F(Cos(gameState->playerEntity->angle), Sin(gameState->playerEntity->angle));
    gameState->playerDDP = V2F();
    
    if (keyboard->keyW.endedFrameDown)
    {
        gameState->playerDDP = V2F(50.0f) * playerForward;
    }
    if (keyboard->keyA.endedFrameDown)
    {
        gameState->playerEntity->angle += playerRotateSpeed;
    }
    if (keyboard->keyD.endedFrameDown)
    {
        gameState->playerEntity->angle -= playerRotateSpeed;
    }
    if (InputNoRepeat(keyboard->keySpace))
    {
        FindFirstNullEntityResult found = FindFirstNullEntity(gameState->entities, ARRAY_COUNT(gameState->entities));
        ASSERT(found.entity);
        
        v2f dP = playerForward * 75.0f;
        *found.entity = MakeEntity_Shot(gameState, found.index, gameState->playerEntity->pos, dP, 1.0f, memory->platform);
    }
    if (InputNoRepeat(keyboard->keyEsc))
    {
        gameState->paused = !gameState->paused;
    }
    if (InputNoRepeat(keyboard->keyF1))
    {
        debug_info = !debug_info;
    }
    if (InputNoRepeat(keyboard->keyF2))
    {
        debug_colliders = !debug_colliders;
    }
    if (InputNoRepeat(keyboard->keyF3))
    {
        debug_cam = !debug_cam;
        
    }
    if (InputNoRepeat(keyboard->keyF4))
    {
        debug_regions = !debug_regions;
    }
    if (keyboard->keyUp.endedFrameDown)
    {
        gameState->gameCamera.pos.y += 5.0f * input->deltaTime;
    }
    if (keyboard->keyDown.endedFrameDown)
    {
        gameState->gameCamera.pos.y -= 5.0f * input->deltaTime;
    }
    if (keyboard->keyLeft.endedFrameDown)
    {
        gameState->gameCamera.pos.x -= 5.0f * input->deltaTime;
    }
    if (keyboard->keyRight.endedFrameDown)
    {
        gameState->gameCamera.pos.x += 5.0f * input->deltaTime;
    }
    
    if (gameState->playerDDP.x != 0.0f || gameState->playerDDP.y != 0.0f)
    {
        f32 maxSpeed = 50.0f;
        gameState->playerEntity->dP = (gameState->playerEntity->dP + (gameState->playerDDP * input->deltaTime));
        f32 vecLen = VectorLength(gameState->playerEntity->dP);
        if (vecLen > maxSpeed)
        {
            gameState->playerEntity->dP *= maxSpeed / vecLen;
        }
    }
    else
    {
        v2f decelRate = V2F(1.0f);
        gameState->playerEntity->dP = gameState->playerEntity->dP - ((decelRate * gameState->playerEntity->dP) * input->deltaTime);
        f32 stopEpsilon = 0.0025f;
        if (gameState->playerEntity->dP.x < stopEpsilon && gameState->playerEntity->dP.x > -stopEpsilon)
        {
            gameState->playerEntity->dP.x = 0.0f;
        }
        if (gameState->playerEntity->dP.y < stopEpsilon && gameState->playerEntity->dP.y > -stopEpsilon)
        {
            gameState->playerEntity->dP.y = 0.0f;
        }
    }
    
    PushClear(renderCommands, V4F(0.0f, 0.0f, 0.0f, 1.0f));
    
    BEGIN_TIMED_BLOCK(UpdateEntities);
    UpdateTimer(&gameState->ufoSpawnTimer, input->deltaTime);
    if (gameState->ufoSpawnTimer.finished && !gameState->ufoSpawned)
    {
        FindFirstNullEntityResult found = FindFirstNullEntity(gameState->entities, ARRAY_COUNT(gameState->entities));
        ASSERT(found.entity);
        
        *found.entity = MakeEntity_UFO(gameState, found.index, V3F(0.0f, rnd_pcg_nextf(&gameState->pcg) * gameState->worldDims.y, 0.0f), V2F(5.0f, 3.0f),
                                       ((rnd_pcg_next(&gameState->pcg) % 2) == 0) ? -1 : 1, memory->platform);
        gameState->ufoSpawned = true;
    }
    
    for (s32 entityIndex = 0; entityIndex < ARRAY_COUNT(gameState->entities); ++entityIndex)
    {
        Entity *entity = &gameState->entities[entityIndex];
        
        if (entity->active)
        {
            entity->angle = RotateEntity(input, *entity);
            
            entity->newPos.xy = MoveEntity(input, *entity, gameState->worldDims, entity->loop);
            entity->newPos.z = entity->pos.z;
            entity->collider.origin = entity->newPos.xy;
            
            HandleCollisions(gameState, entity, platform);
            
            entity->pos = entity->newPos;
            entity->collider.origin = entity->newPos.xy;
            
            switch (entity->type)
            {
                case Entity_Player:
                {
                    PushBitmap(renderCommands, renderGroup->worldToPixelConversion, gameState->gameCamera, BitmapID_Player_NoTrail, entity->pos, entity->dims, entity->angle);
                    PushRect(renderCommands, renderGroup->worldToPixelConversion, gameState->gameCamera, V3F(entity->pos.xy + (playerForward * 2), entity->pos.z), V2F(1.0f), 0.0f, V4F(1.0f, 0.0f, 0.0f, 1.0f));
                    PushRect(renderCommands, renderGroup->worldToPixelConversion, gameState->gameCamera, entity->pos, V2F(1.0f), 0.0f, V4F(1.0f, 1.0f, 0.0f, 1.0f));
                } break;
                
                case Entity_Shot:
                {
                    EntityInfo_Shot *shotInfo = (EntityInfo_Shot *)entity->extraInfo;
                    UpdateTimer(&shotInfo->timer, input->deltaTime);
                    if (shotInfo->timer.finished)
                    {
                        ClearEntity(entity, memory->platform);
                    }
                    else
                    {
                        PushRect(renderCommands, renderGroup->worldToPixelConversion, gameState->gameCamera, entity->pos, entity->dims, 0.0f, V4F(0.0f, 1.0f, 0.0f, 1.0f));
                    }
                } break;
                
                case Entity_Asteroids:
                {
                    BitmapID bitmapID = GetAsteroidBitmapID(((EntityInfo_Asteroid *)entity->extraInfo)->bitmapIndex);
                    PushBitmap(renderCommands, renderGroup->worldToPixelConversion, gameState->gameCamera, bitmapID, entity->pos, entity->dims, entity->angle, V4F(0.0f, 1.0f, 1.0f, 1.0f));
                    PushRect(renderCommands, renderGroup->worldToPixelConversion, gameState->gameCamera, entity->pos, V2F(1.0f), 0.0f, V4F(1.0f, 0.0f, 1.0f, 1.0f));
                } break;
                
                case Entity_UFO:
                {
                    EntityInfo_UFO *ufoInfo = (EntityInfo_UFO *)entity->extraInfo;
                    UpdateTimer(&ufoInfo->timer, input->deltaTime);
                    if (ufoInfo->timer.finished)
                    {
                        ClearEntity(entity, memory->platform);
                        ResetTimer(&gameState->ufoSpawnTimer);
                        gameState->ufoSpawned = false;
                    }
                    else
                    {
                        if (entity->pos.x > (0.45f * gameState->worldDims.x) && entity->pos.x < (0.55f * gameState->worldDims.x))
                        {
                            entity->dP.y = 10.0f * (f32)ufoInfo->vMoveDir;
                        }
                        else
                        {
                            entity->dP.y = 0.0f;
                        }
                        
                        PushBitmap(renderCommands, renderGroup->worldToPixelConversion, gameState->gameCamera, BitmapID_UFO_Large, entity->pos, entity->dims, entity->angle, V4F(1.0f, 1.0f, 1.0f, 1.0f));
                        PushRect(renderCommands, renderGroup->worldToPixelConversion, gameState->gameCamera, entity->pos, V2F(1.0f), 0.0f, V4F(0.0f, 0.0f, 1.0f, 1.0f));
                    }
                } break;
                
                case Entity_Debug_Wall:
                {
                    PushRect(renderCommands, renderGroup->worldToPixelConversion, gameState->gameCamera, entity->pos, entity->dims, 0.0f, V4F(1.0f));
                } break;
                
                default: { if (entity->type != Entity_Null) { printf("Entity Type %d unhandled\n", entity->type); } } break;
            }
            
            if (debug_colliders)
            {
                PushHollowRect(renderCommands, renderGroup->worldToPixelConversion, gameState->gameCamera, V3F(entity->collider.origin, 0.0f), entity->collider.dims, entity->angle, 0.15f, V4F(0.0f, 0.0f, 1.0f, 1.0f));
            }
        }
        
        if (debug_regions)
        {
            PushHollowRect(renderCommands, renderGroup->worldToPixelConversion, gameState->gameCamera, V3F(gameState->worldDims / 2.0f, 0.0f), gameState->worldDims, 0.0f, 0.5f, V4F(1.0f, 0.0f, 0.0f, 1.0f));
            PushHollowRect(renderCommands, renderGroup->worldToPixelConversion, gameState->gameCamera, V3F(gameState->gameCamera.pos.xy, 0.0f), V2F(10.0f), 0.0f, 0.5f, V4F(0.4f, 0.1f, 0.8f, 1.0f));
        }
        
        if (entityIndex == gameState->gameCamera.linkedEntityIndex)
        {
            UpdateCamera(&gameState->gameCamera, *entity);
        }
    }
    END_TIMED_BLOCK(UpdateEntities);
    
    RenderGroup *interfaceGroup = AllocateRenderGroup(&transState->transRegion, gameState, memory, V2F(1.0f));
    
    RenderString scoreString = MakeRenderString(memory->platform, "%02d", gameState->score);
    PushText(renderCommands, interfaceGroup->worldToPixelConversion, scoreString, "Hyperspace", gameState->gameCamera, V3F(gameState->gameCamera.pos.xy + V2F(-425.0f, 350.0f), 0.0f), 0.5f, V4F(1.0f));
    
    END_TIMED_BLOCK(GameUpdateRender);
    
    
#if AST_INTERNAL
    //RenderGroup *debugGroup = AllocateRenderGroup(&transState->transRegion, gameState, memory, V2F(1.0f));
    if (debug_info)
    {
        Debug_CycleCounters(memory, renderCommands, gameState->gameCamera, memory->platform);
    }
    if (debug_cam)
    {
        ChangeCameraDistance(&gameState->gameCamera, 20.0f);
    }
    else
    {
        ChangeCameraDistance(&gameState->gameCamera, 10.0f);
    }
    
    RenderString metricString = MakeRenderString(memory->platform, "%.01f\n%.03f", 1.0f / input->deltaTime, 1000.0f * input->deltaTime);
    PushText(renderCommands, V2F(1.0f), metricString, "Arial", gameState->gameCamera, V3F(gameState->gameCamera.pos.xy + V2F(350.0f, 350.0f), 0.0f), 0.15f, V4F(1.0f));
#endif
    
    FinishTempMemory(renderMemory);
    
    // AUDIO
    //OutputTestSineWave(gameState, audioBuffer, 256);
}
