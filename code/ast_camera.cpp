/*
Project: Asteroids
File: ast_camera.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

function Camera InitialiseCamera(v3f cameraPos, f32 focalLength, f32 nearClip, f32 verticalFOV, s32 linkIndex = 0)
{
    Camera result = {};
    
    result.pos = cameraPos;
    result.focalLength = focalLength;
    result.nearClip = nearClip;
    result.verticalFOV = verticalFOV;
    result.linkedEntityIndex = linkIndex;
    
    return result;
}

function void LinkCameraToEntity(Camera *camera, s32 index)
{
    camera->linkedEntityIndex = index;
}

function void UpdateCamera(Camera *camera, Entity entity)
{
    camera->pos.xy = entity.pos.xy;
}

function void ChangeCameraDistance(Camera *camera, f32 dist)
{
    camera->pos.z = dist;
}