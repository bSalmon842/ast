/*
Project: Asteroids
File: ast_interface.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

function void DiscreteSlider(Game_RenderCommands *commands, PlatformAPI platform, Camera camera, Game_Input *input, f32 *a, f32 *b, f32 leftBound, f32 rightBound, f32 y, f32 height, s32 zLayer)
{
    PushDiscreteSlider(commands, platform, camera, *a, *b, leftBound, rightBound, y, height, zLayer);
}