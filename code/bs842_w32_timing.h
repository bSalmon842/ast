/*
File: bs842_w32_timing.h
Author: Brock Salmon
Notice: Public Domain

// This software is dual-licensed to the public domain and under the following
// license: you are granted a perpetual, irrevocable license to copy, modify,
// publish, and distribute this file as you see fit.
*/


/* USAGE
To include the file:
#define BS842_W32TIMING_IMPLEMENTATION
#include "bs842_w32_timing.h"

Optional Defines:
These defines should be placed before the IMPLEMENTATION define
#define BS842_ASSERT(check) to use your own assert function or define it as empty to disable failed initialisation asserts
#define BS842_DONT_FETCH_WINMM if you are statically linking winmm.lib, otherwise the lib will get timeBeginPeriod from winmm.dll
*/

#if 0
//Example Code:
#define BS842_W32TIMING_IMPLEMENTATION
#include "bs842_w32_timing.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCode)
{
    BS842_Timing_Init();
    
    // Window init...
    // Get Device Context...
    BS842_Timing_ChangeRefreshRate(deviceContext, true, 60);
    
    // Main Loop
    while (running)
    {
        // Do all frame work...
        
        BS842_Timing_FrameEnd();
        
        // To output Frame Metrics
        printf("%.02f / %.03f\n", BS842_Timing_GetFramesPerSecond(), BS842_Timing_GetMSPerFrame());
    }
    
    return 0;
}
#endif

/*
API Usage:
-- LARGE_INTEGER BS842_Timing_GetClock() --
Queries the Performance Counter to return a LARGE_INTEGER meant to be used in BS842_Timing_GetSecondsElapsed.

    -- float BS842_Timing_GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end) --
Returns the seconds between 2 LARGE_INTEGERs as a floating point value
    
     -- void BS842_Timing_Init() --
Initialises the Performance Counter Frequency and the scheduler granularity, should be put near the start of the program

     -- void BS842_Timing_ChangeRefreshRate(HDC deviceContext, bool vsync, int refreshRateHz) --
    Sets the target frame rate using the value passed through refreshRateHz as long as vsync is false, if set to 0 the program will
run at an unlimited rate.
If vsync is set to true, refreshRateHz is ignored and GetDeviceCaps is called using the deviceContext to get the refresh rate of the
primary monitor.

     -- void BS842_Timing_FrameEndWork() --
    Handles sleeping the software until the current frame time is matched to the target frame time, does not handle going
slower than the target frame rate.
Ideally should be placed immediately after all work for the frame is completed.

     -- bool BS842_Timing_IsVSyncEnabled() --
Returns a bool representing if VSync has been enabled

     -- int BS842_Timing_GetRefreshRate() --
Returns an integer representing the target refresh rate in Hz. (60 by default)

     -- float BS842_Timing_GetDeltaTime() --
Returns the last time between calls to BS842_Timing_FrameEndWork as a floating point in seconds

     -- float BS842_Timing_GetMSPerFrame() --
Returns the last time between calls to BS842_Timing_FrameEndWork as a floating point in milliseconds

     -- float BS842_Timing_GetFramesPerSecond() --
Returns the current Frames Per Second value based on the MS Per Frame value

*/

#ifndef BS842_INCLUDE_BS842_W32TIMING_H

#ifdef __cplusplus
extern "C"
{
#endif
    
#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
    
#ifndef _TIMERAPI_H_
#include <timeapi.h>
#endif
    
#ifndef BS842_DONT_FETCH_WINMM
#define TIME_BEGIN_PERIOD(funcName) MMRESULT WINAPI funcName(UINT uPeriod)
    typedef TIME_BEGIN_PERIOD(time_begin_period);
    TIME_BEGIN_PERIOD(timeBeginPeriodStub)
    {
        // Stub, so do nothing
        (void) uPeriod;
        return MMSYSERR_NOTENABLED;
    }
    static time_begin_period *timeBeginPeriod_ = timeBeginPeriodStub;
#define timeBeginPeriod timeBeginPeriod_
    
    HMODULE winmmLib = LoadLibraryA("winmm.dll");
#endif
    
#ifndef BSDEF
#ifdef BS842_W32TIMING_STATIC
#define BSDEF static
#else
#define BSDEF extern
#endif
#endif
    
    typedef struct
    {
        bool initialised;
        long long performanceCounterFrequency;
        bool sleepIsGranular;
        bool vsync;
        int refreshRateHz;
        float targetSecondsPerFrame;
        LARGE_INTEGER lastCounter;
        float lastSecPerFrame;
        float msPerFrame;
        float framesPerSecond;
    } BS842_Timing_Internal_Info;
    
    static BS842_Timing_Internal_Info bs842_timing_internal_info = {};
    
#ifndef BS842_ASSERT
#define BS842_ASSERT(check) if(!(check)) {*(int *)0 = 0;}
#endif
    
#ifndef _MSC_VER
#error BS842_W32_Timing only compiles on MSVC
#endif
    
    BSDEF LARGE_INTEGER BS842_Timing_GetClock();
    BSDEF float         BS842_Timing_GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end);
    
    BSDEF void BS842_Timing_Init();
    BSDEF void BS842_Timing_ChangeRefreshRate(HDC deviceContext, bool vsync, int refreshRateHz);
    
    BSDEF void BS842_Timing_FrameEndWork();
    
    BSDEF bool  BS842_Timing_IsVSyncEnabled();
    BSDEF int   BS842_Timing_GetRefreshRate();
    BSDEF float BS842_Timing_GetDeltaTime();
    BSDEF float BS842_Timing_GetMSPerFrame();
    BSDEF float BS842_Timing_GetFramesPerSecond();
    
#ifdef __cplusplus
}
#endif

#define BS842_INCLUDE_BS842_W32_TIMING_H
#endif // BS842_INCLUDE_BS842_W32_TIMING_H

#ifdef BS842_W32TIMING_IMPLEMENTATION

BSDEF LARGE_INTEGER BS842_Timing_GetClock()
{
    BS842_ASSERT(bs842_timing_internal_info.initialised);
    
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

BSDEF float BS842_Timing_GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    BS842_ASSERT(bs842_timing_internal_info.initialised);
    
    float result = (float)(end.QuadPart - start.QuadPart) / (float)bs842_timing_internal_info.performanceCounterFrequency;
    return result;
}

BSDEF void BS842_Timing_Init()
{
#ifndef BS842_DONT_FETCH_WINMM
    timeBeginPeriod = (time_begin_period *)GetProcAddress(winmmLib, "timeBeginPeriod");
#endif
    
    LARGE_INTEGER perfCountFreqResult;
    QueryPerformanceFrequency(&perfCountFreqResult);
    bs842_timing_internal_info.performanceCounterFrequency = perfCountFreqResult.QuadPart;
    
    unsigned int schedulerGranularity = 1;
    bs842_timing_internal_info.sleepIsGranular = (timeBeginPeriod(schedulerGranularity) == TIMERR_NOERROR);
    
    bs842_timing_internal_info.initialised = true;
    
    bs842_timing_internal_info.lastCounter = BS842_Timing_GetClock();
    bs842_timing_internal_info.lastSecPerFrame = 1.0f / 60.0f;
}

BSDEF void BS842_Timing_ChangeRefreshRate(HDC deviceContext, bool vsync, int refreshRateHz)
{
    BS842_ASSERT(bs842_timing_internal_info.initialised);
    
    bs842_timing_internal_info.vsync = vsync;
    int monitorRefreshRate = 60;
    
    if (vsync)
    {
        s32 w32RefreshRate = GetDeviceCaps(deviceContext, VREFRESH);
        if (w32RefreshRate > 1)
        {
            monitorRefreshRate = w32RefreshRate;
        }
    }
    
    if (refreshRateHz != 0 || vsync)
    {
        bs842_timing_internal_info.targetSecondsPerFrame = 1.0f / (float)monitorRefreshRate;
    }
    else
    {
        bs842_timing_internal_info.targetSecondsPerFrame = 0.0f;
    }
    
    bs842_timing_internal_info.refreshRateHz = monitorRefreshRate;
}

BSDEF void BS842_Timing_FrameEnd()
{
    BS842_ASSERT(bs842_timing_internal_info.initialised);
    
    LARGE_INTEGER workCounter = BS842_Timing_GetClock();
    float workSecondsElapsed = BS842_Timing_GetSecondsElapsed(bs842_timing_internal_info.lastCounter, workCounter);
    
    float secondsElapsedForFrame = workSecondsElapsed;
    if (secondsElapsedForFrame < bs842_timing_internal_info.targetSecondsPerFrame && bs842_timing_internal_info.targetSecondsPerFrame != 0.0f)
    {
        if (bs842_timing_internal_info.sleepIsGranular)
        {
            DWORD sleepMS = (DWORD)(1000.0f * (bs842_timing_internal_info.targetSecondsPerFrame - secondsElapsedForFrame));
            if (sleepMS > 0)
            {
                Sleep(sleepMS - 1);
            }
        }
        
        while (secondsElapsedForFrame < bs842_timing_internal_info.targetSecondsPerFrame)
        {
            secondsElapsedForFrame = BS842_Timing_GetSecondsElapsed(bs842_timing_internal_info.lastCounter, BS842_Timing_GetClock());
        }
    }
    
    LARGE_INTEGER endCounter = BS842_Timing_GetClock();
    bs842_timing_internal_info.lastSecPerFrame = BS842_Timing_GetSecondsElapsed(bs842_timing_internal_info.lastCounter, endCounter);
    bs842_timing_internal_info.msPerFrame = 1000.0f * BS842_Timing_GetSecondsElapsed(bs842_timing_internal_info.lastCounter, endCounter);
    bs842_timing_internal_info.framesPerSecond = 1.0f / BS842_Timing_GetSecondsElapsed(bs842_timing_internal_info.lastCounter, endCounter);
    
    bs842_timing_internal_info.lastCounter = endCounter;
    
}

BSDEF bool BS842_Timing_IsVSyncEnabled()
{
    BS842_ASSERT(bs842_timing_internal_info.initialised);
    return bs842_timing_internal_info.vsync;
}

BSDEF int BS842_Timing_GetRefreshRate()
{
    BS842_ASSERT(bs842_timing_internal_info.initialised);
    return bs842_timing_internal_info.refreshRateHz;
}

BSDEF float BS842_Timing_GetDeltaTime()
{
    BS842_ASSERT(bs842_timing_internal_info.initialised);
    return bs842_timing_internal_info.lastSecPerFrame;
}

BSDEF float BS842_Timing_GetMSPerFrame()
{
    BS842_ASSERT(bs842_timing_internal_info.initialised);
    return bs842_timing_internal_info.msPerFrame;
}

BSDEF float BS842_Timing_GetFramesPerSecond()
{
    BS842_ASSERT(bs842_timing_internal_info.initialised);
    return bs842_timing_internal_info.framesPerSecond;
}

#endif // BS842_W32TIMING_IMPLEMENTATION
