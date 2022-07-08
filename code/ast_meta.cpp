/*
Project: Asteroids
File: ast_meta.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#include "ast_generated.h"

#include "cmt_colour_tables.h"

function void Meta_DumpStruct(u32 defCount, IntrospectMemberDef *defs, void *ptr, Game_RenderCommands *commands, Camera camera, v3f *debugLineOffset, s32 colourTableIndex)
{
    debugLineOffset->x += 20.0f;
    
    for (u32 defIndex = defCount - 1; defIndex >= 0 && defIndex < defCount; --defIndex)
    {
        char string[128] = {};
        
        IntrospectMemberDef *def = &defs[defIndex];
        void *memberPtr = &(((u8 *)ptr)[def->offset]);
        
        switch(def->type)
        {
            case MemberType_void:
            {
                stbsp_sprintf(string, "%s: (void *) 0x%p", def->name, memberPtr);
            } break;
            
            case MemberType_f32:
            {
                stbsp_sprintf(string, "%s: %.03f", def->name, *(f32 *)memberPtr);
            } break;
            
            case MemberType_s32:
            {
                stbsp_sprintf(string, "%s: %d", def->name, *(s32 *)memberPtr);
            } break;
            
            case MemberType_b32:
            {
                stbsp_sprintf(string, "%s: %s", def->name, (*(s32 *)memberPtr) ? "true" : "false");
            } break;
            
            case MemberType_v2f:
            {
                stbsp_sprintf(string, "%s: {%.03f, %.03f}", def->name, ((v2f *)memberPtr)->x, ((v2f *)memberPtr)->y);
            } break;
            
            case MemberType_v3f:
            {
                stbsp_sprintf(string, "%s: {%.03f, %.03f, %.03f}", def->name, ((v3f *)memberPtr)->x, ((v3f *)memberPtr)->y, ((v3f *)memberPtr)->z);
            } break;
            
            case MemberType_EntityType:
            {
                IntrospectEnumDef enumDef = GetEnumDef(EntityType, *(EntityType *)memberPtr);
                stbsp_sprintf(string, "%s: %s (%d)", def->name, enumDef.name, (u16)enumDef.value);
            } break;
            
            case MemberType_Collider:
            {
                // TODO(bSalmon): Click to open nested structs
                stbsp_sprintf(string, "%s: 0x%p", def->name, (Collider *)memberPtr);
                Meta_DumpStruct(ARRAY_COUNT(introspected_Collider), introspected_Collider, (Collider *)memberPtr, commands, camera, debugLineOffset, colourTableIndex + 1);
            } break;
            
            case MemberType_ColliderType:
            {
                IntrospectEnumDef enumDef = GetEnumDef(ColliderType, *(ColliderType *)memberPtr);
                stbsp_sprintf(string, "%s: %s (%d)", def->name, enumDef.name, (u16)enumDef.value);
            } break;
            
            case MemberType_Array_CollisionInfo:
            {
                stbsp_sprintf(string, "%s: [%d] %p", def->name, def->elementCount, (CollisionInfo *)memberPtr);
            } break;
            
            default:
            {
                stbsp_sprintf(string, "%s: Unhandled (%d)\n", def->name, def->type);
            } break;
        }
        
        PushText(commands, camera, string, "Debug", *debugLineOffset, DEBUG_TEXT_SCALE, DEBUG_LAYER, globalColourTable_A[colourTableIndex]);
        debugLineOffset->y += 15.0f;
    }
    
    debugLineOffset->x -= 20.0f;
}

function void AddToDebugConsoleOutput(DebugState *debugState, PlatformAPI platform, char *string, v4f colour);
function void Meta_DumpStruct(u32 defCount, IntrospectMemberDef *defs, void *ptr, DebugState *debugState, PlatformAPI platform, s32 tabCount, s32 colourTableIndex)
{
    tabCount++;
    for (u32 defIndex = defCount - 1; defIndex >= 0 && defIndex < defCount; --defIndex)
    {
        IntrospectMemberDef *def = &defs[defIndex];
        
        void *memberPtr = &(((u8 *)ptr)[def->offset]);
        char string[128] = {};
        
        for (s32 i = 0; i < tabCount; ++i)
        {
            sprintf(&string[i], "\t");
        }
        
        char *partialString = &string[tabCount];
        switch(def->type)
        {
            case MemberType_void:
            {
                stbsp_sprintf(partialString, "%s: (void *) 0x%p", def->name, memberPtr);
            } break;
            
            case MemberType_f32:
            {
                stbsp_sprintf(partialString, "%s: %.03f", def->name, *(f32 *)memberPtr);
            } break;
            
            case MemberType_s32:
            {
                stbsp_sprintf(partialString, "%s: %d", def->name, *(s32 *)memberPtr);
            } break;
            
            case MemberType_b32:
            {
                stbsp_sprintf(partialString, "%s: %s", def->name, (*(s32 *)memberPtr) ? "true" : "false");
            } break;
            
            case MemberType_v2f:
            {
                stbsp_sprintf(partialString, "%s: {%.03f, %.03f}", def->name, ((v2f *)memberPtr)->x, ((v2f *)memberPtr)->y);
            } break;
            
            case MemberType_v3f:
            {
                stbsp_sprintf(partialString, "%s: {%.03f, %.03f, %.03f}", def->name, ((v3f *)memberPtr)->x, ((v3f *)memberPtr)->y, ((v3f *)memberPtr)->z);
            } break;
            
            case MemberType_EntityType:
            {
                IntrospectEnumDef enumDef = GetEnumDef(EntityType, *(EntityType *)memberPtr);
                stbsp_sprintf(partialString, "%s: %s (%d)", def->name, enumDef.name, (u16)enumDef.value);
            } break;
            
            case MemberType_Collider:
            {
                // TODO(bSalmon): Click to open nested structs
                stbsp_sprintf(partialString, "%s: 0x%p", def->name, (Collider *)memberPtr);
                Meta_DumpStruct(ARRAY_COUNT(introspected_Collider), introspected_Collider, (Collider *)memberPtr, debugState, platform, tabCount, colourTableIndex + 1);
            } break;
            
            case MemberType_ColliderType:
            {
                IntrospectEnumDef enumDef = GetEnumDef(ColliderType, *(ColliderType *)memberPtr);
                stbsp_sprintf(partialString, "%s: %s (%d)", def->name, enumDef.name, (u16)enumDef.value);
            } break;
            
            case MemberType_Array_CollisionInfo:
            {
                stbsp_sprintf(partialString, "%s: [%d] %p", def->name, def->elementCount, (CollisionInfo *)memberPtr);
            } break;
            
            default:
            {
                stbsp_sprintf(partialString, "%s: Unhandled (%d)\n", def->name, def->type);
            } break;
        }
        
        AddToDebugConsoleOutput(debugState, platform, string, globalColourTable_A[colourTableIndex]);
    }
}