/*
Project: Asteroids
File: ast_debug.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

function void WriteDebugConfigFile(PlatformAPI platform, DebugConfig config)
{
    Platform_FileHandle fileHandle = platform.OpenFileForWrite("..\\code\\ast_debug_config.h", PlatformWriteType_Overwrite);
    platform.WriteIntoFile(fileHandle, "#define DEBUGUI_FUNC_TIMERS %d\n", config.funcTimers);
    platform.WriteIntoFile(fileHandle, "#define DEBUGUI_FRAME_TIMERS %d\n", config.frameTimers);
    platform.WriteIntoFile(fileHandle, "#define DEBUGUI_RENDER_TIMING %d\n", config.renderTiming);
    platform.WriteIntoFile(fileHandle, "#define DEBUGUI_MEMORY_VIS %d\n", config.memoryVis);
    platform.WriteIntoFile(fileHandle, "#define DEBUGUI_ENTITY_COLLIDERS %d\n", config.entityColliders);
    platform.WriteIntoFile(fileHandle, "#define DEBUGUI_PARTICLE_COLLIDERS %d\n", config.particleColliders);
    platform.WriteIntoFile(fileHandle, "#define DEBUGUI_REGIONS %d\n", config.regions);
    platform.WriteIntoFile(fileHandle, "#define DEBUGUI_CAMMOVE %d\n", config.camMove);
    platform.WriteIntoFile(fileHandle, "#define DEBUGUI_CAMZOOM %d\n", config.camZoom);
    platform.WriteIntoFile(fileHandle, "#define DEBUGUI_MOUSEINFO %d\n", config.mouseInfo);
    platform.WriteIntoFile(fileHandle, "#define DEBUGUI_PICKING %d\n", config.picking);
    platform.CloseFile(&fileHandle);
}

inline DebugMenuItem *AddNextDebugMenuItem(MemoryRegion *memRegion, DebugMenuItem *prev, char *name, DebugMenuFunctionType funcType, void *use, b32 isChild = false)
{
    DebugMenuItem *newItem = PushStruct(memRegion, DebugMenuItem);
    newItem->next = 0;
    newItem->child = 0;
    newItem->isOpen = false;
    newItem->funcType = funcType;
    newItem->use = use;
    
    char *src = name;
    u8 destIndex = 0;
    while (*src)
    {
        newItem->name[destIndex++] = *src++;
    }
    
    if (isChild)
    {
        prev->child = newItem;
    }
    else
    {
        prev->next = newItem;
    }
    
    return newItem;
}

global v4f debugColourTableA[6] = 
{
    V4F(1.0f, 0.5f, 0.5f, 1.0f),
    V4F(0.5f, 1.0f, 0.5f, 1.0f),
    V4F(0.5f, 0.5f, 1.0f, 1.0f),
    V4F(1.0f, 1.0f, 0.5f, 1.0f),
    V4F(1.0f, 0.5f, 1.0f, 1.0f),
    V4F(0.5f, 1.0f, 1.0f, 1.0f),
};

global v4f debugColourTableB[12] = 
{
    V4F(1.0f, 0.0f, 0.0f, 1.0f),
    V4F(0.0f, 1.0f, 0.0f, 1.0f),
    V4F(0.0f, 0.0f, 1.0f, 1.0f),
    V4F(1.0f, 1.0f, 0.0f, 1.0f),
    V4F(1.0f, 0.0f, 1.0f, 1.0f),
    V4F(0.0f, 1.0f, 1.0f, 1.0f),
    V4F(1.0f, 0.5f, 0.0f, 1.0f),
    V4F(1.0f, 0.0f, 0.5f, 1.0f),
    V4F(0.5f, 1.0f, 0.0f, 1.0f),
    V4F(0.0f, 1.0f, 0.5f, 1.0f),
    V4F(0.5f, 0.0f, 1.0f, 1.0f),
    V4F(0.0f, 0.5f, 1.0f, 1.0f),
};

inline void HandleDebugMenuItem(DebugMenuItem *item, Game_RenderCommands *commands, Game_LoadedAssets *loadedAssets, Game_Input *input, PlatformAPI platform, Camera camera, DebugSettings *debugSettings, v2f debugLineOffset, f32 scale, f32 lineHeight, FontMetadata metadata, char *font, v4f textColour)
{
    v2f itemMin = debugLineOffset;
    v2f itemMax = V2F(itemMin.x + (f32)(StringLength(item->name) * metadata.charWidth) * scale, debugLineOffset.y + lineHeight);
    
    if (DebugButton(commands, loadedAssets, input, platform, camera, item->name, font, debugLineOffset,
                    scale, DEBUG_LAYER - 1, textColour, itemMin, itemMax))
    {
        INVERT(item->isOpen);
        if (item->use)
        {
            switch (item->funcType)
            {
                case DebugMenuFuncType_b32:
                {
                    INVERT(*((b32 *)item->use));
                    debugSettings->configChanged = true;
                } break;
                
                INVALID_DEFAULT;
            }
        }
    }
}

function void DisplayDebugMenuChildLayer(DebugMenuItem *parent, Game_RenderCommands *commands, Game_LoadedAssets *loadedAssets, Game_Input *input, PlatformAPI platform, Camera camera, DebugSettings *debugSettings, v2f *debugLineOffset, f32 scale, f32 lineHeight,  FontMetadata metadata, char *font, u32 *debugColourIndex)
{
    v4f textColour = debugColourTableA[(*debugColourIndex)++ % ARRAY_COUNT(debugColourTableA)];
    if (parent->isOpen && parent->child)
    {
        debugLineOffset->x += 20.0f;
        
        for (DebugMenuItem *child = parent->child; child; child = child->next)
        {
            HandleDebugMenuItem(child, commands, loadedAssets, input, platform, camera, debugSettings, *debugLineOffset, scale, lineHeight, metadata, font, textColour);
            
            debugLineOffset->y -= lineHeight;
            
            DisplayDebugMenuChildLayer(child, commands, loadedAssets, input, platform, camera, debugSettings, debugLineOffset, scale, lineHeight, metadata, font, debugColourIndex);
        }
        
        debugLineOffset->x -= 20.0f;
        
    }
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
    u32 debugColourIndex = 0;
    v4f textColour = debugColourTableA[debugColourIndex++ % ARRAY_COUNT(debugColourTableA)];
    
    for (DebugMenuItem *item = debugSettings->menuSentinel.next; item; item = item->next)
    {
        HandleDebugMenuItem(item, commands, loadedAssets, input, platform, camera, debugSettings, debugLineOffset, scale, lineHeight, metadata, font, textColour);
        
        debugLineOffset.y -= lineHeight;
        
        DisplayDebugMenuChildLayer(item, commands, loadedAssets, input, platform, camera, debugSettings, &debugLineOffset, scale, lineHeight, metadata, font, &debugColourIndex);
    }
}

function void DisplayFunctionTimers(Game_Memory *memory, Game_RenderCommands *commands, Game_LoadedAssets *loadedAssets, Game_Input *input, Camera camera, PlatformAPI platform)
{
    DebugState *debugState = (DebugState *)memory->storage[DEBUG_STORAGE_INDEX].ptr;
    if (debugState)
    {
        u32 frameIndex = globalDebugState->table->frameIndex - 1;
        if (frameIndex > (MAX_DEBUG_FRAMES - 1))
        {
            frameIndex = (MAX_DEBUG_FRAMES - 1);
        }
        DebugFrame *lastFrame = &globalDebugState->table->frames[frameIndex];
        
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
                DebugBlockInfo *lastBlockInfo = &lastFrame->blockInfos[translationIndex][blockIndex];
                if (lastBlockInfo->hits && !StringsAreSame(lastBlockInfo->name, "Render_", 7))
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
            DebugBlockInfo *lastBlockInfo = &lastFrame->blockInfos[hoveredTIndex][hoveredBIndex];
            DebugBlockStats *lastBlockStat = &globalDebugState->table->blockStats[hoveredTIndex][hoveredBIndex];
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

function void DisplayFrameTimers(Game_Memory *memory, Game_RenderCommands *commands, Game_LoadedAssets *loadedAssets, Game_Input *input, Camera camera, PlatformAPI platform)
{
    DebugState *debugState = (DebugState *)memory->storage[DEBUG_STORAGE_INDEX].ptr;
    if (debugState)
    {
        v2f mousePos = V2F((f32)input->mouse.x, (f32)input->mouse.y);
        f32 barGap = 2.0f;
        f32 barWidth = 5.0f;
        f32 targetTime = 1.0f / 60.0f;
        f32 chartHeight = 250.0f;
        v2f chartMin = V2F(20.0f);
        v2f chartMax = V2F((f32)commands->width - 20.0f, chartMin.y + chartHeight);
        
        PushRect(commands, platform, camera, V2F(chartMin.x, chartMax.y - 1), chartMax, 0.0f, 0.0f, DEBUG_LAYER, V4F(1.0f));
        
        v2f barMin = chartMin;
        
        for (u32 frameIndex = 0; frameIndex < MAX_DEBUG_FRAMES; ++frameIndex)
        {
            DebugFrame *frame = &globalDebugState->table->frames[frameIndex];
            f32 timeFactor = frame->frameTime / targetTime;
            f32 totalBarHeight = timeFactor * chartHeight;
            u64 remainingClocks = frame->totalClock;
            
            u8 debugColourIndex = 0;
            v2f segmentMin = barMin;
            for (u32 translationIndex = 0; translationIndex < TRANSLATION_UNIT_COUNT; ++translationIndex)
            {
                for (u32 blockIndex = 0; blockIndex < MAX_DEBUG_TRANSLATION_UNIT_INFOS; ++blockIndex)
                {
                    DebugBlockInfo *blockInfo = &frame->blockInfos[translationIndex][blockIndex];
                    if (blockInfo->isMarker)
                    {
                        f32 segPct = ((f32)blockInfo->cycles / (f32)frame->totalClock);
                        f32 segmentHeight = segPct * totalBarHeight;
                        v4f segmentColour = debugColourTableB[debugColourIndex++];
                        v2f segmentMax = segmentMin + V2F(barWidth, segmentHeight);
                        
                        PushRect(commands, platform, camera, segmentMin, segmentMax, 0.0f, 0.0f, DEBUG_LAYER - 1, segmentColour);
                        
                        if (mousePos > segmentMin && mousePos < segmentMax)
                        {
                            char string[128] = {};
                            stbsp_sprintf(string, "Seg Name: %s\nSeg Clock: %llu\nFrame Pct: %.03f", blockInfo->name, blockInfo->cycles, segPct);
                            PushTooltip(commands, loadedAssets, platform, camera, string, "Debug", 1.0f, mousePos, DEBUG_LAYER + 1);
                        }
                        
                        segmentMin.y += segmentHeight;
                        ASSERT(remainingClocks - blockInfo->cycles < remainingClocks);
                        remainingClocks -= blockInfo->cycles;
                    }
                }
            }
            
            if (remainingClocks > 0)
            {
                v2f barMax = barMin + V2F(barWidth, totalBarHeight);
                PushRect(commands, platform, camera, segmentMin, barMax, 0.0f, 0.0f, DEBUG_LAYER - 1, V4F(1.0f));
            }
            
            barMin.x += barWidth + barGap;
        }
        
    }
}

function void DisplayPickedEntityInfo(Game_Memory *memory, Game_RenderCommands *commands, Game_LoadedAssets *loadedAssets, Camera camera, PlatformAPI platform, Entity *entities)
{
    DebugState *debugState = (DebugState *)memory->storage[DEBUG_STORAGE_INDEX].ptr;
    if (debugState)
    {
        s32 entityIndex = globalDebugState->settings.pickedEntityIndex;
        v2f lineOffset = V2F(800.0f, 200.0f);
        PushText(commands, loadedAssets, platform, camera, "Picked Entity Info", "Debug", V3F(800.0f, 200.0f, 0.0f), DEBUG_TEXT_SCALE, DEBUG_LAYER, V4F(0.0f, 1.0f, 0.0f, 1.0f));
        {
            if (entityIndex != -1)
            {
                char string[128];
                stbsp_sprintf(string, "Index: %d\nType: %d\nPos: {%.02f, %.02f}\n", entityIndex, entities[entityIndex].type, entities[entityIndex].pos.x, entities[entityIndex].pos.y);
                PushText(commands, loadedAssets, platform, camera, string, "Debug", V3F(800.0f, 180.0f, 0.0f), DEBUG_TEXT_SCALE, DEBUG_LAYER, V4F(0.0f, 1.0f, 0.0f, 1.0f));
            }
            else
            {
                PushText(commands, loadedAssets, platform, camera, "Invalid Entity Index", "Debug", V3F(800.0f, 180.0f, 0.0f), DEBUG_TEXT_SCALE, DEBUG_LAYER, V4F(0.0f, 1.0f, 0.0f, 1.0f));
            }
        }
    }
}

function void DisplayRenderTiming(Game_Memory *memory, Game_RenderCommands *commands, Game_LoadedAssets *loadedAssets, Game_Input *input, Camera camera, PlatformAPI platform)
{
    DebugState *debugState = (DebugState *)memory->storage[DEBUG_STORAGE_INDEX].ptr;
    if (debugState)
    {
        u32 frameIndex = globalDebugState->table->frameIndex - 1;
        if (frameIndex > (MAX_DEBUG_FRAMES - 1))
        {
            frameIndex = (MAX_DEBUG_FRAMES - 1);
        }
        DebugFrame *lastFrame = &globalDebugState->table->frames[frameIndex];
        
        v2f mousePos = V2F((f32)input->mouse.x, (f32)input->mouse.y);
        
        f32 totalBarWidth = (f32)commands->width - 40.0f;
        f32 barHeight = 100.0f;
        v2f barMin = V2F(20.0f, 300.0f);
        v2f barMax = barMin + V2F(totalBarWidth, barHeight);
        
        u64 totalCycles = 0;
        
        for (u32 blockIndex = 0; blockIndex < MAX_DEBUG_TRANSLATION_UNIT_INFOS; ++blockIndex)
        {
            DebugBlockInfo *blockInfo = &lastFrame->blockInfos[1][blockIndex];
            if (blockInfo->hits)
            {
                if (StringsAreSame(blockInfo->name, "RenderTotal", 11))
                {
                    totalCycles = blockInfo->cycles;
                    break;
                }
            }
        }
        
        u8 debugColourIndex = 0;
        v2f segmentMin = barMin;
        for (u32 blockIndex = 0; blockIndex < MAX_DEBUG_TRANSLATION_UNIT_INFOS; ++blockIndex)
        {
            DebugBlockInfo *blockInfo = &lastFrame->blockInfos[1][blockIndex];
            if (blockInfo->hits && StringsAreSame(blockInfo->name, "Render_", 7))
            {
                f32 segPct = ((f32)blockInfo->cycles / (f32)totalCycles);
                f32 segmentWidth = segPct * totalBarWidth;
                v2f segmentMax = segmentMin + V2F(segmentWidth, barHeight);
                v4f segmentColour = debugColourTableB[debugColourIndex++];
                
                PushRect(commands, platform, camera, segmentMin, segmentMax, 0.0f, 0.0f, DEBUG_LAYER - 1, segmentColour);
                
                if (mousePos > segmentMin && mousePos < segmentMax)
                {
                    char string[128] = {};
                    stbsp_sprintf(string, "Seg Name: %s\nSeg Clock: %llu\nFrame Pct: %.03f", blockInfo->name, blockInfo->cycles, segPct);
                    PushTooltip(commands, loadedAssets, platform, camera, string, "Debug", 1.0f, mousePos, DEBUG_LAYER + 1);
                }
                
                segmentMin.x += segmentWidth;
            }
        }
        
        PushRect(commands, platform, camera, segmentMin, barMax, 0.0f, 0.0f, DEBUG_LAYER - 1, V4F(1.0f));
    }
}

function void DisplayMemoryVis(Game_Memory *memory, Game_RenderCommands *commands, Game_LoadedAssets *loadedAssets, Game_Input *input, Camera camera, PlatformAPI platform)
{
    DebugState *debugState = (DebugState *)memory->storage[DEBUG_STORAGE_INDEX].ptr;
    if (debugState)
    {
        v2f mousePos = V2F((f32)input->mouse.x, (f32)input->mouse.y);
        
        DebugDiscreteSlider(commands, platform, camera, input, &debugState->settings.memZoomMin, &debugState->settings.memZoomMax, 20.0f, (f32)commands->width - 20.0f, 575.0f, 10.0f, DEBUG_LAYER - 2);
        f32 sliderWidth = debugState->settings.memZoomMax - debugState->settings.memZoomMin;
        
        f32 totalBarWidth = (f32)commands->width - 40.0f;
        f32 barHeight = 40.0f;
        f32 barGap = 10.0f;
        v2f barMin = V2F(20.0f, 500.0f);
        v2f barMax = barMin + V2F(totalBarWidth, barHeight);
        
        for (s32 storageIndex = 0; storageIndex < STORAGE_COUNT; ++storageIndex)
        {
            u32 debugColourIndex = 0;
            
            StorageInfo *storage = &memory->storage[storageIndex];
            u64 totalSize = storage->size;
            
            PushRect(commands, platform, camera, barMin, barMax, 0.0f, 0.0f, DEBUG_LAYER - 2, V4F(V3F(0.5f), 1.0f));
            
            usize stateSize = 0;
            if (storageIndex == PERMA_STORAGE_INDEX) { stateSize = sizeof(Game_State); }
            else if (storageIndex == TRANS_STORAGE_INDEX) { stateSize = sizeof(Transient_State); }
            else if (storageIndex == DEBUG_STORAGE_INDEX) { stateSize = sizeof(DebugState); }
            ASSERT(stateSize);
            
            u32 tooltipRegionIndex = storage->regionCount + 1;
            f32 statePct = (f32)stateSize / (f32)totalSize;
            if (statePct > debugState->settings.memZoomMin)
            {
                f32 stateWidth = MAX(((statePct - debugState->settings.memZoomMin) / sliderWidth) * totalBarWidth, 1.0f);
                v2f stateMax = barMin + V2F(stateWidth, barHeight);
                PushRect(commands, platform, camera, barMin, stateMax, 0.0f, 0.0f, DEBUG_LAYER - 1, debugColourTableB[debugColourIndex % ARRAY_COUNT(debugColourTableB)]);
                
                if (mousePos > barMin && mousePos < stateMax)
                {
                    tooltipRegionIndex = storage->regionCount;
                }
            }
            debugColourIndex++;
            
            for (u32 regionIndex = 0; regionIndex < storage->regionCount; ++regionIndex)
            {
                MemoryRegion *region = storage->regions[regionIndex];
                f32 regionWidthPct = (f32)region->size / (f32)totalSize;
                f32 segBeginPct = (f32)(region->base - (u8 *)storage->ptr) / (f32)totalSize;
                if (segBeginPct + regionWidthPct > debugState->settings.memZoomMin)
                {
                    f32 segMinPct = (segBeginPct - debugState->settings.memZoomMin) / sliderWidth;
                    f32 segWidthPct = (regionWidthPct / sliderWidth) + MIN(segMinPct, 0.0f);
                    f32 segWidth = segWidthPct * totalBarWidth;
                    v2f segMin = V2F(barMin.x + (MAX(segMinPct, 0.0f) * totalBarWidth), barMin.y);
                    v2f segMax = segMin + V2F(MIN(segWidth, barMax.x - segMin.x), barHeight);
                    
                    PushRect(commands, platform, camera, segMin, segMax, 0.0f, 0.0f, DEBUG_LAYER - 1, debugColourTableB[debugColourIndex % ARRAY_COUNT(debugColourTableB)]);
                    
                    if (mousePos > segMin && mousePos < segMax)
                    {
                        tooltipRegionIndex = regionIndex;
                    }
                }
                
                debugColourIndex++;
            }
            
            if (tooltipRegionIndex != storage->regionCount + 1)
            {
                char string[128] = {};
                if (tooltipRegionIndex == storage->regionCount)
                {
                    switch (storageIndex)
                    {
                        case PERMA_STORAGE_INDEX: { stbsp_sprintf(string, "Region Name: Game_State\nRegion Size: %llu", sizeof(Game_State)); } break;
                        case TRANS_STORAGE_INDEX: { stbsp_sprintf(string, "Region Name: Transient_State\nRegion Size: %llu", sizeof(Transient_State)); } break;
                        case DEBUG_STORAGE_INDEX: { stbsp_sprintf(string, "Region Name: DebugState\nRegion Size: %llu", sizeof(DebugState)); } break;
                        
                        INVALID_DEFAULT;
                    }
                }
                else
                {
                    MemoryRegion *region = storage->regions[tooltipRegionIndex];
                    stbsp_sprintf(string, "Region Name: %s\nRegion Size: %llu\nRegion Used: %llu", region->name, region->size, region->used);
                }
                PushTooltip(commands, loadedAssets, platform, camera, string, "Debug", 1.0f, mousePos, DEBUG_LAYER + 1);
            }
            
            barMin.y -= (barHeight + barGap);
            barMax.y -= (barHeight + barGap);
        }
    }
}
