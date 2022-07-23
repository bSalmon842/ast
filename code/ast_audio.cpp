/*
Project: Asteroids
File: ast_audio.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#if 0
function void OutputTestSineWave(Game_Audio *audio)
{
    persist f32 tSine;
    
    s16 volume = 3000;
    s32 wavePeriod = audio->samplesPerSecond / 256;
    
    s16 *sampleOut = (s16 *)audio->samples;
    for (s32 sampleIndex = 0; sampleIndex < audio->sampleCount; ++sampleIndex)
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

function void OutputMixedAudio(Game_State *gameState, Game_Audio *audio)
{
    s16 *sampleOut = (s16 *)audio->samples;
    for (s32 sampleIndex = 0; sampleIndex < audio->sampleCount; ++sampleIndex)
    {
        s16 sampleValue = 0;
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
    }
}
#endif
