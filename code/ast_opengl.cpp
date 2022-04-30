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
    
    for (u32 entryIndex = 0; entryIndex < commands->entryCount; ++entryIndex)
    {
        RenderEntry_Header *header = commands->headers[entryIndex];
        
        switch (header->type)
        {
            case RenderEntryType_RenderEntry_Bitmap:
            {
                RenderEntry_Bitmap *entry = (RenderEntry_Bitmap *)(header + 1);
                
                LoadedAssetHeader *assetHeader = entry->assetHeader;
                if (assetHeader->loadState == AssetLoad_Loaded)
                {
                    BitmapInfo texture = assetHeader->bitmap;
                    
                    glBindTexture(GL_TEXTURE_2D, assetHeader->textureHandle);
                    
                    v2f min = entry->positioning.pos.xy - (entry->positioning.dims * texture.align);
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
            } break;
            
            case RenderEntryType_RenderEntry_Rect:
            {
                RenderEntry_Rect *entry = (RenderEntry_Rect *)(header + 1);
                
                v2f min = entry->positioning.pos.xy - (entry->positioning.dims / 2.0f);
                v2f max = entry->positioning.pos.xy + (entry->positioning.dims / 2.0f);
                
                glDisable(GL_TEXTURE_2D);
                OpenGL_Rectangle(min, max, entry->colour);
                glEnable(GL_TEXTURE_2D);
            } break;
            
            case RenderEntryType_RenderEntry_Clear:
            {
                RenderEntry_Clear *entry = (RenderEntry_Clear *)(header + 1);
                
                glClearColor(entry->colour.r, entry->colour.g, entry->colour.b, entry->colour.a);
                glClear(GL_COLOR_BUFFER_BIT);
            } break;
            
            case RenderEntryType_RenderEntry_Text:
            {
                RenderEntry_Text *entry = (RenderEntry_Text *)(header + 1);
                
                char *c = entry->string;
                f32 charPosX = entry->positioning.pos.x;
                for (s32 i = 0; *c; ++i)
                {
                    switch (*c)
                    {
                        case '\n':
                        {
                            entry->positioning.pos.y -= entry->metadata.lineGap * entry->scale;
                            charPosX = entry->positioning.pos.x;
                        } break;
                        
                        default:
                        {
                            LoadedAssetHeader *assetHeader = entry->assetHeaders[i];
                            if (assetHeader->loadState == AssetLoad_Loaded)
                            {
                                GlyphInfo glyph = assetHeader->glyph;
                                
                                glBindTexture(GL_TEXTURE_2D, assetHeader->textureHandle);
                                
                                v2f min = V2F(charPosX, entry->positioning.pos.y) - ((ToV2F(glyph.dims) * entry->scale) * glyph.align);
                                v2f max = min + (ToV2F(glyph.dims) * entry->scale);
                                OpenGL_Rectangle(min, max, entry->colour);
                                
                                if (*(c + 1))
                                {
                                    Kerning kern = {};
                                    KerningTable table = entry->kerningTable;
                                    for (u32 index = 0; index < table.info.infoCount; ++index)
                                    {
                                        Kerning info = table.table[index];
                                        if (info.codepoint0 == *c &&
                                            info.codepoint1 == *(c + 1))
                                        {
                                            kern = info;
                                            break;
                                        }
                                    }
                                    
                                    charPosX += (glyph.dims.x * entry->scale) + (kern.advance * entry->scale) + 1.0f;
                                }
                            }
                        } break;
                    }
                    
                    c++;
                }
            } break;
            
            INVALID_DEFAULT;
        }
    }
    
    //FinishTempMemory(commands->renderTemp);
    commands->pushBufferSize = 0;
    commands->entryCount = 0;
}
