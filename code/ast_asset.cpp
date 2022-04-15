/*
Project: Asteroids
File: ast_asset.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

struct LoadAssetData
{
    ParallelMemory *mem;
    PlatformAPI platform;
    
    Platform_FileHandle fileHandle;
    
    AssetLoadState volatile *state;
    
    usize size;
    usize offset;
    void **data;
};

function void LoadAssetWork(LoadAssetData *data)
{
    // TODO(bSalmon): Eventually use the transient mem instead of allocing for each asset
    *data->data = data->platform.MemAlloc(data->size);
    data->platform.ReadDataFromFile(&data->fileHandle, data->offset, data->size, *data->data);
    
#if 0
    // NOTE(bSalmon): Nothing wrong with this, but just ifed out until it's needed or until it can be logged properly
    if (!Platform_NoFileErrors(&data->fileHandle))
    {
        ASSERT(false);
    }
#endif
    
    *data->state = AssetLoad_Loaded;
}

function PARALLEL_WORK_CALLBACK(ParallelLoadAsset)
{
    LoadAssetData *loadData = (LoadAssetData *)data;
    LoadAssetWork(loadData);
    
    FinishParallelMemory(loadData->mem);
}

function LoadedAssetHeader *GetAsset(Game_LoadedAssets *loadedAssets, AssetType type, void *identifier, b32 immediate)
{
    DEBUG_TIMER_FUNC();
    
    LoadedAssetHeader *result = 0;
    
    for (u32 assetIndex = 0; assetIndex < loadedAssets->assetCount; ++assetIndex)
    {
        LoadedAssetHeader *currHeader = &loadedAssets->headers[assetIndex];
        switch (type)
        {
            case AssetType_Bitmap:
            {
                u32 id = *(u32 *)identifier;
                if (currHeader->type == type &&
                    currHeader->bitmap.id == id)
                {
                    result = currHeader;
                }
            } break;
            
            case AssetType_Glyph:
            {
                GlyphIdentifier id = *(GlyphIdentifier *)identifier;
                if (currHeader->type == type &&
                    currHeader->glyph.glyph == id.glyph &&
                    StringsAreSame(currHeader->glyph.font, id.font, StringLength(currHeader->glyph.font)))
                {
                    result = currHeader;
                }
            } break;
            
            case AssetType_KerningTable:
            {
                char *id = (char *)identifier;
                if (currHeader->type == type &&
                    StringsAreSame(currHeader->kerning.font, id, StringLength(currHeader->kerning.font)))
                {
                    result = currHeader;
                }
            } break;
            
            case AssetType_FontMetadata:
            {
                char *id = (char *)identifier;
                if (currHeader->type == type &&
                    StringsAreSame(currHeader->metadata.font, id, StringLength(currHeader->metadata.font)))
                {
                    result = currHeader;
                }
            } break;
            
            INVALID_DEFAULT;
        }
        
        if (result)
        {
            break;
        }
    }
    ASSERT(result);
    
    if (!result->asset && result->loadState == AssetLoad_Unloaded)
    {
        result->loadState = AssetLoad_Loading;
        if (immediate)
        {
            LoadAssetData loadData = {};
            loadData.platform = loadedAssets->platform;
            
            loadData.fileHandle = result->fileHandle;
            
            loadData.state = &result->loadState;
            
            loadData.size = result->assetSize;
            loadData.offset = result->assetOffset;
            loadData.data = &result->asset;
            
            LoadAssetWork(&loadData);
        }
        else
        {
            ParallelMemory *parallelMem = StartParallelMemory(loadedAssets->transState);
            if (parallelMem)
            {
                LoadAssetData *loadData = PushStruct(&parallelMem->memRegion, LoadAssetData);
                loadData->mem = parallelMem;
                loadData->platform = loadedAssets->platform;
                
                loadData->fileHandle = result->fileHandle;
                
                loadData->state = &result->loadState;
                
                loadData->size = result->assetSize;
                loadData->offset = result->assetOffset;
                loadData->data = &result->asset;
                
                loadedAssets->platform.AddParallelEntry(loadedAssets->parallelQueue, ParallelLoadAsset, loadData);
            }
            else
            {
                result->loadState = AssetLoad_Unloaded;
            }
        }
    }
    
    if (result->loadState == AssetLoad_Loaded && result->textureHandle == 0)
    {
        if (result->type == AssetType_Bitmap)
        {
            loadedAssets->platform.AllocTexture(&result->textureHandle, result->bitmap.dims.w, result->bitmap.dims.h, result->asset);
        }
        else if (result->type == AssetType_Glyph)
        {
            loadedAssets->platform.AllocTexture(&result->textureHandle, result->glyph.dims.w, result->glyph.dims.h, result->asset);
        }
        
    }
    
    return result;
}

inline KerningTable GetKerningTableFromAssetHeader(LoadedAssetHeader *assetHeader)
{
    KerningTable result = {};
    
    result.info = assetHeader->kerning;
    result.table = (Kerning *)assetHeader->asset;
    
    return result;
}

function Game_LoadedAssets InitialiseLoadedAssets(PlatformAPI platform, Platform_ParallelQueue *parallelQueue)
{
    Game_LoadedAssets result = {};
    result.platform = platform;
    result.parallelQueue = parallelQueue;
    
    Platform_FileGroup fileGroup = platform.GetAllFilesOfExtBegin(PlatformFileType_Asset);
    
    for (u32 i = 0; i < fileGroup.fileCount; ++i)
    {
        Platform_FileHandle fileHandle = platform.OpenNextFile(&fileGroup);
        AAFHeader fileHeader;
        platform.ReadDataFromFile(&fileHandle, 0, sizeof(AAFHeader), &fileHeader);
        ASSERT(fileHeader.magicValue == AAF_CODE('B', 'A', 'A', 'F'));
        
        result.assetCount += fileHeader.assetCount;
    }
    
    platform.GetAllFilesOfExtEnd(&fileGroup);
    
    fileGroup = platform.GetAllFilesOfExtBegin(PlatformFileType_Asset);
    
    result.headers = (LoadedAssetHeader *)platform.MemAlloc(sizeof(LoadedAssetHeader) * result.assetCount);
    u32 currAssetIndex = 0;
    for (u32 fileIndex = 0; fileIndex < fileGroup.fileCount; ++fileIndex)
    {
        Platform_FileHandle fileHandle = platform.OpenNextFile(&fileGroup);
        AAFHeader fileHeader;
        platform.ReadDataFromFile(&fileHandle, 0, sizeof(AAFHeader), &fileHeader);
        ASSERT(fileHeader.magicValue == AAF_CODE('B', 'A', 'A', 'F'));
        
        usize offset = sizeof(AAFHeader);
        for (u32 fileAssetIndex = 0; fileAssetIndex < fileHeader.assetCount; ++fileAssetIndex)
        {
            AssetHeader assetHeader;
            platform.ReadDataFromFile(&fileHandle, offset, sizeof(AssetHeader), &assetHeader);
            offset += sizeof(AssetHeader);
            
            LoadedAssetHeader *header = &result.headers[currAssetIndex++];
            header->fileHandle = fileHandle;
            
            header->type = assetHeader.type;
            header->loadState = AssetLoad_Unloaded;
            
            switch (assetHeader.type)
            {
                case AssetType_Bitmap:
                {
                    header->bitmap = assetHeader.bitmap;
                } break;
                
                case AssetType_Glyph:
                {
                    header->glyph = assetHeader.glyph;
                } break;
                
                case AssetType_KerningTable:
                {
                    header->kerning = assetHeader.kerning;
                } break;
                
                case AssetType_FontMetadata:
                {
                    header->metadata = assetHeader.metadata;
                } break;
                
                INVALID_DEFAULT;
            }
            
            header->assetSize = assetHeader.assetSize;
            header->assetOffset = offset;
            
            offset += assetHeader.assetSize;
        }
    }
    
    platform.GetAllFilesOfExtEnd(&fileGroup);
    
    return result;
}
