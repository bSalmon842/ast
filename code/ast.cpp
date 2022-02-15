/*
Project: Asteroids
File: ast.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

// TODO(bSalmon): Today:
// TODO(bSalmon): UFO
// TODO(bSalmon): Death and lives

// TODO(bSalmon): Bitmap list
// TODO(bSalmon): Ship (w/ & w/o rocket trail
// TODO(bSalmon): UFO
// TODO(bSalmon): Asteroids (4 variants)
// TODO(bSalmon): Shot (?)

// TODO(bSalmon): Audio list
// TODO(bSalmon): Ambient noise once every 2 seconds or so
// TODO(bSalmon): Shot
// TODO(bSalmon): Propulsion
// TODO(bSalmon): Asteroid destroyed
// TODO(bSalmon): Ambient UFO active

#include "ast.h"
#include "ast_intrinsics.h"
#include "ast_math.h"
#include "ast_render.cpp"
#include "ast_entity.cpp"

function void Debug_CycleCounters(Game_Memory *memory)
{
    char *nameTable[] = 
    {
        "GameUpdateRender",
        "RenderBitmap_Slow",
        "ProcessPixel",
    };
    
    printf("\n\nCYCLE COUNTS:\n");
    for (s32 counterIndex = 0; counterIndex < ARRAY_COUNT(memory->counters); ++counterIndex)
    {
        DebugCycleCounter *counter = &memory->counters[counterIndex];
        
        if (counter->hitCount)
        {
            char string[128];
            sprintf(string, "%s: %I64u cycles | %u hits | %I64u cycles/hit", nameTable[counterIndex], counter->cycleCount, counter->hitCount, counter->cycleCount / counter->hitCount);
            printf("%s\n", string);
        }
    }
}

inline BitScanResult FindLeastSignificantSetBit(u32 value)
{
    BitScanResult result = {};
    
    for (u32 test = 0; test < 32; ++test)
    {
        if (value & (1 << test))
        {
            result.index = test;
            result.found = true;
            break;
        }
    }
    
    return result;
}

function Bitmap LoadBMP(PlatformAPI platform, char *filename)
{
    Bitmap result = {};
    
    Debug_ReadFileResult readResult = platform.Debug_ReadFile(filename);
    if (readResult.contents)
    {
        BitmapHeader *header = (BitmapHeader *)readResult.contents;
        u32 *pixels = (u32 *)((u8 *)readResult.contents + header->bitmapOffset);
        result.memory = pixels;
        
        result.dims = V2S(header->width, header->height);
        result.pitch = -header->width * (header->bitsPerPixel / 8);
        
        ASSERT(result.dims.h >= 0);
        ASSERT(header->compression == 3);
        
        u32 redMask = header->redMask;
        u32 greenMask = header->greenMask;
        u32 blueMask = header->blueMask;
        u32 alphaMask = ~(redMask | greenMask | blueMask);
        
        BitScanResult redShift = FindLeastSignificantSetBit(redMask);
        BitScanResult greenShift = FindLeastSignificantSetBit(greenMask);
        BitScanResult blueShift = FindLeastSignificantSetBit(blueMask);
        BitScanResult alphaShift = FindLeastSignificantSetBit(alphaMask);
        
        ASSERT(redShift.found);
        ASSERT(greenShift.found);
        ASSERT(blueShift.found);
        ASSERT(alphaShift.found);
        
        v2s min = V2S(header->width, header->height);
        v2s max = V2S();
        for (s32 y = 0; y < header->height; ++y)
        {
            for (s32 x = 0; x < header->width; ++x)
            {
                u32 pixel = *(pixels + (y * header->width) + x);
                v4f texel = V4F((f32)((pixel >> redShift.index) & 0xFF),
                                (f32)((pixel >> greenShift.index) & 0xFF),
                                (f32)((pixel >> blueShift.index) & 0xFF),
                                (f32)((pixel >> alphaShift.index) & 0xFF));
                if (texel.a > 0.0f)
                {
                    if (x < min.x)
                    {
                        min.x = x;
                    }
                    if (y < min.y)
                    {
                        min.y = y;
                    }
                    if (x > max.x)
                    {
                        max.x = x;
                    }
                    if (y > max.y)
                    {
                        max.y = y;
                    }
                }
            }
        }
        
        max += 1;
        result.dims = max - min;
        result.pitch = -result.dims.w * BITMAP_BYTES_PER_PIXEL;
        
        result.memory = platform.MemAlloc((result.dims.h * result.dims.w) * BITMAP_BYTES_PER_PIXEL);
        
        u32 *srcDest = pixels;
        u32 *outPixel = (u32 *)result.memory;
        for (s32 y = min.y; y < max.y; ++y)
        {
            for (s32 x = min.x; x < max.x; ++x)
            {
                u32 pixel = *(srcDest + (y * header->width) + x);
                v4f texel = V4F((f32)((pixel >> redShift.index) & 0xFF),
                                (f32)((pixel >> greenShift.index) & 0xFF),
                                (f32)((pixel >> blueShift.index) & 0xFF),
                                (f32)((pixel >> alphaShift.index) & 0xFF));
                
                texel.r = Sq(texel.r / 255.0f);
                texel.g = Sq(texel.g / 255.0f);
                texel.b = Sq(texel.b / 255.0f);
                texel.a = texel.a / 255.0f;
                
                texel.rgb *= texel.a;
                
                texel.r = SqRt(texel.r) * 255.0f;
                texel.g = SqRt(texel.g) * 255.0f;
                texel.b = SqRt(texel.b) * 255.0f;
                texel.a = texel.a * 255.0f;
                
                *outPixel++ = (RoundF32ToU32(texel.a) << 24 |
                               RoundF32ToU32(texel.r) << 16 |
                               RoundF32ToU32(texel.g) << 8 |
                               RoundF32ToU32(texel.b));
            }
        }
    }
    
    result.memory = (u8 *)result.memory - result.pitch * (result.dims.h - 1);
    
    platform.Debug_FreeFile(readResult.contents);
    
    return result;
}

function stbtt_fontinfo LoadTTF(PlatformAPI platform, char *filename)
{
    stbtt_fontinfo result = {};
    
    Debug_ReadFileResult readResult = platform.Debug_ReadFile(filename);
    
    b32 fontInitResult = false;
    fontInitResult = stbtt_InitFont(&result, (u8 *)readResult.contents, 0);
    ASSERT(fontInitResult);
    
    return result;
}

function void OutputTestSineWave(Game_State *gameState, Game_AudioBuffer *audioBuffer, s32 toneHz)
{
    persist f32 tSine;
    
    s16 volume = 3000;
    s32 wavePeriod = audioBuffer->samplesPerSecond / toneHz;
    
    s16 *sampleOut = (s16 *)audioBuffer->samples;
    for (s32 sampleIndex = 0; sampleIndex < audioBuffer->sampleCount; ++sampleIndex)
    {
        f32 sineValue = Sin(tSine);
        s16 sampleValue = (s16)(sineValue * volume);
        
        //s16 sampleValue = 0;
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        
        tSine += (TAU * (1.0f / (f32)wavePeriod));
        if (tSine > TAU)
        {
            tSine -= TAU;
        }
    }
}

inline b32 InputNoRepeat(Game_ButtonState buttonState)
{
    b32 result = buttonState.endedFrameDown && (buttonState.halfTransitionCount % 2 != 0);
    return result;
}

#if AST_INTERNAL
Game_Memory *debugGlobalMem;
#endif
extern "C" GAME_UPDATE_RENDER(Game_UpdateRender)
{
#if AST_INTERNAL
    debugGlobalMem = memory;
#endif
    BEGIN_TIMED_BLOCK(GameUpdateRender);
    
    ASSERT(sizeof(Game_State) <= memory->permaStorageSize);
    Game_State *gameState = (Game_State *)memory->permaStorage;
    if (!gameState->initialised)
    {
        // TODO(bSalmon): Replace with time seed
        rnd_pcg_seed(&gameState->pcg, 0);
        
        gameState->worldDims = V2F(100.0f);
        
        for (s32 i = 0; i < ARRAY_COUNT(gameState->entities); ++i)
        {
            gameState->entities[i] = NullEntity;
        }
        
        gameState->entities[0] = NullEntity;
        gameState->entities[1] = MakeEntity(Entity_Player, true, gameState->worldDims / 2.0f, V2F(5.0f));
        gameState->playerEntity = &gameState->entities[1];
        
        gameState->maxShots = 8;
        
        s32 initialAsteroidCount = 1;
        for (s32 i = 0; i < initialAsteroidCount; ++i)
        {
            Entity *currAst = &gameState->entities[2 + i];
            // TODO(bSalmon): Make sure Asteroids aren't placed on the player
            v2f astInitialPos = V2F(rnd_pcg_nextf(&gameState->pcg) * gameState->worldDims.x,
                                    rnd_pcg_nextf(&gameState->pcg) * gameState->worldDims.y);
            *currAst = MakeEntity(Entity_Asteroids, true, astInitialPos, GetAsteroidDims(AsteroidSize_Large));
            
            v2f baseDP = {};
            while (baseDP.x == 0.0f) { baseDP.x = (f32)rnd_pcg_range(&gameState->pcg, -1, 1); }
            while (baseDP.y == 0.0f) { baseDP.y = (f32)rnd_pcg_range(&gameState->pcg, -1, 1); }
            baseDP *= GetAsteroidBaseSpeed(AsteroidSize_Large);
            
            currAst->dP = baseDP * (V2F(rnd_pcg_nextf(&gameState->pcg), rnd_pcg_nextf(&gameState->pcg)) + 0.5f);
        }
        gameState->asteroidCount = 4;
        
        gameState->playerBitmap = LoadBMP(memory->platform, ".\\player.bmp");
        gameState->asteroidBitmaps[0] = LoadBMP(memory->platform, ".\\ast1.bmp");
        
        gameState->gameFont = LoadTTF(memory->platform, ".\\HyperspaceBold.ttf");
        
        s32 cpuInfo[4];
        __cpuid(cpuInfo, 1);
        gameState->availableInstructions.sse3 = cpuInfo[2] & (1 << 0) || false;
        gameState->availableInstructions.sse4_2 = cpuInfo[2] & (1 << 20) || false;
        gameState->availableInstructions.avx = cpuInfo[2] & (1 << 28) || false;
        
        gameState->initialised = true;
    }
    
    ASSERT(sizeof(Transient_State) <= memory->transStorageSize);
    Transient_State *transState = (Transient_State *)memory->transStorage;
    if (!transState->initialised)
    {
        InitMemRegion(&transState->transRegion, memory->transStorageSize - sizeof(Transient_State), (u8 *)memory->transStorage + sizeof(Transient_State));
        
        transState->initialised = true;
    }
    
    v2f backBufferF = V2F((f32)backBuffer->width, (f32)backBuffer->height);
    
    TempMemory renderMemory = StartTempMemory(&transState->transRegion);
    RenderGroup *renderGroup = AllocateRenderGroup(&transState->transRegion, MEGABYTE(4), backBufferF / gameState->worldDims);
    
    if (gameState->paused)
    {
        input->deltaTime = 0.0f;
    }
    
    u32 boxColour = 0xFFFFFFFF;
    Game_Keyboard *keyboard = &input->keyboard;
    f32 playerRotateSpeed = 5.0f * input->deltaTime;
    v2f playerForward = V2F(Cos(gameState->playerEntity->angle), Sin(gameState->playerEntity->angle));
    gameState->playerDDP = V2F();
    
    persist b32 debugText = false;
    
    if (keyboard->keyW.endedFrameDown)
    {
        gameState->playerDDP = V2F(50.0f) * playerForward;
    }
    if (keyboard->keyA.endedFrameDown)
    {
        gameState->playerEntity->angle -= playerRotateSpeed;
    }
    if (keyboard->keyD.endedFrameDown)
    {
        gameState->playerEntity->angle += playerRotateSpeed;
    }
    if (InputNoRepeat(keyboard->keySpace))
    {
        // TODO(bSalmon): Shot needs a timer, should loop and cover around 66% of the screen
        if (gameState->shotCount < gameState->maxShots)
        {
            Entity *shot = FindFirstNullEntity(gameState->entities, ARRAY_COUNT(gameState->entities));
            ASSERT(shot);
            
            *shot = MakeEntity(Entity_Shot, true, gameState->playerEntity->pos, V2F(1.0f));
            shot->dP = playerForward * 75.0f;
            gameState->shotCount++;
        }
    }
    if (InputNoRepeat(keyboard->keyEsc))
    {
        gameState->paused = !gameState->paused;
    }
    if (InputNoRepeat(keyboard->keyF1))
    {
        debugText = !debugText;
    }
    
    if (gameState->playerDDP.x != 0.0f || gameState->playerDDP.y != 0.0f)
    {
        f32 maxSpeed = 50.0f;
        gameState->playerEntity->dP = (gameState->playerEntity->dP + (gameState->playerDDP * input->deltaTime));
        f32 vecLen = VectorLength(gameState->playerEntity->dP);
        if (vecLen > maxSpeed)
        {
            gameState->playerEntity->dP *= maxSpeed / vecLen;
        }
    }
    else
    {
        v2f decelRate = V2F(1.0f);
        gameState->playerEntity->dP = gameState->playerEntity->dP - ((decelRate * gameState->playerEntity->dP) * input->deltaTime);
        f32 stopEpsilon = 0.0025f;
        if (gameState->playerEntity->dP.x < stopEpsilon && gameState->playerEntity->dP.x > -stopEpsilon)
        {
            gameState->playerEntity->dP.x = 0.0f;
        }
        if (gameState->playerEntity->dP.y < stopEpsilon && gameState->playerEntity->dP.y > -stopEpsilon)
        {
            gameState->playerEntity->dP.y = 0.0f;
        }
    }
    
    PushClear(renderGroup, V4F(0.0f, 0.0f, 0.0f, 1.0f));
    
    for (s32 entityIndex = 0; entityIndex < ARRAY_COUNT(gameState->entities); ++entityIndex)
    {
        Entity *entity = &gameState->entities[entityIndex];
        if (entity->active)
        {
            switch (entity->type)
            {
                case Entity_Player:
                {
                    MoveEntityLoop(input, entity, gameState->worldDims);
                    
                    PushBitmap(renderGroup, &gameState->playerBitmap, entity->pos, entity->dims, entity->angle);
                    PushRect(renderGroup, entity->pos + (playerForward * 2), V2F(1.0f), 0.0f, V4F(1.0f, 0.0f, 0.0f, 1.0f));
                    PushRect(renderGroup, entity->pos, V2F(1.0f), 0.0f, V4F(1.0f, 1.0f, 0.0f, 1.0f));
                } break;
                
                case Entity_Shot:
                {
                    for (s32 astIndex = 0; astIndex < ARRAY_COUNT(gameState->entities); ++astIndex)
                    {
                        Entity *ast = &gameState->entities[astIndex];
                        if (ast->type == Entity_Asteroids && ast->active)
                        {
                            v2f min = ast->pos - (ast->dims / 2.0f);
                            v2f max = ast->pos + (ast->dims / 2.0f);
                            if (entity->pos >= min && entity->pos <= max)
                            {
                                v2f oldPos = ast->pos;
                                v2f oldDims = ast->dims;
                                AsteroidSize oldSize = GetAsteroidSizeFromDims(ast->dims);
                                
                                *entity = NullEntity;
                                gameState->shotCount--;
                                
                                *ast = NullEntity;
                                gameState->asteroidCount--;
                                gameState->score += GetScoreForAsteroidSize(oldSize);
                                
                                if (oldDims > GetAsteroidDims(AsteroidSize_Small))
                                {
                                    s32 asteroidsToCreate = 2;
                                    for (s32 newAstIndex = 0; newAstIndex < ARRAY_COUNT(gameState->entities) && asteroidsToCreate > 0; ++newAstIndex)
                                    {
                                        Entity *newAst = FindFirstNullEntity(gameState->entities, ARRAY_COUNT(gameState->entities));
                                        if (newAst)
                                        {
                                            AsteroidSize newSize = (AsteroidSize)(oldSize + 1);
                                            *newAst = MakeEntity(Entity_Asteroids, true, oldPos, GetAsteroidDims(newSize));
                                            
                                            v2f baseDP = {};
                                            while (baseDP.x == 0.0f) { baseDP.x = (f32)rnd_pcg_range(&gameState->pcg, -1, 1); }
                                            while (baseDP.y == 0.0f) { baseDP.y = (f32)rnd_pcg_range(&gameState->pcg, -1, 1); }
                                            baseDP *= GetAsteroidBaseSpeed(newSize);
                                            
                                            newAst->dP = baseDP * (V2F(rnd_pcg_nextf(&gameState->pcg), rnd_pcg_nextf(&gameState->pcg)) + 0.5f);
                                            --asteroidsToCreate;
                                        }
                                        
                                    }
                                }
                                
                                break;
                            }
                        }
                    }
                    
                    MoveEntity(input, entity);
                    if (entity->pos.x < 0.0f || entity->pos.y < 0.0f ||
                        entity->pos.x > gameState->worldDims.x || entity->pos.y > gameState->worldDims.y)
                    {
                        *entity = NullEntity;
                        gameState->shotCount--;
                    }
                    else if (entity->type == Entity_Shot)
                    {
                        PushRect(renderGroup, entity->pos, entity->dims, 0.0f, V4F(0.0f, 1.0f, 0.0f, 1.0f));
                    }
                } break;
                
                case Entity_Asteroids:
                {
                    // TODO(bSalmon): Give the asteroids some random rotation
                    MoveEntityLoop(input, entity, gameState->worldDims);
                    
                    PushBitmap(renderGroup, &gameState->asteroidBitmaps[0], entity->pos, entity->dims, entity->angle, V4F(0.0f, 1.0f, 1.0f, 1.0f));
                    PushRect(renderGroup, entity->pos, V2F(1.0f), 0.0f, V4F(1.0f, 0.0f, 1.0f, 1.0f));
                } break;
                
                default: {} break;
            }
        }
    }
    
    char scoreText[16] = {};
    sprintf(scoreText, "%02d", gameState->score);
    
    RenderGroup *interfaceGroup = AllocateRenderGroup(&transState->transRegion, MEGABYTE(1), V2F(1.0f));
    PushText(interfaceGroup, scoreText, &gameState->gameFont, V2F(15.0f, 10.0f), TextAlign_TopLeft, 32.0f, V4F(1.0f), memory->platform);
    
    RenderToBuffer(backBuffer, renderGroup);
    RenderToBuffer(backBuffer, interfaceGroup);
    FinishTempMemory(renderMemory);
    
    // AUDIO
    //OutputTestSineWave(gameState, audioBuffer, 256);
    
    END_TIMED_BLOCK(GameUpdateRender);
    Debug_CycleCounters(memory);
}