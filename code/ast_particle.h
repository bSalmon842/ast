/*
Project: Asteroids
File: ast_particle.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_PARTICLE_H

#define PARTICLE_SIM(funcName) void funcName(struct Emitter *emitter, struct Particle *particle, f32 deltaTime)
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
    
    v3f pos;
    v3f dP;
    v4f colour;
    
    Timer timer;
};

struct EmitterShapeInfo
{
    f32 minAngle;
    f32 maxAngle;
    v2f base;
};

struct Emitter
{
    Particle particles[64];
    
    particleSim *simFunc;
    
    EmitterLifeType life;
    EmitterShapeInfo shape;
    
    v3f pos;
    
    b32 active;
    Timer emitTimer;
    f32 maxParticleTime;
    
    v4f initialColour;
    v4f endColour;
};

#define AST_PARTICLE_H
#endif // AST_PARTICLE_H
