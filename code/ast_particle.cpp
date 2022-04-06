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
    
    result.pos = emitter->pos;
    f32 angle = rnd_pcg_nextf(&gameState->pcg) * TAU;
    result.dP = V3F(V2F(Cos(angle), Sin(angle)), 0.0f) * (5.0f * rnd_pcg_nextf(&gameState->pcg));
    result.colour = V4F(rnd_pcg_nextf(&gameState->pcg), rnd_pcg_nextf(&gameState->pcg), rnd_pcg_nextf(&gameState->pcg), 1.0f);
    
    result.timer = InitialiseTimer(rnd_pcg_nextf(&gameState->pcg) * emitter->maxParticleTime, 0.0f);
    
    return result;
};

function Emitter InitialiseEmitter(Game_State *gameState, v3f emitterPos, b32 startActive, EmitterLifeType life, EmitterShapeType shape, f32 maxParticleTime,  f32 emitTimeLife = 0.0f)
{
    Emitter result = {};
    
    result.active = startActive;
    result.pos = emitterPos;
    result.life = life;
    result.shape = shape;
    result.emitTimer = InitialiseTimer(emitTimeLife, 0.0f, true);
    result.maxParticleTime = maxParticleTime;
    
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

function void UpdateRenderEmitter(Game_State *gameState, Game_RenderCommands *renderCommands, Camera camera, Game_Input *input, Emitter *emitter)
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
                
                particle->pos += particle->dP * input->deltaTime;
                
                PushRect(renderCommands, camera, particle->pos, V2F(0.5f), 0.0f, particle->colour);
                
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