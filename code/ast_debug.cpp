/*
Project: Asteroids
File: ast_debug.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

inline void StartDebugStat(DebugStat *stat)
{
    stat->min = FLT_MAX;
    stat->max = -FLT_MAX;
    stat->ave = 0.0;
    stat->count = 0;
}

inline void CalcDebugStat(DebugStat *stat, f64 value)
{
    ++stat->count;
    
    if (stat->min > value)
    {
        stat->min = value;
    }
    
    if (stat->max < value)
    {
        stat->max = value;
    }
    
    stat->ave += value;
}

inline void FinishDebugStat(DebugStat *stat)
{
    if (stat->count)
    {
        stat->ave /= (f64)stat->count;
    }
    else
    {
        stat->min = 0.0;
        stat->max = 0.0;
    }
}

function void PrintDebugRecords(Game_Memory *memory, Game_RenderCommands *commands, Camera camera, PlatformAPI platform)
{
    DebugState *debugState = (DebugState *)memory->debugStorage;
    if (debugState)
    {
        f32 scale = DEBUG_TEXT_SCALE;
        char *font = "Debug";
        
        v2f topLine = V2F(20.0f, (f32)commands->height - 100.0f);
        v2f debugLineOffset = topLine;
        
        LoadedAssetHeader *metadataHeader = GetAsset(commands->loadedAssets, AssetType_FontMetadata, font, true);
        FontMetadata metadata = metadataHeader->metadata;
        f32 lineHeight = metadata.lineGap * scale;
        
        char *title = "Debug Perf Metrics";
        PushText(commands, platform, camera, title, font, V3F(debugLineOffset, 0.0f), scale, DEBUG_LAYER, V4F(1.0f));
        debugLineOffset.y -= lineHeight;
        
        for (u32 ctrIndex = 0; ctrIndex < debugState->counterCount; ++ctrIndex)
        {
            DebugCounter *counter = &debugState->counters[ctrIndex];
            
            DebugStat hitStat, cycleStat, cphStat;
            StartDebugStat(&hitStat);
            StartDebugStat(&cycleStat);
            StartDebugStat(&cphStat);
            for (u32 datumIndex = 0; datumIndex < DEBUG_DATUM_COUNT; ++datumIndex)
            {
                CalcDebugStat(&hitStat, counter->data[datumIndex].hits);
                CalcDebugStat(&cycleStat, counter->data[datumIndex].cycles);
                
                f64 cyclesPerHit = 0.0f;
                if (counter->data[datumIndex].hits)
                {
                    cyclesPerHit = ((f64)counter->data[datumIndex].cycles / (f64)counter->data[datumIndex].hits);
                }
                CalcDebugStat(&cphStat, cyclesPerHit);
            }
            FinishDebugStat(&hitStat);
            FinishDebugStat(&cycleStat);
            FinishDebugStat(&cphStat);
            
            if (hitStat.max > 0.0f)
            {
                char string[128];
                stbsp_sprintf(string, "%24s: %10uc | %6u hits | %10uc/hit",
                              counter->functionName, (u32)cycleStat.ave, (u32)hitStat.ave, (u32)cphStat.ave);
                PushText(commands, platform, camera, string, font, V3F(debugLineOffset, 0.0f), scale, DEBUG_LAYER, V4F(1.0f));
                
                f32 chartWidth = (f32)commands->width * 0.25f;
                f32 datumWidth = ((1.0f / (f32)DEBUG_DATUM_COUNT) * chartWidth);
                for (u32 datumIndex = 0; datumIndex < DEBUG_DATUM_COUNT; ++datumIndex)
                {
                    f32 minX = ((f32)commands->width * 0.725f) + (datumWidth * datumIndex);
                    f32 maxX = minX + datumWidth;
                    f32 chartScale = (f32)counter->data[datumIndex].cycles / (f32)cycleStat.max;
                    
                    v2f chartMin = V2F(minX, debugLineOffset.y);
                    v2f chartMax = V2F(maxX, chartMin.y + (lineHeight * chartScale));
                    v4f datumColour = V4F(chartScale, 1.0f - chartScale, 0.0f, 1.0f);
                    PushRect(commands, platform, camera, chartMin, chartMax, 0.0f, 0.0f, DEBUG_LAYER - 1, datumColour);
                }
                
                debugLineOffset.y -= lineHeight;
            }
        }
        
        for (u32 frameIndex = 0; frameIndex < DEBUG_DATUM_COUNT; ++frameIndex)
        {
            DebugFrameInfo *frame = &debugState->frames[frameIndex];
            f32 prevTime = 0.0f;
            for (u32 timestampIndex = 0; timestampIndex < frame->timestampCount; ++timestampIndex)
            {
                DebugFrameTimestamp *timestamp = &frame->timestamps[timestampIndex];
                f32 timeElapsed = timestamp->time - prevTime;
                prevTime = timestamp->time;
            }
        }
        
        v2f min = V2F(10.0f, debugLineOffset.y);
        v2f max = V2F((f32)commands->width - 10.0f, topLine.y + (metadata.lineGap * scale));
        PushRect(commands, platform, camera, min, max, 0.0f, 0.0f, DEBUG_LAYER - 2, V4F(0.05f, 0.0f, 0.1f, 0.66f));
    }
}

function void UpdateDebugRecords(DebugState *debugState, DebugRecord *records, u32 recordCount)
{
    for (u32 recordIndex = 0; recordIndex < recordCount; ++recordIndex)
    {
        DebugRecord *srcRecord = &records[recordIndex];
        DebugCounter *destCounter = &debugState->counters[debugState->counterCount++];
        
        u64 counts = AtomicExchange(&srcRecord->counts, 0);
        destCounter->fileName = srcRecord->fileName;
        destCounter->functionName = srcRecord->functionName;
        destCounter->lineNumber = srcRecord->lineNumber;
        destCounter->data[debugState->datumIndex].hits = (u32)(counts >> 32);
        destCounter->data[debugState->datumIndex].cycles = (u32)(counts & 0xFFFFFFFF);
    }
}
