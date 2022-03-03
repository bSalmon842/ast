/*
Project: Asteroids
File: ast.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_H

#include "ast_platform.h"
#include "bs842_vector.h"
#include "ast_memory.h"

#define RND_IMPLEMENTATION
#include "rnd.h"

Make2DStruct(f32, v2f, V2F);
Make2DStruct(s32, v2s, V2S);
Make3DStruct(f32, v3f, V3F);
Make4DStruct(f32, v4f, V4F, v3f);

#include "ast_timer.h"
#include "ast_entity.h"
#include "ast_asset.h"
#include "ast_render.h"

struct Game_State
{
    b32 initialised;
    
    b32 paused;
    
    v2f worldDims;
    
    rnd_pcg_t pcg;
    
    Entity entities[64];
    Entity *playerEntity;
    v2f playerDDP;
    f32 asteroidRotationRate;
    
    u8 asteroidCount;
    
    Timer ufoSpawnTimer;
    b32 ufoSpawned;
    
    u32 score;
};

struct Transient_State
{
    b32 initialised;
    
    MemoryRegion transRegion;
};

#define AST_H
#endif //AST_H
