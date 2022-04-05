/*
Project: Asteroids
File: ast_camera.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#define DEFAULT_CAMERA_Z 10.0f

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

inline v2f UnprojectPoint(f32 distanceFromCamera, f32 focalLength, v2f projPoint)
{
    v2f result = (distanceFromCamera / focalLength) * projPoint;
    return result;
}

function Rect2f GetCameraBoundsForDistance(Game_RenderCommands *commands, Camera camera, v2f worldToPixelConversion, v2f pos, f32 distance)
{
    Rect2f result = {};
    
    v2f projXY = V2F((f32)commands->width, (f32)commands->height) / worldToPixelConversion;
    v2f rawXY = UnprojectPoint(distance, camera.focalLength, projXY);
    
    result = CreateRect2f_CenterDims(pos, rawXY);
    
    return result;
}