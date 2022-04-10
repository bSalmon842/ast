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

#include "ast_timer.h"
#include "ast_collision.h"
#include "ast_entity.h"
#include "ast_asset.h"
#include "ast_render.h"
#include "ast_world.h"
#include "ast_particle.h"

global b32 debug_info = false;
global b32 debug_colliders = false;
global b32 debug_cam = false;
global b32 debug_regions = false;
global b32 debug_camMove = false;

struct Game_State
{
    b32 initialised;
    
    b32 paused;
    
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

struct Transient_State
{
    b32 initialised;
    
    MemoryRegion transRegion;
    ParallelMemory parallelMems[8];
    
    Game_LoadedAssets loadedAssets;
};

#define AST_H
#endif //AST_H
