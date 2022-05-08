/*
Project: Asteroids
File: ast_camera.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#define DEFAULT_CAMERA_Z 10.0f

function void CameraMode_Perspective(Camera *camera, f32 focalLength, f32 nearClip, v2f worldToPixelConversion)
{
    camera->focalLength = focalLength;
    camera->nearClip = nearClip;
    camera->worldToPixelConversion = worldToPixelConversion;
    camera->orthographic = false;
}

function void CameraMode_Orthographic(Camera *camera)
{
    camera->focalLength = 1.0f;
    camera->nearClip = 0.1f;
    camera->worldToPixelConversion = V2F(1.0f) * camera->rect.center.z;
    camera->orthographic = true;
}

function Camera InitialiseCamera(v3f cameraPos, s32 linkIndex = 0)
{
    Camera result = {};
    
    result.rect = CreateRect3f_CenterDims(cameraPos, V3F(100.0f));
    result.linkedEntityIndex = linkIndex;
    CameraMode_Orthographic(&result);
    
    return result;
}

function void LinkCameraToEntity(Camera *camera, s32 index)
{
    camera->linkedEntityIndex = index;
}

function void ChangeCameraDistance(Camera *camera, f32 dist)
{
    camera->rect.center.z = dist;
}

inline v2f UnprojectPoint(Camera camera, f32 distanceFromCamera, v2f projPoint)
{
    v2f result = {};
    
    ASSERT(!camera.orthographic);
    result = (distanceFromCamera / camera.focalLength) * (projPoint / camera.worldToPixelConversion);
    
    return result;
}

function Rect2f GetCameraBoundsForDistance(Game_RenderCommands *commands, Camera camera, v2f pos, f32 distance)
{
    Rect2f result = {};
    
    v2f projXY = V2F((f32)commands->width, (f32)commands->height);
    v2f rawXY = UnprojectPoint(camera, distance, projXY);
    
    result = CreateRect2f_CenterDims(pos, rawXY);
    
    return result;
}

function void UpdateCamera(Game_RenderCommands *commands, Camera *camera, Entity entity)
{
    camera->rect.center.xy = entity.pos.xy;
    Rect2f boundsRect = GetCameraBoundsForDistance(commands, *camera, camera->rect.center.xy, camera->rect.center.z);
    camera->rect = CreateRect3f_CenterDims(camera->rect.center, V3F(boundsRect.dims, camera->rect.center.z));
    camera->screenCenterPixels = V2F((f32)commands->width, (f32)commands->height) / 2.0f;
}
