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

// NOTE(bSalmon): WGL defines
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

// NOTE(bSalmon): Universal GL defines
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_SRGB8_ALPHA8             0x8C43
#define GL_FRAMEBUFFER_SRGB         0x8DB9

global GLuint defaultOpenGLInternalFormat;

struct OpenGL_Info
{
    char *vendor;
    char *renderer;
    char *version;
    char *glslVersion;
    char *extensions;
    
    b32 GL_EXT_texture_sRGB;
    b32 GL_ARB_framebuffer_sRGB;
};

function OpenGL_Info OpenGL_GetInfo()
{
    OpenGL_Info result = {};
    
    result.vendor = (char *)glGetString(GL_VENDOR);
    result.renderer = (char *)glGetString(GL_RENDERER);
    result.version = (char *)glGetString(GL_VERSION);
    result.glslVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    result.extensions = (char *)glGetString(GL_EXTENSIONS);
    
    char *token = result.extensions;
    while (*token)
    {
        while (IsCharWhitespace(*token)) { token++; }
        char *end = token;
        while (*end && !IsCharWhitespace(*end)) { end++; }
        
        s32 charCount = (s32)(end - token);
        
        if (StringsAreSame(token, "GL_EXT_texture_sRGB", charCount)) { result.GL_EXT_texture_sRGB = true; }
        else if (StringsAreSame(token, "GL_ARB_framebuffer_sRGB", charCount)) { result.GL_ARB_framebuffer_sRGB = true; }
        
        token = end;
    }
    
    return result;
}

function void OpenGL_Init()
{
    OpenGL_Info glInfo = OpenGL_GetInfo();
    
    defaultOpenGLInternalFormat = GL_RGBA8;
    if (glInfo.GL_EXT_texture_sRGB)
    {
        defaultOpenGLInternalFormat = GL_SRGB8_ALPHA8;
    }
    
    if (glInfo.GL_ARB_framebuffer_sRGB)
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

global u32 globalTexHandleCount = 0;
function void OpenGL_Render(Game_RenderCommands *commands, PlatformAPI platform)
{
    glViewport(0, 0, commands->width, commands->height);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    
    f32 proj[] = 
    {
        SafeRatio(2.0f, (f32)commands->width, 1.0f), 0.0f, 0.0f, 0.0f,
        0.0f, SafeRatio(2.0f, (f32)commands->height, 1.0f), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
    };
    glLoadMatrixf(proj);
    
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
                v2f min = V2F();
                v2f max = V2F();
                
                LoadedAssetHeader *assetHeader = GetAsset(commands->loadedAssets, AssetType_Bitmap, &entry->bitmapID, false);
                if (assetHeader->loadState == AssetLoad_Loaded)
                {
                    Bitmap texture = GetBitmapFromAssetHeader(assetHeader);
                    ASSERT(texture.memory);
                    
                    if (texture.info.handle)
                    {
                        glBindTexture(GL_TEXTURE_2D, texture.info.handle);
                    }
                    else
                    {
                        texture.info.handle = ++globalTexHandleCount;
                        glBindTexture(GL_TEXTURE_2D, texture.info.handle);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture.info.dims.w, texture.info.dims.h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texture.memory);
                        
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                    }
                    
                    min = entry->offset - (entry->dims * texture.info.align);
                    max = min + entry->dims;
                }
                
                f32 cosAngle = Cos(entry->angle);
                f32 sinAngle = Sin(entry->angle);
                
                glMatrixMode(GL_MODELVIEW);
                f32 modelView[] = 
                {
                    cosAngle, sinAngle, 0.0f, 0.0f,
                    -sinAngle, cosAngle, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    entry->offset.x - cosAngle * entry->offset.x - -sinAngle * entry->offset.y,
                    entry->offset.y - sinAngle * entry->offset.x - cosAngle * entry->offset.y, 0.0f, 1.0f,
                };
                glLoadMatrixf(modelView);
                
                OpenGL_Rectangle(min, max, entry->colour);
                
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                
                baseAddress += sizeof(RenderEntry_Bitmap);
            } break;
            
            case RenderEntryType_RenderEntry_Rect:
            {
                RenderEntry_Rect *entry = (RenderEntry_Rect *)(commands->pushBufferBase + baseAddress);
                
                v2f min = entry->offset - (entry->dims / 2.0f);
                v2f max = entry->offset + (entry->dims / 2.0f);
                
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
                f32 charPosX = entry->offset.x;
                while (*c)
                {
                    
                    GlyphIdentifier glyphID = {*c, entry->font};
                    LoadedAssetHeader *assetHeader = GetAsset(commands->loadedAssets, AssetType_Glyph, &glyphID, false);
                    if (assetHeader->loadState == AssetLoad_Loaded)
                    {
                        Glyph glyph = GetGlyphFromAssetHeader(assetHeader);
                        ASSERT(glyph.memory);
                        if (glyph.info.glyph != ' ')
                        {
                            if (glyph.info.handle)
                            {
                                glBindTexture(GL_TEXTURE_2D, glyph.info.handle);
                            }
                            else
                            {
                                glyph.info.handle = ++globalTexHandleCount;
                                glBindTexture(GL_TEXTURE_2D, glyph.info.handle);
                                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, glyph.info.dims.w, glyph.info.dims.h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, glyph.memory);
                                
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                            }
                            
                            v2f min = V2F(charPosX, entry->offset.y) - ((ToV2F(glyph.info.dims) * entry->scale) * glyph.info.align);
                            v2f max = min + (ToV2F(glyph.info.dims) * entry->scale);
                            OpenGL_Rectangle(min, max, entry->colour);
                            
                            if (*(c + 1))
                            {
                                Kerning kerning = GetKerningInfo(&entry->kerningTable, *c, *(c + 1));
                                charPosX += (glyph.info.dims.x * entry->scale) + (kerning.advance * entry->scale) + 1.0f;
                            }
                        }
                        else if (glyph.info.glyph == ' ')
                        {
                            LoadedAssetHeader *metadataHeader = GetAsset(commands->loadedAssets, AssetType_FontMetadata, entry->font, true);
                            FontMetadata metadata = metadataHeader->metadata;
                            if (metadata.monospace)
                            {
                                GlyphInfo sizeGlyph = assetHeader->glyph;
                                charPosX += sizeGlyph.dims.w * entry->scale;
                            }
                            else
                            {
                                charPosX += 32.0f * entry->scale;
                            }
                        }
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
