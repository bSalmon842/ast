/*
Project: Asteroids
File: w32_ast.cpp
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mmdeviceapi.h>
#include <AudioClient.h>
#include <stdio.h>

#include "ast_utility.h"

#define BS842_W32TIMING_IMPLEMENTATION
#include "bs842_w32_timing.h"

#define BS842_PLOTTING_IMPLEMENTATION
#include "bs842_plotting.h"

#define BITMAP_BYTES_PER_PIXEL 4

struct W32_BackBuffer
{
    BITMAPINFO info;
    void *memory;
    s32 width;
    s32 height;
    u32 pitch;
};

struct W32_WindowDims
{
    s32 width;
    s32 height;
};

struct W32_AudioOutput
{
    IAudioClient *audioClient;
    IAudioRenderClient *renderClient;
    s32 samplesPerSecond;
    s32 bytesPerSample;
    s32 secondaryBufferSize;
    s32 latencySampleCount;
    u32 runningSampleIndex;
};

struct W32_DebugTimeMarker
{
    u32 playCursor;
    u32 writeCursor;
};

struct Game_AudioBuffer
{
    s16 *samples;
};

struct Game_ButtonState
{
    b32 endedFrameDown;
    s32 halfTransitionCount;
};

struct Game_Keyboard
{
    b32 isConnected;
    
    union
    {
        Game_ButtonState keys[6];
        struct
        {
            Game_ButtonState keyW;
            Game_ButtonState keyS;
            Game_ButtonState keyA;
            Game_ButtonState keyD;
            Game_ButtonState keySpace;
            Game_ButtonState keyEsc;
        };
    };
};

struct Game_Input
{
    b32 exeReloaded;
    f32 deltaTime;
    
    Game_Keyboard keyboard;
};

global b32 globalRunning;

#define CHECK_COM_FAILURE_ASSERT(func) if (FAILED(func)) {ASSERT(!"Error");}

//~ AUDIO
function void W32_InitWASAPI(IAudioClient **audioClient, IAudioRenderClient **audioRenderClient, s32 samplesPerSecond, s32 bufferSizeSamples)
{
    IMMDeviceEnumerator *enumerator;
    IMMDevice *device;
    
    CHECK_COM_FAILURE_ASSERT(CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY));
    CHECK_COM_FAILURE_ASSERT(CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, IID_PPV_ARGS(&enumerator)));
    CHECK_COM_FAILURE_ASSERT(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device));
    CHECK_COM_FAILURE_ASSERT(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (LPVOID *)&(*audioClient)));
    
    WAVEFORMATEXTENSIBLE wavFmt;
    wavFmt.Format.cbSize = sizeof(wavFmt);
    wavFmt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    wavFmt.Format.wBitsPerSample = 16;
    wavFmt.Format.nChannels = 2;
    wavFmt.Format.nSamplesPerSec = samplesPerSecond;
    wavFmt.Format.nBlockAlign = wavFmt.Format.nChannels * (wavFmt.Format.wBitsPerSample / 8);
    wavFmt.Format.nAvgBytesPerSec = wavFmt.Format.nSamplesPerSec * wavFmt.Format.nBlockAlign;
    wavFmt.Samples.wValidBitsPerSample = 16;
    wavFmt.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
    wavFmt.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    
    REFERENCE_TIME bufDuration = 10000000ULL * bufferSizeSamples / samplesPerSecond;
    u32 audioFrameCount;
    
    CHECK_COM_FAILURE_ASSERT((*audioClient)->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_NOPERSIST, bufDuration, 0, &wavFmt.Format, 0));
    CHECK_COM_FAILURE_ASSERT((*audioClient)->GetService(IID_PPV_ARGS(&(*audioRenderClient))));
    CHECK_COM_FAILURE_ASSERT((*audioClient)->GetBufferSize(&audioFrameCount));
    ASSERT(bufferSizeSamples <= (s32)audioFrameCount);
}

function void W32_FillAudioBuffer(W32_AudioOutput *audioOutput, u32 samplesWriteCount, Game_AudioBuffer *src)
{
    BYTE *audioBufferData;
    if (SUCCEEDED(audioOutput->renderClient->GetBuffer(samplesWriteCount, &audioBufferData)))
    {
        s16 *srcSample = src->samples;
        s16 *destSample = (s16 *)audioBufferData;
        for (u32 sampleIndex = 0; sampleIndex < samplesWriteCount; ++sampleIndex)
        {
            *destSample++ = *srcSample++;
            *destSample++ = *srcSample++;
            ++audioOutput->runningSampleIndex;
        }
        
        audioOutput->renderClient->ReleaseBuffer(samplesWriteCount, 0);
    }
}

//~ INPUT
function void W32_ProcessKeyboardEvent(Game_ButtonState *state, b32 keyIsDown)
{
    if (state->endedFrameDown != keyIsDown)
    {
        state->endedFrameDown = keyIsDown;
        ++state->halfTransitionCount;
    }
}

//~ RENDERING
inline W32_WindowDims W32_GetWindowDims(HWND window)
{
    W32_WindowDims result = {};
    
    RECT clientRect;
    GetClientRect(window, &clientRect);
    result.width = clientRect.right - clientRect.left;
    result.height = clientRect.bottom - clientRect.top;
    
    return result;
}

function void W32_ResizeDIBSection(W32_BackBuffer *backBuffer, s32 width, s32 height)
{
    if (backBuffer->memory)
    {
        VirtualFree(backBuffer->memory, 0, MEM_RELEASE);
    }
    
    backBuffer->width = width;
    backBuffer->height = height;
    backBuffer->pitch = backBuffer->width * BITMAP_BYTES_PER_PIXEL;
    
    backBuffer->info.bmiHeader.biSize = sizeof(backBuffer->info.bmiHeader);
    backBuffer->info.bmiHeader.biWidth = backBuffer->width;
    backBuffer->info.bmiHeader.biHeight = -backBuffer->height;
    backBuffer->info.bmiHeader.biPlanes = 1;
    backBuffer->info.bmiHeader.biBitCount = 32;
    backBuffer->info.bmiHeader.biCompression = BI_RGB;
    
    s32 bitmapMemSize = backBuffer->pitch * backBuffer->height;
    backBuffer->memory = VirtualAlloc(0, bitmapMemSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

function void W32_PresentBuffer(HDC deviceContext, W32_BackBuffer *backBuffer, W32_WindowDims windowDim)
{
    if ((windowDim.width >= (backBuffer->width * 1.5f)) && (windowDim.height >= (backBuffer->height * 1.5f)))
    {
        StretchDIBits(deviceContext,
                      0, 0, windowDim.width, windowDim.height,     // DEST
                      0, 0, backBuffer->width, backBuffer->height, // SRC
                      backBuffer->memory, &backBuffer->info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
    else
    {
        s32 offsetX = 0;
        s32 offsetY = 0;
        
        PatBlt(deviceContext, 0, 0, windowDim.width, offsetY, BLACKNESS);
        PatBlt(deviceContext, 0, 0, offsetX, windowDim.height, BLACKNESS);
        PatBlt(deviceContext, offsetX + backBuffer->width, 0, windowDim.width, windowDim.height, BLACKNESS);
        PatBlt(deviceContext, 0, offsetY + backBuffer->height, windowDim.width, windowDim.height, BLACKNESS);
        
        StretchDIBits(deviceContext,
                      offsetX, offsetY, backBuffer->width, backBuffer->height, // DEST
                      0, 0, backBuffer->width, backBuffer->height,             // SRC
                      backBuffer->memory, &backBuffer->info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
}

function void RenderClear(W32_BackBuffer *backBuffer, u32 colour)
{
    u8 *row = (u8 *)backBuffer->memory;
    for (s32 y = 0; y < backBuffer->height; ++y)
    {
        u32 *pixel = (u32 *)row;
        for (s32 x = 0; x < backBuffer->width; ++x)
        {
            *pixel++ = colour;
        }
        
        row += backBuffer->pitch;
    }
}

function void RenderBox(W32_BackBuffer *backBuffer, s32 originX, s32 originY, s32 width, s32 height, u32 colour)
{
    // TODO(bSalmon): Do checks for screen bounds
    
    s32 minX = originX - width / 2;
    s32 minY = originY - height / 2;
    s32 maxX = originX + (width - width / 2);
    s32 maxY = originY + (height - height / 2);
    
    if (minX < 0)
    {
        minX = 0;
    }
    if (minY < 0)
    {
        minY = 0;
    }
    if (maxX == backBuffer->width)
    {
        maxX = backBuffer->width - 1;
    }
    if (maxY == backBuffer->height)
    {
        maxY = backBuffer->height - 1;
    }
    
    u8 *row = (u8 *)backBuffer->memory + (minY * backBuffer->pitch);
    for (s32 y = minY; y < maxY; ++y)
    {
        u32 *pixel = (u32 *)row + minX;
        for (s32 x = minX; x < maxX; ++x)
        {
            *pixel++ = colour;
        }
        
        row += backBuffer->pitch;
    }
}

function void RenderDebugVerticalLine(W32_BackBuffer *backBuffer, s32 x, s32 yTop, s32 yBottom, u32 colour)
{
    if (x >= 0 && x < backBuffer->width)
    {
        u8 *row = (u8 *)backBuffer->memory + (yTop * backBuffer->pitch);
        for (s32 y = yTop; y < yBottom; ++y)
        {
            u32 *pixel = (u32 *)row + x;
            *pixel = colour;
            
            row += backBuffer->pitch;
        }
    }
}

function void W32_RenderAudioBufferMarker(W32_BackBuffer *backBuffer, W32_AudioOutput *audioOutput, f32 c, s32 padX, s32 top, s32 bot,
                                          s32 value, u32 colour)
{
    ASSERT(value < audioOutput->secondaryBufferSize);
    f32 xFloat = (c * (f32)value);
    s32 x = padX + (s32)xFloat;
    RenderDebugVerticalLine(backBuffer, x, top, bot, colour);
}

function void W32_RenderAudioSyncDisplay(W32_BackBuffer *backBuffer, s32 markerCount, W32_DebugTimeMarker *markers,
                                         W32_AudioOutput *audioOutput, f32 targetSecondsPerFrame)
{
    s32 padX = 16;
    s32 padY = 16;
    
    s32 top = padY;
    s32 bot = backBuffer->height - padY;
    
    f32 c = (f32)(backBuffer->width - (2 * padX) / (f32)audioOutput->secondaryBufferSize);
    for (s32 markerIndex = 0; markerIndex < markerCount; ++markerIndex)
    {
        W32_DebugTimeMarker *currMarker = &markers[markerIndex];
        W32_RenderAudioBufferMarker(backBuffer, audioOutput, c, padX, top, bot, currMarker->playCursor, 0xFFFFFFFF);
    }
}

//~ WINDOWS
function void W32_ProcessPendingMessages(HWND window, W32_BackBuffer *backBuffer, Game_Keyboard *keyboard)
{
    MSG msg;
    
    while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
    {
        switch (msg.message)
        {
            case WM_QUIT:
            {
                globalRunning = false;
            }break;
            
            case WM_PAINT:
            {
                PAINTSTRUCT paint;
                HDC deviceContext = BeginPaint(window, &paint);
                
                W32_WindowDims windowDim = W32_GetWindowDims(window);
                W32_PresentBuffer(deviceContext, backBuffer, windowDim);
                EndPaint(window, &paint);
            }break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 vkCode = (u32)msg.wParam;
                b32 keyWasDown = (msg.lParam & (1 << 30)) != 0;
                b32 keyIsDown = (msg.lParam & (1 << 31)) == 0;
                if (keyWasDown != keyIsDown)
                {
                    if (vkCode == 'W')
                    {
                        W32_ProcessKeyboardEvent(&keyboard->keyW, keyIsDown);
                    }
                    if (vkCode == 'S')
                    {
                        W32_ProcessKeyboardEvent(&keyboard->keyS, keyIsDown);
                    }
                    if (vkCode == 'A')
                    {
                        W32_ProcessKeyboardEvent(&keyboard->keyA, keyIsDown);
                    }
                    if (vkCode == 'D')
                    {
                        W32_ProcessKeyboardEvent(&keyboard->keyD, keyIsDown);
                    }
                    if (vkCode == VK_SPACE)
                    {
                        W32_ProcessKeyboardEvent(&keyboard->keySpace, keyIsDown);
                    }
                    if (vkCode == VK_ESCAPE)
                    {
                        W32_ProcessKeyboardEvent(&keyboard->keyEsc, keyIsDown);
                    }
                    
                    if (keyIsDown)
                    {
                        b32 altKeyDown = msg.lParam & (1 << 29);
                        if (altKeyDown && vkCode == VK_F4)
                        {
                            globalRunning = false;
                        }
                    }
                }
            }break;
            
            default:
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }break;
        }
    }
}

function LRESULT CALLBACK W32_WndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    
    switch (msg)
    {
        case WM_CLOSE:
        {
            // TODO(bSalmon): Handle with message to the user
            globalRunning = false;
        }break;
        
        case WM_DESTROY:
        {
            // TODO(bSalmon): Handle as an error
            globalRunning = false;
        }break;
        
        case WM_PAINT:
        {
            //ASSERT(!"Paint Dispatched");
        }break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            ASSERT(!"Keyboard Input Dispatched");
        }break;
        
        default:
        {
            result = DefWindowProcA(window, msg, wParam, lParam);
        }break;
    }
    
    return result;
}

s32 WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, s32 showCode)
{
#if AST_INTERNAL
    AllocConsole();
    s32 hConsole = 0;
    FILE *fp;
    fp = _fdopen(hConsole, "w");
    
    freopen_s(&fp, "CONOUT$", "w", stdout);
#endif
    
    BS842_Timing_Init();
    
    BS842_Plotting_Init(1, 1200, 300, "C:\\Windows\\Fonts\\Arial.ttf");
    
    W32_BackBuffer platformBackBuffer = {};
    W32_ResizeDIBSection(&platformBackBuffer, 1280, 720);
    
    WNDCLASSA windowClass = {};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = W32_WndProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "AsteroidsWindowClass";
    
    if (RegisterClassA(&windowClass))
    {
        HWND window = CreateWindowExA(0,
                                      windowClass.lpszClassName, "Asteroids",
                                      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                      CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
                                      0, 0, instance, 0);
        
        if (window)
        {
            HDC deviceContext = GetDC(window);
            
            BS842_Timing_ChangeRefreshRate(deviceContext, true, 60);
            
            s32 audioLatencyFrames = 1;
            W32_AudioOutput audioOutput = {};
            audioOutput.samplesPerSecond = 48000;
            audioOutput.bytesPerSample = sizeof(s16) * 2;
            audioOutput.secondaryBufferSize = audioOutput.samplesPerSecond;
            audioOutput.latencySampleCount = audioLatencyFrames * (audioOutput.samplesPerSecond / BS842_Timing_GetRefreshRate());
            
            W32_InitWASAPI(&audioOutput.audioClient, &audioOutput.renderClient, audioOutput.samplesPerSecond, audioOutput.secondaryBufferSize);
            audioOutput.audioClient->Start();
            
            globalRunning = true;
            
            Game_Input inputs[2] = {};
            Game_Input *newInput = &inputs[0];
            Game_Input *oldInput = &inputs[1];
            
            s32 boxX = 100;
            s32 boxY = 100;
            
            s16 *samples = (s16 *)VirtualAlloc(0, audioOutput.secondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
#if AST_INTERNAL
            LPVOID baseAddress = (LPVOID)TERABYTE(2);
#else
            LPVOID baseAddress = 0;
#endif
            
            
            
            while (globalRunning)
            {
                newInput->deltaTime = BS842_Timing_GetDeltaTime();
                newInput->exeReloaded = false;
                
                Game_Keyboard *oldKeyboard = &oldInput->keyboard;
                Game_Keyboard *newKeyboard = &newInput->keyboard;
                *newKeyboard = {};
                newKeyboard->isConnected = true;
                
                for (s32 keyIndex = 0; keyIndex < ARRAY_COUNT(newKeyboard->keys); ++keyIndex)
                {
                    newKeyboard->keys[keyIndex].endedFrameDown = oldKeyboard->keys[keyIndex].endedFrameDown;
                }
                
                W32_ProcessPendingMessages(window, &platformBackBuffer, newKeyboard);
                
                RenderClear(&platformBackBuffer, 0xFFAAAAAA);
                
                u32 boxColour = 0xFFFFFFFF;
                if (newKeyboard->keyW.endedFrameDown)
                {
                    boxY--;
                }
                if (newKeyboard->keyS.endedFrameDown)
                {
                    boxY++;
                }
                if (newKeyboard->keyA.endedFrameDown)
                {
                    boxX--;
                }
                if (newKeyboard->keyD.endedFrameDown)
                {
                    boxX++;
                }
                if (newKeyboard->keySpace.endedFrameDown)
                {
                    boxColour -= 0x00FF0000;
                }
                if (newKeyboard->keyEsc.endedFrameDown)
                {
                    boxColour -= 0x0000FF00;
                }
                
#if 0
                RenderBox(&platformBackBuffer, boxX, boxY, 50, 50, boxColour);
                
                RenderDebugVerticalLine(&platformBackBuffer, 100, 200, 300, 0xFF00FF00);
#endif
                
                float xData0[8] = {12.0f, 24.5f, 32.0f, 48.0f, 60.0f, 65.0f, 79.0f, 90.0f};
                float yData0[8] = {0.0f, -5.0f, 20.0f, 50.0f, 80.0f, 86.0f, 70.0f, 50.0f};
                BS842_Plotting_PlotData(0, 0, xData0, yData0, 8);
                
                float xData1[8] = {10.0f, 20.5f, 32.0f, 52.0f, 67.0f, 70.0f, 78.0f, 92.0f};
                float yData1[8] = {40.0f, -10.0f, 30.0f, 33.0f, 90.0f, 150.0f, 60.0f, 40.0f};
                BS842_Plotting_PlotData(0, 1, xData1, yData1, 8);
                
                BS842_Plotting_UpdatePlot(0);
                
                BS842_Plotting_GetPlotMemory(0, (u8 *)platformBackBuffer.memory + ((128 * platformBackBuffer.pitch) + 64), platformBackBuffer.pitch);
                
                BS842_Timing_FrameEnd();
                
                //printf("%.02f / %.03f\n", BS842_Timing_GetFramesPerSecond(), BS842_Timing_GetMSPerFrame());
                
                W32_WindowDims windowDim = W32_GetWindowDims(window);
                W32_PresentBuffer(deviceContext, &platformBackBuffer, windowDim);
                
                SWAP(newInput, oldInput);
            }
        }
    }
    
    return 0;
}