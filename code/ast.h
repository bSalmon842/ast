/*
Project: Asteroids
File: ast.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_H

#include "ast_platform.h"

#define BS842_MAKE_STRUCTS
#include "bs842_vector.h"

#include "ast_intrinsics.h"
#include "ast_memory.h"
#include "ast_debug.h"

#define RND_IMPLEMENTATION
#include "rnd.h"

#include "ast_id.h"
#include "ast_timer.h"
#include "ast_intrinsics.h"
#include "ast_math.h"
#include "ast_collision.h"
#include "ast_entity.h"
#include "ast_asset.h"
#include "ast_render.h"
#include "ast_world.h"
#include "ast_particle.h"
#include "ast_token.h"
#include "ast_meta.h"

struct Game_State
{
    b32 initialised;
    
    b32 paused;
    
    UniversalIDList universalIDList;
    
    World world;
    
    rnd_pcg_t pcg;
    
    Camera gameCamera;
    
    Entity entities[256];
    Entity *playerEntity;
    v2f playerDDP;
    f32 asteroidRotationRate;
    
    u8 asteroidCount;
    
    Timer ufoSpawnTimer;
    b32 ufoSpawned;
    
    Emitter emitters[16];
    
    u32 collisionRules[ColliderType_Count][ColliderType_Count];
    CollisionTriggerAction collisionActions[ColliderType_Count][ColliderType_Count];
    
    u32 score;
};

#define PARALLEL_MEM_COUNT 8
struct Transient_State
{
    b32 initialised;
    
    MemoryRegion transRegion;
    ParallelMemory parallelMems[PARALLEL_MEM_COUNT];
    char parallelNameStrings[PARALLEL_MEM_COUNT][32];
    
    Game_LoadedAssets loadedAssets;
};

#define AST_H
#endif //AST_H
