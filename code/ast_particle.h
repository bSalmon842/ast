/*
Project: Asteroids
File: ast_particle.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_PARTICLE_H

#define PARTICLE_SIM(funcName) void funcName(Game_State *gameState, PlatformAPI platform, struct Emitter *emitter, struct Particle *particle, f32 deltaTime)
typedef PARTICLE_SIM(particleSim);

enum EmitterLifeType
{
    EmitterLife_Timed,      // Emitter is deactivated at end of timer and will respawn particles if they are destroyed in the meantime
    EmitterLife_Wait,       // Emitter will wait until all particles have destroyed themselves to deactivate
    EmitterLife_Continuous, // Emitter will proceed continuously and respawn particles
};

struct Particle
{
    b32 active;
    
    Collider collider;
    
    v3f pos;
    v2f dP;
    v2f newPos;
    
    v2f dims;
    
    BitmapID bitmap;
    v4f colour;
    Timer timer;
};

struct EmitterShapeInfo
{
    f32 minAngle;
    f32 maxAngle;
    v2f base;
};

struct EmitterProgressionInfo
{
    EmitterLifeType life;
    
    f32 minParticleTime;
    f32 maxParticleTime;
    
    v4f initialColour;
    v4f endColour;
    
    f32 emitTimeLife;
    f32 maxParticleSpeed;
};

struct Emitter
{
    b32 active; // Won't work for pulsing emitters if we want them, as inactive emitters can be overwritten in AddEmitter
    v3f pos;
    b32 collide;
    
    Particle *particles;
    u32 particleCount;
    v2f particleDims;
    particleSim *simFunc;
    
    EmitterProgressionInfo progress;
    EmitterShapeInfo shape;
    
    BitmapID bitmaps[8];
    u8 usableBitmapCount;
    
    Timer emitTimer;
};

#define AST_PARTICLE_H
#endif // AST_PARTICLE_H
