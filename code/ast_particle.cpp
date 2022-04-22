/*
Project: Asteroids
File: ast_particle.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

function Particle SpawnParticle(Game_State *gameState, Emitter *emitter, BitmapID bitmap)
{
    Particle result = {};
    
    result.active = true;
    
    result.pos = emitter->pos + V3F((rnd_pcg_nextf(&gameState->pcg) * emitter->shape.base) - (emitter->shape.base / 2.0f), 0.0f);
    result.dims = emitter->particleDims;
    result.collider = MakeCollider(gameState, ColliderType_Debug_Particle, result.pos, result.dims);
    result.bitmap = bitmap;
    
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
    
    result.dP = V2F(Cos(angle), Sin(angle)) * (emitter->progress.maxParticleSpeed * rnd_pcg_nextf(&gameState->pcg));
    
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

function Emitter *AddEmitter(Game_State *gameState, PlatformAPI platform, v3f emitterPos, b32 startActive, u32 particleCount, EmitterProgressionInfo progress, EmitterShapeInfo shape, v2f particleDims, b32 collide, BitmapID *bitmaps, u8 bitmapCount, particleSim *simFunc = 0)
{
    Emitter *result = 0;
    
    for (s32 addIndex = 0; addIndex < ARRAY_COUNT(gameState->emitters); ++addIndex)
    {
        if (!gameState->emitters[addIndex].active)
        {
            result = &gameState->emitters[addIndex];
            break;
        }
    }
    
    result->active = startActive;
    result->collide = collide;
    result->simFunc = simFunc;
    result->pos = emitterPos;
    result->progress = progress;
    result->shape = shape;
    result->emitTimer = InitialiseTimer(progress.emitTimeLife, 0.0f, true);
    
    ASSERT(bitmapCount <= ARRAY_COUNT(result->bitmaps));
    result->usableBitmapCount = bitmapCount;
    if (bitmaps && bitmapCount)
    {
        for (s32 i = 0; i < bitmapCount; ++i)
        {
            result->bitmaps[i] = bitmaps[i];
        }
    }
    
    if (result->progress.life == EmitterLife_Timed)
    {
        if (result->emitTimer.startTime == 0.0f)
        {
            result->progress.life = EmitterLife_Wait;
        }
        else
        {
            ToggleTimer(&result->emitTimer);
        }
    }
    
    result->particleDims = particleDims;
    result->particleCount = particleCount;
    result->particles = (Particle *)platform.MemAlloc(result->particleCount * sizeof(Particle));
    for (u32 particleIndex = 0; particleIndex < result->particleCount; ++particleIndex)
    {
        Particle *particle = &result->particles[particleIndex];
        
        if (result->usableBitmapCount)
        {
            BitmapID id = (BitmapID)rnd_pcg_range(&gameState->pcg, 0, result->usableBitmapCount - 1);
            *particle = SpawnParticle(gameState, result, result->bitmaps[id]);
        }
        else
        {
            *particle = SpawnParticle(gameState, result, BitmapID_Null);
        }
    }
    
    return result;
}

function void UpdateRenderEmitter(Game_State *gameState, Game_RenderCommands *renderCommands, Game_LoadedAssets *loadedAssets, Camera camera, Game_Input *input, Emitter *emitter, PlatformAPI platform, DebugSettings debugSettings)
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
                
                
                if (particle->bitmap == BitmapID_Null)
                {
                    PushRect(renderCommands, platform, camera, particle->pos, particle->dims, 0.0f, 0, particle->colour);
                }
                else
                {
                    PushBitmap(renderCommands, loadedAssets, platform, camera, particle->bitmap, particle->pos, particle->dims, 0.0f, 0, particle->colour);
                }
                
                if (debugSettings.colliders)
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
                    *particle = SpawnParticle(gameState, emitter, particle->bitmap);
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
