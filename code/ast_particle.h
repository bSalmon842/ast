/*
Project: Asteroids
File: ast_particle.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_PARTICLE_H

enum EmitterLifeType
{
    EmitterLife_Timed,      // Emitter is deactivated at end of timer and will respawn particles if they are destroyed in the meantime
    EmitterLife_Wait,       // Emitter will wait until all particles have destroyed themselves to deactivate
    EmitterLife_Continuous, // Emitter will proceed continuously and respawn particles
};

enum EmitterShapeType
{
    EmitterShape_Radial,
    EmitterShape_Cone,
    EmitterShape_Cylinder,
};

// TODO(bSalmon): Colour lerp
struct Particle
{
    b32 active;
    
    v3f pos;
    v3f dP;
    v4f colour;
    
    Timer timer;
};

struct Emitter
{
    Particle particles[64];
    
    EmitterLifeType life;
    EmitterShapeType shape;
    
    v3f pos;
    
    b32 active;
    Timer emitTimer;
    f32 maxParticleTime;
};

#define AST_PARTICLE_H
#endif // AST_PARTICLE_H
