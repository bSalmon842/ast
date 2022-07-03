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
        IntrospectMemberDef *def = &defs[defIndex];
        
        void *memberPtr = &(((u8 *)ptr)[def->offset]);
        char string[128] = {};
        
        // TODO(bSalmon): Enum printing
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
                stbsp_sprintf(string, "%s: %d", def->name, *(EntityType *)memberPtr);
            } break;
            
            case MemberType_Collider:
            {
                // TODO(bSalmon): Click to open nested structs
                stbsp_sprintf(string, "%s: 0x%p", def->name, (Collider *)memberPtr);
                Meta_DumpStruct(ARRAY_COUNT(introspected_Collider), introspected_Collider, (Collider *)memberPtr, commands, camera, debugLineOffset, colourTableIndex + 1);
            } break;
            
            case MemberType_ColliderType:
            {
                stbsp_sprintf(string, "%s: %d", def->name, *(ColliderType *)memberPtr);
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
        debugLineOffset->y += 20.0f;
    }
    
    debugLineOffset->x -= 20.0f;
}