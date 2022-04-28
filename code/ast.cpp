/*
Project: Asteroids
File: ast.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

// TODO(bSalmon): Engine:
// TODO(bSalmon): Audio Mixing
// TODO(bSalmon): More OpenGL work
// TODO(bSalmon): Animation (Sprite-sheets?)
// TODO(bSalmon): Menues
// TODO(bSalmon): Loading Screens
// TODO(bSalmon): Collision double dispatch method review
// TODO(bSalmon): Fix warping on resolution change
// TODO(bSalmon): Multiple resolutions
// TODO(bSalmon): Better memory system (actually use MemoryRegions and work out how region eviction needs to work)

// New Debug Infrastructure
// TODO(bSalmon): Record last 256 frames
// TODO(bSalmon): Nesting variables
// TODO(bSalmon): Nesting Functions in timers
// TODO(bSalmon): Memory Vis
// TODO(bSalmon): Render Vis

// Sim Region Brainstorming
// TODO(bSalmon): 2 kinds of entities, one in/near the sim region, and the other dormant which is occasionally updated (1 per frame or multithreaded?)
// TODO(bSalmon): Use a bool to choose whether to full sim or have 2 separate lists?
// TODO(bSalmon): Update Area -> Entities to be full simmed
// TODO(bSalmon): Apron Area -> Entities to be included in the region for collision detection but not simmed at full granularity
// TODO(bSalmon): 3 entity uses (Full Sim | Collision Sim | Dormant Sim)
// TODO(bSalmon): Collision sim could technically live in both lists, so maybe have 2 lists and also use a bool

// TODO(bSalmon): Game:
// TODO(bSalmon): Death and lives

// TODO(bSalmon): Bitmap list
// NOTE(bSalmon): COMPLETE

// TODO(bSalmon): Audio list
// TODO(bSalmon): Ambient noise once every x seconds which speeds up as wave progresses
// TODO(bSalmon): Shot
// TODO(bSalmon): Propulsion
// TODO(bSalmon): Asteroid destroyed
// TODO(bSalmon): Ambient UFO active

#include "ast.h"
#include "ast_timer.cpp"
#include "ast_memory.cpp"
#include "ast_world.cpp"
#include "ast_asset.cpp"
#include "ast_render.cpp"
#include "ast_collision.cpp"
#include "ast_entity.cpp"
#include "ast_particle.cpp"
#include "ast_collision_actions.cpp"
#include "ast_camera.cpp"
#include "ast_debug.cpp"

inline BitmapID GetAsteroidBitmapID(u32 index)
{
    BitmapID result = (BitmapID)(BitmapID_Asteroid0 + index);
    return result;
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
        f32 x1 = (rnd_pcg_nextf(&gameState->pcg) * (gameState->world.area.dims.x - max.x)) + max.x;
        f32 x = rnd_pcg_range(&gameState->pcg, 0, 1) ? x1 : x0;
        
        f32 y0 = rnd_pcg_nextf(&gameState->pcg) * min.y;
        f32 y1 = (rnd_pcg_nextf(&gameState->pcg) * (gameState->world.area.dims.y - max.y)) + max.y;
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
    SetCollisionRuleForPair(ColliderType_Debug_Wall, ColliderType_Debug_Particle, Collision_Solid);
    
    SetTriggerAction(ColliderType_Shot_Player, ColliderType_Asteroid, CollisionTrigger_Shot_Asteroid);
    SetTriggerAction(ColliderType_Shot_Player, ColliderType_UFO, CollisionTrigger_PlayerShot_UFO);
    SetTriggerAction(ColliderType_Shot_UFO, ColliderType_Asteroid, CollisionTrigger_Shot_Asteroid);
}

#if AST_INTERNAL
Game_Memory *debugGlobalMem;
#endif
extern "C" GAME_UPDATE_RENDER(Game_UpdateRender)
{
#if AST_INTERNAL
    debugGlobalMem = memory;
    if (!globalDebugState)
    {
        globalDebugState = (DebugState *)memory->debugStorage;
    }
#endif
    DEBUG_BLOCK_FUNC;
    
    ASSERT(sizeof(Game_State) <= memory->permaStorageSize);
    Game_State *gameState = (Game_State *)memory->permaStorage;
    PlatformAPI platform = memory->platform;
    if (!gameState->initialised)
    {
        u32 timeSeed = SafeTruncateU64(memory->platform.SecondsSinceEpoch());
        rnd_pcg_seed(&gameState->pcg, timeSeed);
        
        //gameState->world = InitialiseWorld(V2F(360.0f, 180.0f), WorldBorder_Loop);
        gameState->world = InitialiseWorld(V2F(100.0f, 100.0f), WorldBorder_Loop);
        
        for (s32 i = 0; i < ARRAY_COUNT(gameState->entities); ++i)
        {
            gameState->entities[i] = NullEntity;
        }
        
        BuildCollisionRulesAndTriggers(gameState);
        
        gameState->gameCamera = InitialiseCamera(V3F(gameState->world.area.dims / 2.0f, DEFAULT_CAMERA_Z));
        
        gameState->entities[0] = NullEntity;
        
        v3f playerStartPos = V3F(gameState->world.area.dims / 2.0f, 0.0f);
        v2f playerDims = V2F(2.5f);
        gameState->entities[1] = MakeEntity(Entity_Player, MakeCollider(gameState, ColliderType_Player, playerStartPos, playerDims), 1, true, playerStartPos, playerDims, TAU * 0.25f);
        gameState->playerEntity = &gameState->entities[1];
        LinkCameraToEntity(&gameState->gameCamera, gameState->playerEntity->index);
        
        v3f wallPos = V3F(gameState->world.area.dims / 4.0f, -1.0f);
        v2f wallDims = V2F(10.0f);
        gameState->entities[ARRAY_COUNT(gameState->entities) - 1] = MakeEntity(Entity_Debug_Wall, MakeCollider(gameState, ColliderType_Debug_Wall, wallPos, wallDims), ARRAY_COUNT(gameState->entities) - 1, true, wallPos, wallDims, false);
        gameState->entities[ARRAY_COUNT(gameState->entities) - 2] = MakeEntity(Entity_Debug_Wall, MakeCollider(gameState, ColliderType_Debug_Wall, V3F(wallPos.x, wallPos.y - 10.0f, 0.0f), wallDims), ARRAY_COUNT(gameState->entities) - 2, true, V3F(wallPos.x, wallPos.y - 10.0f, 0.0f), wallDims, false);
        gameState->entities[ARRAY_COUNT(gameState->entities) - 3] = MakeEntity(Entity_Debug_Wall, MakeCollider(gameState, ColliderType_Debug_Wall, V3F(70.0f, 80.0f, 0.0f), wallDims), ARRAY_COUNT(gameState->entities) - 3, true, V3F(70.0f, 80.0f, 0.0f), wallDims, false);
        
        gameState->asteroidRotationRate = 0.5f;
        
        s32 initialAsteroidCount = 4;
        for (s32 i = 0; i < initialAsteroidCount; ++i)
        {
            Entity *currAst = &gameState->entities[2 + i];
            v3f astInitialPos = V3F(RandomChoosePointInArea(gameState, 0.4f * gameState->world.area.dims, 0.6f * gameState->world.area.dims, true), 0.0f);
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
        
#if 1 
        EmitterProgressionInfo progress1 = {EmitterLife_Continuous, 0.0f, 5.0f, V4F(), V4F(), 0.0f, 5.0f};
        EmitterProgressionInfo progress2 = {EmitterLife_Continuous, 2.0f, 5.0f, V4F(1.0f, 0.3f, 0.1f, 1.0f), V4F(0.3f, 0.3f, 0.3f, 0.5f), 0.0f, 5.0f};
        
        EmitterShapeInfo shape1 = {0.0f, 0.0f, V2F()};
        EmitterShapeInfo shape2 = {TAU * 0.75f, TAU * 0.25f, V2F()};
        EmitterShapeInfo shape3 = {TAU * 0.25f, TAU * 0.25f, V2F(5.0f, 0.0f)};
        
        AddEmitter(gameState, platform, V3F(gameState->world.area.dims / 2.0f, 0.0f), true, 128,
                   progress1, shape1, V2F(0.5f), false, 0, 0);
        
        BitmapID testBitmaps[4] = {BitmapID_Player_NoTrail, BitmapID_Asteroid0, BitmapID_Asteroid1, BitmapID_UFO_Large};
        AddEmitter(gameState, platform, V3F(gameState->world.area.dims / 3.0f, -2.0f), true, 128,
                   progress1, shape2, V2F(1.0f), false, testBitmaps, 4);
        
        AddEmitter(gameState, platform, V3F(65.0f, 70.0f, 0.0f), true, 128,
                   progress2, shape3, V2F(0.5f), true, 0, 0, TestParticleSimCollision);
#endif
        
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
            mem->memRegion = CreateMemorySubRegion(&transState->transRegion, MEGABYTE(1));
        }
        
        transState->loadedAssets = InitialiseLoadedAssets(platform, memory->parallelQueue);
        transState->loadedAssets.transState = transState;
        renderCommands->loadedAssets = &transState->loadedAssets;
        
        transState->initialised = true;
    }
    
    if (gameState->paused)
    {
        input->deltaTime = 0.0f;
    }
    
    b32 trail = false;
    Game_Keyboard *keyboard = &input->keyboard;
    f32 playerRotateSpeed = 5.0f * input->deltaTime;
    v2f playerForward = V2F(Cos(gameState->playerEntity->angle), Sin(gameState->playerEntity->angle));
    gameState->playerDDP = V2F();
    
    if (keyboard->keyW.endedFrameDown)
    {
        gameState->playerDDP = V2F(50.0f) * playerForward;
        trail = true;
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
        *found.entity = MakeEntity_Shot_Player(gameState, found.index, gameState->playerEntity->pos, dP, 1.0f, memory->platform);
    }
    if (InputNoRepeat(keyboard->keyEsc))
    {
        INVERT(gameState->paused);
    }
    if (InputNoRepeat(keyboard->keyF1))
    {
        INVERT(globalDebugState->openMenu);
    }
    
#if DEBUGUI_CAMMOVE
    f32 debugCamMoveRate = 10.0f * input->deltaTime;
    if (keyboard->keyUp.endedFrameDown)
    {
        gameState->gameCamera.rect.center.y += debugCamMoveRate;
    }
    if (keyboard->keyDown.endedFrameDown)
    {
        gameState->gameCamera.rect.center.y -= debugCamMoveRate;
    }
    if (keyboard->keyLeft.endedFrameDown)
    {
        gameState->gameCamera.rect.center.x -= debugCamMoveRate;
    }
    if (keyboard->keyRight.endedFrameDown)
    {
        gameState->gameCamera.rect.center.x += debugCamMoveRate;
    }
#endif
    
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
    
    f32 widthOverHeight = (f32)renderCommands->width / (f32)renderCommands->height;
    v2f worldToPixelConversion = V2F(10.0f, 10.0f * widthOverHeight);
    CameraMode_Perspective(&gameState->gameCamera, 10.0f, 0.1f, worldToPixelConversion);
    
    PushClear(renderCommands, platform, V4F(0.0f, 0.0f, 0.0f, 1.0f));
    
    UpdateTimer(&gameState->ufoSpawnTimer, input->deltaTime);
    if (gameState->ufoSpawnTimer.finished && !gameState->ufoSpawned)
    {
        FindFirstNullEntityResult found = FindFirstNullEntity(gameState->entities, ARRAY_COUNT(gameState->entities));
        ASSERT(found.entity);
        
        b32 smallUFO = (rnd_pcg_range(&gameState->pcg, 0, 9) >= 8);
        v2f ufoDims = smallUFO ? V2F(3.0f, 3.0f) : V2F(5.0f, 3.0f);
        *found.entity = MakeEntity_UFO(gameState, found.index, V3F(0.0f, rnd_pcg_nextf(&gameState->pcg) * gameState->world.area.dims.y, 0.0f), ufoDims, smallUFO,
                                       ((rnd_pcg_next(&gameState->pcg) % 2) == 0) ? -1 : 1, memory->platform);
        gameState->ufoSpawned = true;
    }
    
    for (s32 entityIndex = 0; entityIndex < ARRAY_COUNT(gameState->entities); ++entityIndex)
    {
        Entity *entity = &gameState->entities[entityIndex];
        
        if (entity->active)
        {
            entity->angle = RotateEntity(input, *entity);
            
            entity->newPos.xy = MoveEntity(input, *entity, gameState->world);
            entity->newPos.z = entity->pos.z;
            entity->collider.origin = entity->newPos;
            
            HandleCollisions(gameState, entity, platform);
            
            entity->pos = entity->newPos;
            entity->collider.origin = entity->newPos;
            
            switch (entity->type)
            {
                case Entity_Player:
                {
                    PushBitmap(renderCommands, &transState->loadedAssets, platform, gameState->gameCamera, trail ? BitmapID_Player_Trail : BitmapID_Player_NoTrail, entity->pos, entity->dims, entity->angle, 0);
                    PushRect(renderCommands, platform, gameState->gameCamera, V3F(entity->pos.xy + (playerForward * 2), entity->pos.z), V2F(1.0f), 0.0f, 0, V4F(1.0f, 0.0f, 0.0f, 1.0f));
                    PushRect(renderCommands, platform, gameState->gameCamera, entity->pos, V2F(1.25f), 0.0f, 0, V4F(0.3f, 0.5f, 0.0f, 1.0f));
                } break;
                
                case Entity_Shot_UFO:
                case Entity_Shot_Player:
                {
                    EntityInfo_Shot *shotInfo = (EntityInfo_Shot *)entity->extraInfo;
                    UpdateTimer(&shotInfo->timer, input->deltaTime);
                    if (shotInfo->timer.finished)
                    {
                        ClearEntity(entity, memory->platform);
                    }
                    else
                    {
                        v4f shotColour = V4F(0.0f, 1.0f, 0.0f, 1.0f);
                        if (entity->type == Entity_Shot_UFO)
                        {
                            shotColour = V4F(1.0f, 0.5f, 0.0f, 1.0f);
                        }
                        
                        PushRect(renderCommands, platform, gameState->gameCamera, entity->pos, entity->dims, 0.0f, 0, shotColour);
                    }
                } break;
                
                case Entity_Asteroids:
                {
                    BitmapID bitmapID = GetAsteroidBitmapID(((EntityInfo_Asteroid *)entity->extraInfo)->bitmapIndex);
                    PushBitmap(renderCommands, &transState->loadedAssets, platform, gameState->gameCamera, bitmapID, entity->pos, entity->dims, entity->angle, 0, V4F(0.0f, 1.0f, 1.0f, 1.0f));
                    PushRect(renderCommands, platform, gameState->gameCamera, entity->pos, V2F(1.0f), 0.0f, 0, V4F(1.0f, 0.0f, 1.0f, 1.0f));
                } break;
                
                case Entity_UFO:
                {
                    EntityInfo_UFO *ufoInfo = (EntityInfo_UFO *)entity->extraInfo;
                    UpdateTimer(&ufoInfo->timer, input->deltaTime);
                    UpdateTimer(&ufoInfo->shotTimer, input->deltaTime);
                    if (ufoInfo->timer.finished)
                    {
                        ClearEntity(entity, memory->platform);
                        ResetTimer(&gameState->ufoSpawnTimer);
                        gameState->ufoSpawned = false;
                    }
                    else
                    {
                        if (ufoInfo->shotTimer.finished)
                        {
                            FindFirstNullEntityResult found = FindFirstNullEntity(gameState->entities, ARRAY_COUNT(gameState->entities));
                            ASSERT(found.entity);
                            
                            // TODO(bSalmon): Make this target nearby asteroids occasionally
                            v2f dP = V2F((rnd_pcg_nextf(&gameState->pcg) * 2.0f) - 1.0f, (rnd_pcg_nextf(&gameState->pcg) * 2.0f) - 1.0f) * 75.0f;
                            *found.entity = MakeEntity_Shot_UFO(gameState, found.index, entity->pos, dP, 1.0f, memory->platform);
                            
                            ResetTimer(&ufoInfo->shotTimer);
                        }
                        
                        if (entity->pos.x > (0.45f * gameState->world.area.dims.x) && entity->pos.x < (0.55f * gameState->world.area.dims.x))
                        {
                            entity->dP.y = 10.0f * (f32)ufoInfo->vMoveDir;
                        }
                        else
                        {
                            entity->dP.y = 0.0f;
                        }
                        
                        BitmapID ufoBitmap = ufoInfo->smallUFO ? BitmapID_UFO_Small : BitmapID_UFO_Large;
                        PushBitmap(renderCommands, &transState->loadedAssets, platform, gameState->gameCamera, ufoBitmap, entity->pos, entity->dims, entity->angle, 0, V4F(1.0f, 1.0f, 1.0f, 1.0f));
                        PushRect(renderCommands, platform, gameState->gameCamera, entity->pos, V2F(1.0f), 0.0f, 0, V4F(0.0f, 0.0f, 1.0f, 1.0f));
                    }
                } break;
                
                case Entity_Debug_Wall:
                {
                    v4f colour = (entity->index % 2 == 1) ? V4F(1.0f) : V4F(0.0f, 0.0f, 1.0f, 1.0f);
                    PushRect(renderCommands, platform, gameState->gameCamera, entity->pos, entity->dims, 0.0f, 0, colour);
                } break;
                
                default: { if (entity->type != Entity_Null) { printf("Entity Type %d unhandled\n", entity->type); } } break;
            }
            
#if DEBUGUI_ENTITY_COLLIDERS
            PushHollowRect(renderCommands, platform, gameState->gameCamera, entity->collider.origin, entity->collider.dims, entity->angle, 0.25f, 0, V4F(0.0f, 0.0f, 1.0f, 1.0f));
#endif
        }
        
#if !DEBUGUI_CAMMOVE
        if (entityIndex == gameState->gameCamera.linkedEntityIndex)
        {
            UpdateCamera(renderCommands, &gameState->gameCamera, *entity);
        }
#endif
    }
    
#if DEBUGUI_REGIONS
    PushHollowRect(renderCommands, platform, gameState->gameCamera, V3F(gameState->world.area.dims / 2.0f, 0.0f), gameState->world.area.dims, 0.0f, 0.5f, 0, V4F(1.0f, 0.0f, 0.0f, 1.0f));
    
    Rect2f cameraRect = GetCameraBoundsForDistance(renderCommands, gameState->gameCamera, gameState->gameCamera.rect.center.xy, DEFAULT_CAMERA_Z);
    PushHollowRect(renderCommands, platform, gameState->gameCamera, V3F(cameraRect.center, 0.0f), cameraRect.dims, 0.0f, 0.5f, 0, V4F(0.0f, 1.0f, 1.0f, 1.0f));
#endif
    
    for (s32 emitterIndex = 0; emitterIndex < ARRAY_COUNT(gameState->emitters); ++emitterIndex)
    {
        Emitter *currEmitter = &gameState->emitters[emitterIndex];
        if (currEmitter->active)
        {
            UpdateRenderEmitter(gameState, renderCommands, &transState->loadedAssets, gameState->gameCamera, input, currEmitter, platform, globalDebugState->settings);
        }
    }
    
    CameraMode_Orthographic(&gameState->gameCamera);
    
    char scoreString[16];
    stbsp_sprintf(scoreString, "%02d", gameState->score);
    v2f scorePixelOffset = V2F(25.0f, (f32)renderCommands->height - 60.0f);
    PushText(renderCommands, &transState->loadedAssets, platform, gameState->gameCamera, scoreString, "Hyperspace",
             V3F(scorePixelOffset, 0.0f), 0.5f, 0, V4F(1.0f));
    
#if AST_INTERNAL
    if (globalDebugState->openMenu)
    {
        DisplayDebugMenu(renderCommands, &transState->loadedAssets, input, platform, gameState->gameCamera, &globalDebugState->settings);
    }
    
#if DEBUGUI_FUNC_TIMERS
    PrintDebugRecords(memory, renderCommands, &transState->loadedAssets, input, gameState->gameCamera, platform);
#endif
    
#if DEBUGUI_CAMZOOM
    ChangeCameraDistance(&gameState->gameCamera, 30.0f);
#else
    ChangeCameraDistance(&gameState->gameCamera, DEFAULT_CAMERA_Z);
#endif
    
    char metricString[16];
    stbsp_sprintf(metricString, "%.01f\n%.03f", 1.0f / input->deltaTime, 1000.0f * input->deltaTime);
    v2f fpsPixelOffset = V2F((f32)renderCommands->width - 100.0f, (f32)renderCommands->height - 50.0f);
    PushText(renderCommands, &transState->loadedAssets, platform, gameState->gameCamera, metricString, "Debug", V3F(fpsPixelOffset, 0.0f), DEBUG_TEXT_SCALE, DEBUG_LAYER, V4F(1.0f));
    
#if DEBUGUI_MOUSEINFO
    char mouseString[16];
    stbsp_sprintf(mouseString, "Mouse X: %4d\nMouse Y: %4d", input->mouse.x, input->mouse.y);
    v2f mousePixelOffset = V2F((f32)renderCommands->width - 300.0f, (f32)renderCommands->height - 50.0f);
    PushText(renderCommands, &transState->loadedAssets, platform, gameState->gameCamera, mouseString, "Debug", V3F(mousePixelOffset, 0.0f), DEBUG_TEXT_SCALE, DEBUG_LAYER, V4F(1.0f));
    
    char mbString[32];
    stbsp_sprintf(mbString, "Mouse L Down: %d\nMouse M Down: %d\nMouse R Down: %d", input->mouse.buttons[0].endedFrameDown, input->mouse.buttons[1].endedFrameDown, input->mouse.buttons[2].endedFrameDown);
    v2f mbPixelOffset = V2F((f32)renderCommands->width - 500.0f, (f32)renderCommands->height - 50.0f);
    PushText(renderCommands, &transState->loadedAssets, platform, gameState->gameCamera, mbString, "Debug", V3F(mbPixelOffset, 0.0f), DEBUG_TEXT_SCALE, DEBUG_LAYER, V4F(1.0f));
#endif
    
#if DEBUGUI_CAMMOVE
    char unlockString[32];
    stbsp_sprintf(unlockString, "CAMERA UNLOCKED");
    v2f unlockPixelOffset = V2F(450.0f, (f32)renderCommands->height - 25.0f);
    PushText(renderCommands, &transState->loadedAssets, platform, gameState->gameCamera, unlockString, "Debug", V3F(unlockPixelOffset, 0.0f), DEBUG_TEXT_SCALE, DEBUG_LAYER, V4F(1.0f, 0.0f, 0.0f, 1.0f));
#endif
    
#endif
    
    // AUDIO
    //OutputTestSineWave(gameState, audioBuffer, 256);
}

////////// DEBUG STUFF FROM HERE ON OUT /////////////*
/////// ABANDON ALL HOPE, YE WHO ENTER HERE ////////

extern "C" GAME_INITIALISE_DEBUG_STATE(Game_InitialiseDebugState)
{
    if (!globalDebugState)
    {
        globalDebugState = (DebugState *)memory->debugStorage;
        if (!globalDebugState->memInitialised)
        {
            InitMemRegion(&globalDebugState->dataRegion, memory->debugStorageSize - sizeof(DebugState), (u8 *)memory->debugStorage + sizeof(DebugState));
            globalDebugState->table = PushStruct(&globalDebugState->dataRegion, DebugTable);
            
            for (u32 translationIndex = 0; translationIndex < TRANSLATION_UNIT_COUNT; ++translationIndex)
            {
                for (u32 blockIndex = 0; blockIndex < MAX_DEBUG_TRANSLATION_UNIT_INFOS; ++blockIndex)
                {
                    DebugBlockStats *lastBlockStat = &globalDebugState->table->lastBlockStats[translationIndex][blockIndex];
                    lastBlockStat->minCycles = U64_MAX;
                    lastBlockStat->maxCycles = 0;
                    lastBlockStat->minHits = U32_MAX;
                    lastBlockStat->maxHits = 0;
                }
            }
            
            globalDebugState->settings.menuSentinel = {};
            DebugMenuItem *timingVisItem = AddNextDebugMenuItem(&globalDebugState->dataRegion, &globalDebugState->settings.menuSentinel, "Timing Vis", DebugMenuFuncType_None, 0);
            AddChildDebugMenuItem(&globalDebugState->dataRegion, timingVisItem, "Function Timers", DebugMenuFuncType_b32, &globalDebugState->settings.config.funcTimers);
            
            DebugMenuItem *boundsItem = AddNextDebugMenuItem(&globalDebugState->dataRegion, timingVisItem, "Bounds Vis", DebugMenuFuncType_None, 0);
            DebugMenuItem *collidersChild = AddChildDebugMenuItem(&globalDebugState->dataRegion, boundsItem, "Show Colliders...", DebugMenuFuncType_None, 0);
            DebugMenuItem *entityCollidersChild = AddChildDebugMenuItem(&globalDebugState->dataRegion, collidersChild, "Entities", DebugMenuFuncType_b32, &globalDebugState->settings.config.entityColliders);
            AddNextDebugMenuItem(&globalDebugState->dataRegion, entityCollidersChild, "Particles", DebugMenuFuncType_b32, &globalDebugState->settings.config.particleColliders);
            AddNextDebugMenuItem(&globalDebugState->dataRegion, collidersChild, "Show Regions", DebugMenuFuncType_b32, &globalDebugState->settings.config.regions);
            
            DebugMenuItem *cameraItem = AddNextDebugMenuItem(&globalDebugState->dataRegion, boundsItem, "Camera Controls", DebugMenuFuncType_None, 0);
            DebugMenuItem *camLockChild = AddChildDebugMenuItem(&globalDebugState->dataRegion, cameraItem, "Camera Lock", DebugMenuFuncType_b32, &globalDebugState->settings.config.camMove);
            AddNextDebugMenuItem(&globalDebugState->dataRegion, camLockChild, "Camera Zoom", DebugMenuFuncType_b32, &globalDebugState->settings.config.camZoom);
            
            AddNextDebugMenuItem(&globalDebugState->dataRegion, cameraItem, "Mouse Info", DebugMenuFuncType_b32, &globalDebugState->settings.config.mouseInfo);
            
            globalDebugState->settings.timerWindowPosY = 700.0f;
            
            globalDebugState->memInitialised = true;
        }
    }
}

extern "C" GAME_DEBUG_FRAME_END(Game_DebugFrameEnd)
{
    ASSERT(!globalDebugState->inFrame);
    
    for (u32 translationIndex = 0; translationIndex < TRANSLATION_UNIT_COUNT; ++translationIndex)
    {
        for (u32 blockIndex = 0; blockIndex < MAX_DEBUG_TRANSLATION_UNIT_INFOS; ++blockIndex)
        {
            DebugBlockInfo *lastBlockInfo = &globalDebugState->table->lastBlockInfos[translationIndex][blockIndex];
            DebugBlockInfo *blockInfo = &globalDebugState->table->blockInfos[translationIndex][blockIndex];
            DebugBlockStats *lastBlockStat = &globalDebugState->table->lastBlockStats[translationIndex][blockIndex];
            
            *lastBlockInfo = *blockInfo;
            blockInfo->cycles = 0;
            blockInfo->hits = 0;
            
            if (lastBlockInfo->cycles > lastBlockStat->maxCycles) { lastBlockStat->maxCycles = lastBlockInfo->cycles; }
            if (lastBlockInfo->cycles < lastBlockStat->minCycles) { lastBlockStat->minCycles = lastBlockInfo->cycles; }
            if (lastBlockInfo->hits > lastBlockStat->maxHits) { lastBlockStat->maxHits = lastBlockInfo->hits; }
            if (lastBlockInfo->hits < lastBlockStat->minHits) { lastBlockStat->minHits = lastBlockInfo->hits; }
        }
    }
    
    if (globalDebugState->settings.configChanged)
    {
        WriteDebugConfigFile(memory->platform, globalDebugState->settings.config);
        memory->platform.DebugSystemCommand("/C running_build.bat", "..\\code");
        globalDebugState->settings.configChanged = false;
    }
}
