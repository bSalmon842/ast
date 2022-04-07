/*
Project: Asteroids
File: ast_particle.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

function PARTICLE_SIM(DefaultParticleSim)
{
    particle->pos += particle->dP * deltaTime;
}

function Particle SpawnParticle(Game_State *gameState, Emitter *emitter)
{
    Particle result = {};
    
    result.active = true;
    
    result.pos = emitter->pos + V3F((rnd_pcg_nextf(&gameState->pcg) * emitter->shape.base) - (emitter->shape.base / 2.0f), 0.0f);
    
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
    
    result.dP = V3F(V2F(Cos(angle), Sin(angle)), 0.0f) * (5.0f * rnd_pcg_nextf(&gameState->pcg));
    
    result.timer = InitialiseTimer(rnd_pcg_nextf(&gameState->pcg) * emitter->maxParticleTime, 0.0f);
    
    if (emitter->initialColour == V4F())
    {
        result.colour = V4F(rnd_pcg_nextf(&gameState->pcg), rnd_pcg_nextf(&gameState->pcg), rnd_pcg_nextf(&gameState->pcg), 1.0f);
    }
    else
    {
        result.colour = emitter->initialColour;
    }
    
    return result;
};

function Emitter InitialiseEmitter(Game_State *gameState, v3f emitterPos, b32 startActive, EmitterLifeType life, EmitterShapeInfo shape, f32 maxParticleTime, v4f initialColour, v4f endColour, particleSim *simFunc = DefaultParticleSim, f32 emitTimeLife = 0.0f)
{
    Emitter result = {};
    
    result.active = startActive;
    result.simFunc = simFunc;
    result.pos = emitterPos;
    result.life = life;
    result.shape = shape;
    result.emitTimer = InitialiseTimer(emitTimeLife, 0.0f, true);
    result.maxParticleTime = maxParticleTime;
    result.initialColour = initialColour;
    result.endColour = endColour;
    
    if (result.life == EmitterLife_Timed)
    {
        if (result.emitTimer.startTime == 0.0f)
        {
            result.life = EmitterLife_Wait;
        }
        else
        {
            ToggleTimer(&result.emitTimer);
        }
    }
    
    for (s32 particleIndex = 0; particleIndex < ARRAY_COUNT(result.particles); ++particleIndex)
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
        
        for (s32 particleIndex = 0; particleIndex < ARRAY_COUNT(emitter->particles); ++particleIndex)
        {
            Particle *particle = &emitter->particles[particleIndex];
            
            if (particle->active)
            {
                if (!particle->timer.finished)
                {
                    UpdateTimer(&particle->timer, input->deltaTime);
                }
                
                emitter->simFunc(emitter, particle, input->deltaTime);
                
                if (emitter->initialColour != emitter->endColour)
                {
                    particle->colour = Lerp(emitter->initialColour, emitter->endColour,
                                            (particle->timer.startTime - particle->timer.currTime) / particle->timer.startTime);
                }
                
                PushRect(renderCommands, platform, camera, particle->pos, V2F(0.5f), 0.0f, particle->colour);
                
                if (particle->timer.finished)
                {
                    particle->active = false;
                }
            }
            else
            {
                if (emitter->life != EmitterLife_Wait)
                {
                    *particle = SpawnParticle(gameState, emitter);
                }
            }
            
            areAllParticlesInactive &= !particle->active;
        }
        
        if (areAllParticlesInactive || (emitter->life == EmitterLife_Timed && emitter->emitTimer.finished))
        {
            emitter->active = false;
        }
    }
}