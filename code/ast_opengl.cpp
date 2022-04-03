/*
Project: Asteroids
File: ast_opengl.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#include "ast.h"
#include "ast_utility.h"
#include "ast_math.h"
#include "ast_memory.h"
#include "ast_asset.h"
#include "ast_render.h"

#include "ast_memory.cpp"
#include "ast_asset.cpp"

// NOTE(bSalmon): Universal GL defines
#define GL_NUM_EXTENSIONS           0x821D
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_SRGB8_ALPHA8             0x8C43
#define GL_FRAMEBUFFER_SRGB         0x8DB9

struct OpenGL_Info
{
    char *vendor;
    char *renderer;
    char *version;
    char *glslVersion;
    char *extensions;
    
    type_glGetStringi *glGetStringi;
    
    b32 support_SRGB;
    b32 ARB_framebuffer_object;
};

function PLATFORM_ALLOC_TEXTURE(WGL_AllocTexture)
{
    glGenTextures(1, handle);
    glBindTexture(GL_TEXTURE_2D, *handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data);
    if (glGetError() != 0)
    {
        printf("glTexImage2D errored: 0x%x\n", glGetError());
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

function PLATFORM_FREE_TEXTURE(WGL_FreeTexture)
{
    GLuint handle = (GLuint)textureHandle;
    glDeleteTextures(1, &handle);
}

function void OpenGL_GetInfo(OpenGL_Info *glInfo)
{
    glInfo->vendor = (char *)glGetString(GL_VENDOR);
    glInfo->renderer = (char *)glGetString(GL_RENDERER);
    glInfo->version = (char *)glGetString(GL_VERSION);
    glInfo->glslVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    
    if (glInfo->glGetStringi)
    {
        s32 extCount = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &extCount);
        
        for (s32 extIndex = 0; extIndex < extCount; ++extIndex)
        {
            char *ext = (char *)glInfo->glGetStringi(GL_EXTENSIONS, extIndex);
            
            if (StringsAreSame(ext, "GL_ARB_framebuffer_object", StringLength(ext))) { glInfo->ARB_framebuffer_object = true; }
        }
    }
}

function void OpenGL_Init(OpenGL_Info *info)
{
    if (info->support_SRGB)
    {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
}

inline void OpenGL_Rectangle(v2f min, v2f max, v4f colour = V4F(1.0f))
{
    glBegin(GL_TRIANGLES);
    
    glColor4fv(colour.e);
    
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(min.x, min.y);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(max.x, min.y);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(max.x, max.y);
    
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(min.x, min.y);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(max.x, max.y);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(min.x, max.y);
    
    glEnd();
}

function void OpenGL_Render(Game_RenderCommands *commands, PlatformAPI platform)
{
    glViewport(0, 0, commands->width, commands->height);
    
    glMatrixMode(GL_PROJECTION);
    
    f32 proj[] = 
    {
        SafeRatio(2.0f, (f32)commands->width, 1.0f), 0.0f, 0.0f, 0.0f,
        0.0f, SafeRatio(2.0f, (f32)commands->height, 1.0f), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
    };
    glLoadMatrixf(proj);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    for (usize baseAddress = 0; baseAddress < commands->pushBufferSize;)
    {
        RenderEntry_Header *header = (RenderEntry_Header *)(commands->pushBufferBase + baseAddress);
        baseAddress += sizeof(RenderEntry_Header);
        
        switch (header->type)
        {
            case RenderEntryType_RenderEntry_Bitmap:
            {
                RenderEntry_Bitmap *entry = (RenderEntry_Bitmap *)(commands->pushBufferBase + baseAddress);
                
                LoadedAssetHeader *assetHeader = GetAsset(commands->loadedAssets, AssetType_Bitmap, &entry->bitmapID, false);
                if (assetHeader->loadState == AssetLoad_Loaded)
                {
                    Bitmap texture = GetBitmapFromAssetHeader(assetHeader);
                    ASSERT(texture.memory);
                    
                    glBindTexture(GL_TEXTURE_2D, assetHeader->textureHandle);
                    
                    v2f min = entry->positioning.pos - (entry->positioning.dims * texture.info.align);
                    v2f max = min + entry->positioning.dims;
                    
                    f32 cosAngle = Cos(entry->angle);
                    f32 sinAngle = Sin(entry->angle);
                    
                    glMatrixMode(GL_MODELVIEW);
                    f32 modelView[] = 
                    {
                        cosAngle, sinAngle, 0.0f, 0.0f,
                        -sinAngle, cosAngle, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        entry->positioning.pos.x - cosAngle * entry->positioning.pos.x - -sinAngle * entry->positioning.pos.y,
                        entry->positioning.pos.y - sinAngle * entry->positioning.pos.x - cosAngle * entry->positioning.pos.y, 0.0f, 1.0f,
                    };
                    glLoadMatrixf(modelView);
                    
                    OpenGL_Rectangle(min, max, entry->colour);
                    
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                }
                
                baseAddress += sizeof(RenderEntry_Bitmap);
            } break;
            
            case RenderEntryType_RenderEntry_Rect:
            {
                RenderEntry_Rect *entry = (RenderEntry_Rect *)(commands->pushBufferBase + baseAddress);
                
                v2f min = entry->positioning.pos - (entry->positioning.dims / 2.0f);
                v2f max = entry->positioning.pos + (entry->positioning.dims / 2.0f);
                
                glDisable(GL_TEXTURE_2D);
                OpenGL_Rectangle(min, max, entry->colour);
                glEnable(GL_TEXTURE_2D);
                
                baseAddress += sizeof(RenderEntry_Rect);
            } break;
            
            case RenderEntryType_RenderEntry_Clear:
            {
                RenderEntry_Clear *entry = (RenderEntry_Clear *)(commands->pushBufferBase + baseAddress);
                
                glClearColor(entry->colour.r, entry->colour.g, entry->colour.b, entry->colour.a);
                glClear(GL_COLOR_BUFFER_BIT);
                
                baseAddress += sizeof(RenderEntry_Clear);
            } break;
            
            case RenderEntryType_RenderEntry_Text:
            {
                RenderEntry_Text *entry = (RenderEntry_Text *)(commands->pushBufferBase + baseAddress);
                
                char *c = entry->string.text;
                f32 charPosX = entry->positioning.pos.x;
                while (*c)
                {
                    switch (*c)
                    {
                        case '\n':
                        {
                            entry->positioning.pos.y += entry->metadata.lineGap * entry->scale;
                            charPosX = entry->positioning.pos.x;
                        } break;
                        
                        default:
                        {
                            GlyphIdentifier glyphID = {*c, entry->font};
                            LoadedAssetHeader *assetHeader = GetAsset(commands->loadedAssets, AssetType_Glyph, &glyphID, false);
                            if (assetHeader->loadState == AssetLoad_Loaded)
                            {
                                Glyph glyph = GetGlyphFromAssetHeader(assetHeader);
                                ASSERT(glyph.memory);
                                
                                glBindTexture(GL_TEXTURE_2D, assetHeader->textureHandle);
                                
                                v2f min = V2F(charPosX, entry->positioning.pos.y) - ((ToV2F(glyph.info.dims) * entry->scale) * glyph.info.align);
                                v2f max = min + (ToV2F(glyph.info.dims) * entry->scale);
                                OpenGL_Rectangle(min, max, entry->colour);
                                
                                if (*(c + 1))
                                {
                                    Kerning kerning = GetKerningInfo(&entry->kerningTable, *c, *(c + 1));
                                    charPosX += (glyph.info.dims.x * entry->scale) + (kerning.advance * entry->scale) + 1.0f;
                                }
                            }
                        } break;
                    }
                    
                    c++;
                }
                
                platform.MemFree(entry->string.text);
                baseAddress += sizeof(RenderEntry_Text);
            } break;
            
            INVALID_DEFAULT;
        }
    }
    
    commands->pushBufferSize = 0;
}
