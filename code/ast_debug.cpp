/*
Project: Asteroids
File: ast_debug.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

function b32 DebugButton(Game_RenderCommands *commands, Game_LoadedAssets *loadedAssets, Game_Input *input, PlatformAPI platform, Camera camera, char *text, char *font, v2f textOrigin, f32 textScale, s32 zLayer, v4f textColour, v2f min, v2f max)
{
    b32 result = false;
    
    PushText(commands, loadedAssets, platform, camera, text, font, V3F(textOrigin, 0.0f),
             textScale, zLayer, textColour);
    
    v2f mousePos = V2F((f32)input->mouse.x, (f32)input->mouse.y);
    if (InputNoRepeat(input->mouse.buttons[MouseButton_L]))
    {
        if (mousePos > min && mousePos < max)
        {
            result = true;
        }
    }
    
    return result;
}

function void DisplayDebugMenu(Game_RenderCommands *commands, Game_LoadedAssets *loadedAssets, Game_Input *input, PlatformAPI platform, Camera camera, DebugSettings *debugSettings)
{
    f32 scale = 1.0f;
    v2f topLine = V2F(50.0f, (f32)commands->height - 100.0f);
    v2f debugLineOffset = topLine;
    
    LoadedAssetHeader *metadataHeader = GetAsset(commands->loadedAssets, AssetType_FontMetadata, "DebugLarge", true);
    FontMetadata metadata = metadataHeader->metadata;
    ASSERT(metadata.monospace);
    f32 lineHeight = metadata.lineGap * scale;
    
    v2f mousePos = V2F((f32)input->mouse.x, (f32)input->mouse.y);
    
    char *font = "DebugLarge";
    v4f textColour = V4F(1.0f, 0.5f, 0.5f, 1.0f);
    s32 selectedOption = -1;
    for (s32 optionIndex = 0; optionIndex < ARRAY_COUNT(debugSettings->options); ++optionIndex)
    {
        char *string = debugSettings->options[optionIndex];
        v2f lineMin = V2F(topLine.x, debugLineOffset.y);
        v2f lineMax = V2F(lineMin.x + (f32)(StringLength(string) * metadata.charWidth) * scale, debugLineOffset.y + lineHeight);
        
        if (mousePos > lineMin && mousePos < lineMax)
        {
            PushRect(commands, platform, camera, lineMin, lineMax, 0.0f, 0.0f, DEBUG_LAYER - 2, V4F(1.0f, 0.0f, 1.0f, 1.0f));
        }
        if (DebugButton(commands, loadedAssets, input, platform, camera, string, font, debugLineOffset,
                        scale, DEBUG_LAYER - 1, textColour, lineMin, lineMax))
        {
            // TODO(bSalmon): Bug with selected options flickering on hover without click
            selectedOption = optionIndex;
        }
        debugLineOffset.y -= lineHeight;
    }
    
    switch (selectedOption)
    {
        case -1:
        {
            // Do Nothing
        } break;
        
        case 0:
        {
            INVERT(debugSettings->timers);
        } break;
        
        case 1:
        {
            INVERT(debugSettings->colliders);
        } break;
        
        case 2:
        {
            INVERT(debugSettings->regions);
        } break;
        
        case 3:
        {
            INVERT(debugSettings->camMove);
        } break;
        
        case 4:
        {
            INVERT(debugSettings->zoom);
        } break;
        
        case 5:
        {
            INVERT(debugSettings->mouseInfo);
        } break;
        
        INVALID_DEFAULT;
    }
}

function void PrintDebugRecords(Game_Memory *memory, Game_RenderCommands *commands, Game_LoadedAssets *loadedAssets, Game_Input *input, Camera camera, PlatformAPI platform)
{
    DebugState *debugState = (DebugState *)memory->debugStorage;
    if (debugState)
    {
        f32 scale = DEBUG_TEXT_SCALE;
        char *font = "Debug";
        
        if (globalDebugState->settings.movingTimerWindow)
        {
            globalDebugState->settings.timerWindowPosY = (f32)input->mouse.y;
        }
        
        v2f topLine = V2F(20.0f, globalDebugState->settings.timerWindowPosY);
        v2f debugLineOffset = topLine;
        
        LoadedAssetHeader *metadataHeader = GetAsset(commands->loadedAssets, AssetType_FontMetadata, font, true);
        FontMetadata metadata = metadataHeader->metadata;
        f32 lineHeight = metadata.lineGap * scale;
        
        char *title = "Debug Perf Metrics";
        PushText(commands, loadedAssets, platform, camera, title, font, V3F(debugLineOffset, 0.0f), scale, DEBUG_LAYER, V4F(1.0f));
        debugLineOffset.y -= lineHeight;
        
        v2f mousePos = V2F((f32)input->mouse.x, (f32)input->mouse.y);
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
            char string[128];
            DebugBlockInfo *lastBlockInfo = &globalDebugState->table->lastBlockInfos[hoveredTIndex][hoveredBIndex];
            DebugBlockStats *lastBlockStat = &globalDebugState->table->lastBlockStats[hoveredTIndex][hoveredBIndex];
            stbsp_sprintf(string, "%s:\nMin: %10lluc | Max: %10lluc\nMin: %10uh | Max: %10uh",
                          lastBlockInfo->name, lastBlockStat->minCycles, lastBlockStat->maxCycles, lastBlockStat->minHits, lastBlockStat->maxHits);
            PushTooltip(commands, loadedAssets, platform, camera, string, font, scale, mousePos, DEBUG_LAYER + 1);
        }
        
        v2f min = V2F(10.0f, debugLineOffset.y);
        v2f max = V2F((f32)commands->width - 10.0f, topLine.y + (metadata.lineGap * scale));
        PushRect(commands, platform, camera, min, max, 0.0f, 0.0f, DEBUG_LAYER - 2, V4F(0.05f, 0.0f, 0.1f, 0.66f));
        
        if (mousePos > min && mousePos < max && input->mouse.buttons[MouseButton_R].endedFrameDown)
        {
            globalDebugState->settings.movingTimerWindow = true;
        }
        else
        {
            globalDebugState->settings.movingTimerWindow = false;
        }
    }
}
