/*
Project: Asteroids
File: ast_interface.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

function b32 DebugButton(Game_RenderCommands *commands, Game_LoadedAssets *loadedAssets, Game_Input *input, PlatformAPI platform, Camera camera, char *text, char *font, v2f textOrigin, f32 textScale, s32 zLayer, v4f textColour, v2f min, v2f max)
{
    b32 result = false;
    
    PushText(commands, loadedAssets, platform, camera, text, font, V3F(textOrigin, 0.0f),
             textScale, zLayer, textColour);
    
    v2f mousePos = V2F((f32)input->mouse.x, (f32)input->mouse.y);
    if (mousePos > min && mousePos < max)
    {
        PushRect(commands, platform, camera, min, max, 0.0f, 0.0f, DEBUG_LAYER - 2, V4F(1.0f, 0.0f, 1.0f, 1.0f));
        if (InputNoRepeat(input->mouse.buttons[MouseButton_L]))
        {
            result = true;
        }
    }
    
    return result;
}

function void DebugDiscreteSlider(Game_RenderCommands *commands, PlatformAPI platform, Camera camera, Game_Input *input, f32 *a, f32 *b, f32 leftBound, f32 rightBound, f32 y, f32 height, s32 zLayer)
{
    persist b32 leftHeld = false;
    persist b32 rightHeld = false;
    persist b32 middleHeld = false;
    
    v2f mousePos = V2F((f32)input->mouse.x, (f32)input->mouse.y);
    f32 width = rightBound - leftBound;
    f32 valueDist = *b - *a;
    
    f32 maxZoom = 0.015f;
    
    if (input->mouse.buttons[MouseButton_L].endedFrameDown)
    {
        f32 newPos = (mousePos.x - leftBound) / width;
        f32 newA = *a;
        f32 newB = *b;
        
        if (leftHeld)
        {
            newA = newPos;
            
            if (newA < 0.0f)
            {
                newA = 0.0f;
            }
            if (newA > newB - maxZoom)
            {
                newA = newB - maxZoom;
            }
        }
        
        if (rightHeld)
        {
            newB = newPos;
            
            if (newB > 1.0f)
            {
                newB = 1.0f;
            }
            if (newB < newA + maxZoom)
            {
                newB = newA + maxZoom;
            }
        }
        
        if (middleHeld)
        {
            newA = newPos - (valueDist / 2.0f);
            newB = newPos + (valueDist / 2.0f);
            
            if (newA < 0.0f)
            {
                newA = 0.0f;
                newB = newA + valueDist;
            }
            else if (newB > 1.0f)
            {
                newB = 1.0f;
                newA = newB - valueDist;
            }
        }
        
        *a = newA;
        *b = newB;
    }
    
    PushRect(commands, platform, camera, V2F(leftBound, y - 1.0f), V2F(rightBound, y + 1.0f), 0.0f, 0.0f, zLayer, V4F(1.0f));
    
    v2f leftMin = V2F((leftBound + (width * *a)) - (height / 4.0f), y - (height / 2.0f));
    v2f leftMax = leftMin + V2F(height / 2.0f, height);
    
    v2f rightMin = V2F((leftBound + (width * *b)) - (height / 4.0f), y - (height / 2.0f));
    v2f rightMax = rightMin + V2F(height / 2.0f, height);
    
    f32 middleLoc = *a + ((*b - *a) / 2.0f);
    v2f middleMin = V2F((leftBound + (width * middleLoc)) - (height / 2.0f), y - (height / 2.0f));
    v2f middleMax = middleMin + V2F(height);
    
    if (!input->mouse.buttons[MouseButton_L].endedFrameDown)
    {
        leftHeld = false;
        middleHeld = false;
        rightHeld = false;
    }
    
    if (mousePos > leftMin && mousePos < leftMax && input->mouse.buttons[MouseButton_L].endedFrameDown && !middleHeld && !rightHeld)
    {
        leftHeld = true;
    }
    else if (mousePos > middleMin && mousePos < middleMax && input->mouse.buttons[MouseButton_L].endedFrameDown && !leftHeld && !rightHeld)
    {
        middleHeld = true;
    }
    else if (mousePos > rightMin && mousePos < rightMax && input->mouse.buttons[MouseButton_L].endedFrameDown && !leftHeld && !middleHeld)
    {
        rightHeld = true;
    }
    
    PushRect(commands, platform, camera, leftMin, leftMax, 0.0f, 0.0f, zLayer + 1, V4F(1.0f, 0.0f, 0.0f, 1.0f));
    PushRect(commands, platform, camera, rightMin, rightMax, 0.0f, 0.0f, zLayer + 1, V4F(0.0f, 0.0f, 1.0f, 1.0f));
    PushRect(commands, platform, camera, middleMin, middleMax, 0.0f, 0.0f, zLayer + 1, V4F(0.0f, 1.0f, 0.0f, 1.0f));
}