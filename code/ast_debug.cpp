/*
Project: Asteroids
File: ast_debug.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

function void PrintDebugRecords(Game_Memory *memory, Game_RenderCommands *commands, Game_LoadedAssets *loadedAssets, Game_Input *input, Camera camera, PlatformAPI platform)
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
        PushText(commands, loadedAssets, platform, camera, title, font, V3F(debugLineOffset, 0.0f), scale, DEBUG_LAYER, V4F(1.0f));
        debugLineOffset.y -= lineHeight;
        
        v2f mousePos = V2F((f32)input->mouseX, (f32)input->mouseY);
        s16 hoveredTIndex = -1;
        s16 hoveredBIndex = -1;
        for (u32 translationIndex = 0; translationIndex < TRANSLATION_UNIT_COUNT; ++translationIndex)
        {
            for (u32 blockIndex = 0; blockIndex < MAX_DEBUG_TRANSLATION_UNIT_INFOS; ++blockIndex)
            {
                DebugBlockInfo *lastBlockInfo = &globalDebugState->table->lastBlockInfos[translationIndex][blockIndex];
                if (lastBlockInfo->hits)
                {
                    v2f lineMin = V2F(10.0f, debugLineOffset.y);
                    v2f lineMax = V2F((f32)commands->width - 10.0f, debugLineOffset.y + lineHeight);
                    if (mousePos > lineMin && mousePos < lineMax)
                    {
                        hoveredTIndex = (s16)translationIndex;
                        hoveredBIndex = (s16)blockIndex;
                    }
                    
                    char string[128];
                    stbsp_sprintf(string, "%24s(%u): %10lluc | %6u hits | %10lluc/hit",
                                  lastBlockInfo->name, translationIndex, lastBlockInfo->cycles, lastBlockInfo->hits, lastBlockInfo->cycles / lastBlockInfo->hits);
                    PushText(commands, loadedAssets, platform, camera, string, font, V3F(debugLineOffset, 0.0f), scale, DEBUG_LAYER, V4F(1.0f));
                    debugLineOffset.y -= lineHeight;
                }
            }
        }
        
        if (hoveredTIndex != -1 && hoveredBIndex != -1)
        {
            v2f hoverDims = V2F(450.0f, 40.0f);
            
            v2f hoverMax = V2F();(mousePos - V2F(10.0f));
            v2f hoverMin = V2F();hoverMax - hoverDims;
            if (mousePos.x <= commands->width / 2)
            {
                hoverMin = V2F(mousePos.x + 10.0f, mousePos.y - hoverDims.y);
                hoverMax = hoverMin + hoverDims;
            }
            else
            {
                hoverMax = (mousePos - V2F(10.0f));
                hoverMin = hoverMax - hoverDims;
            }
            PushRect(commands, platform, camera, hoverMin, hoverMax, 0.0f, 0.0f, DEBUG_LAYER + 1, V4F(V3F(0.1f), 0.66f));
            DebugBlockInfo *lastBlockInfo = &globalDebugState->table->lastBlockInfos[hoveredTIndex][hoveredBIndex];
            DebugBlockStats *lastBlockStat = &globalDebugState->table->lastBlockStats[hoveredTIndex][hoveredBIndex];
            
            v2f hoverLineOffset = V2F(hoverMin.x + 10.0f, hoverMax.y - lineHeight);
            char string[128];
            stbsp_sprintf(string, "%s:\nMin: %10lluc | Max: %10lluc\nMin: %10uh | Max: %10uh",
                          lastBlockInfo->name, lastBlockStat->minCycles, lastBlockStat->maxCycles, lastBlockStat->minHits, lastBlockStat->maxHits);
            PushText(commands, loadedAssets, platform, camera, string, font, V3F(hoverLineOffset, 0.0f), scale, DEBUG_LAYER + 2, V4F(1.0f));
        }
        
        v2f min = V2F(10.0f, debugLineOffset.y);
        v2f max = V2F((f32)commands->width - 10.0f, topLine.y + (metadata.lineGap * scale));
        PushRect(commands, platform, camera, min, max, 0.0f, 0.0f, DEBUG_LAYER - 2, V4F(0.05f, 0.0f, 0.1f, 0.66f));
    }
}
