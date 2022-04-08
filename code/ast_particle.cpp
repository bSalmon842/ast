/*
Project: Asteroids
File: ast_particle.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

function Particle SpawnParticle(Game_State *gameState, Emitter *emitter)
{
    Particle result = {};
    
    result.active = true;
    
    result.pos = emitter->pos + V3F((rnd_pcg_nextf(&gameState->pcg) * emitter->shape.base) - (emitter->shape.base / 2.0f), 0.0f);
    result.dims = emitter->particleDims;
    result.collider = MakeCollider(gameState, ColliderType_Debug_Particle, result.pos, result.dims);
    
    f32 angle = 0.0f;
    if (emitter->shape.minAngle == emitter->shape.maxAngle && emitter->shape.base == V2F())
    {
        angle = rnd_pcg_nextf(&gameState->pcg) * TAU;
    }
    else if (emitter->shape.minAngle == emitter->shape.maxAngle && emitter->shape.base != V2F())
    {
        angle = emitter->shape.minAngle;
    }
    else
    {
        if (emitter->shape.minAngle < emitter->shape.maxAngle)
        {
            angle = emitter->shape.minAngle + (rnd_pcg_nextf(&gameState->pcg) * (emitter->shape.maxAngle - emitter->shape.minAngle));
        }
        else
        {
            f32 adjustedMaxAngle = emitter->shape.maxAngle + TAU;
            angle = emitter->shape.minAngle + (rnd_pcg_nextf(&gameState->pcg) * (adjustedMaxAngle - emitter->shape.minAngle));
        }
    }
    angle = (angle < 0.0f) ? angle + TAU : angle;
    angle = (angle > TAU) ? angle - TAU : angle;
    
    result.dP = V2F(Cos(angle), Sin(angle)) * (5.0f * rnd_pcg_nextf(&gameState->pcg));
    
    result.timer = InitialiseTimer(rnd_pcg_nextf(&gameState->pcg) * emitter->progress.maxParticleTime, 0.0f);
    
    if (emitter->progress.initialColour == V4F())
    {
        result.colour = V4F(rnd_pcg_nextf(&gameState->pcg), rnd_pcg_nextf(&gameState->pcg), rnd_pcg_nextf(&gameState->pcg), 1.0f);
    }
    else
    {
        result.colour = emitter->progress.initialColour;
    }
    
    return result;
};

function Emitter InitialiseEmitter(Game_State *gameState, PlatformAPI platform, v3f emitterPos, b32 startActive, u32 particleCount, EmitterProgressionInfo progress, EmitterShapeInfo shape, v2f particleDims, b32 collide, particleSim *simFunc = 0)
{
    Emitter result = {};
    
    result.active = startActive;
    result.collide = collide;
    result.simFunc = simFunc;
    result.pos = emitterPos;
    result.progress = progress;
    result.shape = shape;
    result.emitTimer = InitialiseTimer(progress.emitTimeLife, 0.0f, true);
    
    if (result.progress.life == EmitterLife_Timed)
    {
        if (result.emitTimer.startTime == 0.0f)
        {
            result.progress.life = EmitterLife_Wait;
        }
        else
        {
            ToggleTimer(&result.emitTimer);
        }
    }
    
    result.particleDims = particleDims;
    result.particleCount = particleCount;
    result.particles = (Particle *)platform.MemAlloc(result.particleCount * sizeof(Particle));
    for (u32 particleIndex = 0; particleIndex < result.particleCount; ++particleIndex)
    {
        Particle *particle = &result.particles[particleIndex];
        *particle = SpawnParticle(gameState, &result);
    }
    
    return result;
}

function void UpdateRenderEmitter(Game_State *gameState, Game_RenderCommands *renderCommands, Camera camera, Game_Input *input, Emitter *emitter, PlatformAPI platform)
{
    if (emitter->active)
    {
        UpdateTimer(&emitter->emitTimer, input->deltaTime);
        b32 areAllParticlesInactive = true;
        
        for (u32 particleIndex = 0; particleIndex < emitter->particleCount; ++particleIndex)
        {
            Particle *particle = &emitter->particles[particleIndex];
            
            if (particle->active)
            {
                if (!particle->timer.finished)
                {
                    UpdateTimer(&particle->timer, input->deltaTime);
                }
                
                if (emitter->simFunc)
                {
                    emitter->simFunc(gameState, platform, emitter, particle, input->deltaTime);
                }
                else
                {
                    particle->pos.xy += particle->dP * input->deltaTime;
                    particle->collider.origin.xy = particle->pos.xy;
                }
                
                if (emitter->progress.initialColour != emitter->progress.endColour)
                {
                    particle->colour = Lerp(emitter->progress.initialColour, emitter->progress.endColour,
                                            (particle->timer.startTime - particle->timer.currTime) / particle->timer.startTime);
                }
                
                PushRect(renderCommands, platform, camera, particle->pos, particle->dims, 0.0f, 0, particle->colour);
                
                if (debug_colliders)
                {
                    PushHollowRect(renderCommands, platform, gameState->gameCamera, particle->collider.origin, particle->collider.dims, 0.0f, 0.25f, 0, V4F(0.0f, 0.0f, 1.0f, 1.0f));
                }
                
                if (particle->timer.finished)
                {
                    particle->active = false;
                }
            }
            else
            {
                if (emitter->progress.life != EmitterLife_Wait)
                {
                    *particle = SpawnParticle(gameState, emitter);
                }
            }
            
            areAllParticlesInactive &= !particle->active;
        }
        
        if (areAllParticlesInactive || (emitter->progress.life == EmitterLife_Timed && emitter->emitTimer.finished))
        {
            emitter->active = false;
            platform.MemFree(emitter->particles);
        }
    }
}

// Particle Sim Functions
function PARTICLE_SIM(TestParticleSimCollision)
{
    particle->newPos = particle->pos.xy + particle->dP * deltaTime;
    particle->collider.origin.xy = particle->newPos;
    
    if (emitter->collide)
    {
        HandleParticleCollisions(gameState, particle, platform);
    }
    
    particle->pos.xy = particle->newPos;
    particle->collider.origin.xy = particle->newPos;
}
