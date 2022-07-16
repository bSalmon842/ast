/*
Project: Asteroids
File: w32_ast.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mmdeviceapi.h>
#include <AudioClient.h>
#include <direct.h>
#include <gl/gl.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif // MSC_VER

#include "ast_platform.h"
#include "w32_ast_dev.cpp"

#include "w32_ast.h"

#define BS842_DONT_FETCH_WINMM
#define BS842_W32TIMING_IMPLEMENTATION
#include "bs842_w32_timing.h"

global b32 globalRunning;
global HDC globalDeviceContext;
global HGLRC globalGLContext;

#include "w32_ast_opengl.cpp"
#include "w32_ast_thread.cpp"

function PLATFORM_MEM_ALLOC(W32_MemAlloc)
{
    void *result = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return result;
}

function PLATFORM_MEM_FREE(W32_MemFree)
{
    if (memory)
    {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

function PLATFORM_MICROSECONDS_SINCE_EPOCH(W32_MicrosecondsSinceEpoch)
{
    u64 result = 0;
    
    FILETIME fileTime = {};
    SYSTEMTIME systemTime = {};
    GetSystemTime(&systemTime);
    SystemTimeToFileTime(&systemTime, &fileTime);
    
    ULARGE_INTEGER fileTimeInt = {};
    fileTimeInt.LowPart = fileTime.dwLowDateTime;
    fileTimeInt.HighPart = fileTime.dwHighDateTime;
    
    SYSTEMTIME epoch = {};
    epoch.wYear = 1970;
    epoch.wMonth = 1;
    epoch.wDayOfWeek = 4;
    epoch.wDay = 1;
    
    SystemTimeToFileTime(&epoch, &fileTime);
    
    ULARGE_INTEGER epochFileTime = {};
    epochFileTime.LowPart = fileTime.dwLowDateTime;
    epochFileTime.HighPart = fileTime.dwHighDateTime;
    
    result = (fileTimeInt.QuadPart - epochFileTime.QuadPart) / 10;
    
    return result;
}

function PLATFORM_SECONDS_SINCE_EPOCH(W32_SecondsSinceEpoch)
{
    u64 result = 0;
    
    FILETIME fileTime = {};
    SYSTEMTIME systemTime = {};
    GetSystemTime(&systemTime);
    SystemTimeToFileTime(&systemTime, &fileTime);
    
    ULARGE_INTEGER fileTimeInt = {};
    fileTimeInt.LowPart = fileTime.dwLowDateTime;
    fileTimeInt.HighPart = fileTime.dwHighDateTime;
    
    SYSTEMTIME epoch = {};
    epoch.wYear = 1970;
    epoch.wMonth = 1;
    epoch.wDayOfWeek = 4;
    epoch.wDay = 1;
    
    SystemTimeToFileTime(&epoch, &fileTime);
    
    ULARGE_INTEGER epochFileTime = {};
    epochFileTime.LowPart = fileTime.dwLowDateTime;
    epochFileTime.HighPart = fileTime.dwHighDateTime;
    
    result = (fileTimeInt.QuadPart - epochFileTime.QuadPart) / 10000000;
    
    return result;
}

//~ AUDIO
#define CHECK_COM_FAILURE_ASSERT(func) if (FAILED(func)) {ASSERT(!"Error");}
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

//~ FILE I/O
function PLATFORM_GET_ALL_FILES_OF_EXT_BEGIN(W32_GetAllFilesOfExtBegin)
{
    Platform_FileGroup result = {};
    
    W32_FileGroup *w32FileGroup = (W32_FileGroup *)VirtualAlloc(0, sizeof(W32_FileGroup), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    result.platform = w32FileGroup;
    
    wchar_t *searchName = L"*.*";
    switch (fileType)
    {
        case PlatformFileType_Asset:
        {
            searchName = L"*.aaf";
        } break;
        
        case PlatformFileType_Save:
        {
            searchName = L"*.asf";
        } break;
        
        INVALID_DEFAULT;
    }
    
    WIN32_FIND_DATAW findData = {};
    HANDLE findHandle = FindFirstFileW(searchName, &findData);
    
    while (findHandle != INVALID_HANDLE_VALUE)
    {
        ++result.fileCount;
        
        if (!FindNextFileW(findHandle, &findData))
        {
            FindClose(findHandle);
            break;
        }
    }
    
    w32FileGroup->findHandle = FindFirstFileW(searchName, &w32FileGroup->findData);
    
    return result;
}

function PLATFORM_GET_ALL_FILES_OF_EXT_END(W32_GetAllFilesOfExtEnd)
{
    W32_FileGroup *w32FileGroup = (W32_FileGroup *)fileGroup->platform;
    if (w32FileGroup)
    {
        if (w32FileGroup->findHandle != INVALID_HANDLE_VALUE)
        {
            FindClose(w32FileGroup->findHandle);
        }
        
        VirtualFree(w32FileGroup, 0, MEM_RELEASE);
    }
}

function PLATFORM_OPEN_NEXT_FILE(W32_OpenNextFile)
{
    Platform_FileHandle result = {};
    
    W32_FileGroup *w32FileGroup = (W32_FileGroup *)fileGroup->platform;
    W32_FileHandle *w32FileHandle = 0;
    
    if (w32FileGroup->findHandle != INVALID_HANDLE_VALUE)
    {
        w32FileHandle = (W32_FileHandle *)VirtualAlloc(0, sizeof(W32_FileHandle), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        result.platform = w32FileHandle;
        
        if (w32FileHandle)
        {
            w32FileHandle->w32Handle = CreateFileW(w32FileGroup->findData.cFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
            result.errored = (w32FileHandle->w32Handle == INVALID_HANDLE_VALUE);
        }
        
        if (!FindNextFileW(w32FileGroup->findHandle, &w32FileGroup->findData))
        {
            FindClose(w32FileGroup->findHandle);
            w32FileGroup->findHandle = INVALID_HANDLE_VALUE;
        }
    }
    
    return result;
}

function PLATFORM_MARK_FILE_ERROR(W32_MarkFileError)
{
#if AST_INTERNAL
    printf("FILE ERROR: %s\n", errorMsg);
#endif
    
    fileHandle->errored = true;
}

function PLATFORM_READ_DATA_FROM_FILE(W32_ReadDataFromFile)
{
    if (Platform_NoFileErrors(fileHandle))
    {
        W32_FileHandle *handle = (W32_FileHandle *)fileHandle->platform;
        OVERLAPPED overlapped = {};
        overlapped.Offset = (u32)(offset & 0xFFFFFFFF);
        overlapped.OffsetHigh = (u32)((offset >> 32) & 0xFFFFFFFF);
        
        u32 fileSize = SafeTruncateU64(size);
        
        DWORD bytesRead;
        if (!ReadFile(handle->w32Handle, dest, fileSize, &bytesRead, &overlapped) || !(fileSize == bytesRead))
        {
            W32_MarkFileError(fileHandle, "Failed to Read Data from File");
        }
    }
}

function PLATFORM_OPEN_FILE_FOR_WRITE(W32_OpenFileForWrite)
{
    Platform_FileHandle result = {};
    
    W32_FileHandle *w32FileHandle = (W32_FileHandle *)VirtualAlloc(0, sizeof(W32_FileHandle), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (w32FileHandle)
    {
        if (writeType == PlatformWriteType_Overwrite)
        {
            w32FileHandle->w32Handle = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
        }
        else if (writeType == PlatformWriteType_Append)
        {
            w32FileHandle->w32Handle = CreateFileA(filename, FILE_APPEND_DATA, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
        }
        else
        {
            INVALID_CODE_PATH;
        }
        result.platform = w32FileHandle;
        result.errored = (w32FileHandle->w32Handle == INVALID_HANDLE_VALUE);
    }
    
    return result;
}

function PLATFORM_WRITE_INTO_FILE(W32_WriteIntoFile)
{
    W32_FileHandle *w32FileHandle = (W32_FileHandle *)fileHandle.platform;
    if (w32FileHandle)
    {
        va_list args;
        va_start(args, fmtStr);
        
        char string[128];
        s32 bytesToWrite = stbsp_vsprintf(string, fmtStr, args);
        
        va_end(args);
        
        WriteFile(w32FileHandle->w32Handle, string, bytesToWrite, 0, 0);
    }
}

function PLATFORM_CLOSE_FILE(W32_CloseFile)
{
    W32_FileHandle *w32FileHandle = (W32_FileHandle *)fileHandle->platform;
    if (w32FileHandle)
    {
        CloseHandle(w32FileHandle->w32Handle);
        VirtualFree(w32FileHandle, 0, MEM_RELEASE);
        fileHandle->platform = 0;
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

function void W32_ProcessMouseButtonEvent(Game_ButtonState *state, b32 keyIsDown)
{
    if (state->endedFrameDown != keyIsDown)
    {
        state->endedFrameDown = keyIsDown;
        ++state->halfTransitionCount;
    }
}

//~ RENDERING / OPENGL
function void W32_InitOpenGL(HDC deviceContext, HGLRC *glContext)
{
    globalGLContext = 0;
    
    OpenGL_Info glInfo = {};
    WGL_LoadWGLExtensions(&glInfo.support_SRGB);
    WGL_SetPixelFormat(deviceContext, glInfo.support_SRGB);
    HGLRC glRC = wglCreateContext(deviceContext);
    if (wglMakeCurrent(deviceContext, glRC))
    {
        if (wglCreateContextAttribsARB)
        {
            HGLRC shareContext = 0;
            HGLRC modernGLContext = wglCreateContextAttribsARB(deviceContext, shareContext, globalWGLContextAttribs);
            if (modernGLContext)
            {
                if (wglMakeCurrent(deviceContext, modernGLContext))
                {
                    WGL_LoadGLFunctions(&glInfo);
                    OpenGL_GetInfo(&glInfo);
                }
                wglDeleteContext(glRC);
                *glContext = modernGLContext;
                printf("Got Modern GL Context\n");
            }
            else
            {
                ASSERT(false);
            }
        }
        else
        {
            ASSERT(false);
        }
        
        wglSwapInterval = (wgl_swapIntervalEXT *)wglGetProcAddress("wglSwapIntervalEXT");
        wglGetSwapInterval = (wgl_getSwapIntervalEXT *)wglGetProcAddress("wglGetSwapIntervalEXT");
        if (wglSwapInterval && wglGetSwapInterval)
        {
            wglSwapInterval(0);
        }
        else
        {
            ASSERT(false);
        }
    }
    else
    {
        ASSERT(false);
    }
    
    OpenGL_Init(&glInfo);
}

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
    backBuffer->info.bmiHeader.biHeight = backBuffer->height;
    backBuffer->info.bmiHeader.biPlanes = 1;
    backBuffer->info.bmiHeader.biBitCount = 32;
    backBuffer->info.bmiHeader.biCompression = BI_RGB;
    
    s32 bitmapMemSize = backBuffer->pitch * backBuffer->height;
    backBuffer->memory = VirtualAlloc(0, bitmapMemSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

function void W32_PresentBuffer(Game_RenderCommands *commands, PlatformAPI platform, HDC deviceContext)
{
    OpenGL_Render(commands, platform);
    SwapBuffers(deviceContext);
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
function PLATFORM_DEBUG_SYSTEM_COMMAND(W32_DebugSystemCommand)
{
    STARTUPINFO startInfo = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION processInfo;
    
    if (!CreateProcess("C:\\Windows\\System32\\cmd.exe", command, 0, 0, 0, 0, 0, path, &startInfo, &processInfo))
    {
        ASSERT(false);
    }
}

function void W32_ProcessPendingMessages(HWND window, Game_Mouse *mouse, Game_Keyboard *keyboard, Game_RenderCommands *renderCommands, PlatformAPI platform)
{
    MSG msg;
    
    while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
    {
        switch (msg.message)
        {
            case WM_QUIT:
            {
                globalRunning = false;
            } break;
            
            case WM_PAINT:
            {
                PAINTSTRUCT paint;
                HDC deviceContext = BeginPaint(window, &paint);
                
                W32_PresentBuffer(renderCommands, platform, deviceContext);
                EndPaint(window, &paint);
            } break;
            
            case WM_LBUTTONDOWN:
            {
                W32_ProcessMouseButtonEvent(&mouse->buttons[MouseButton_L], true);
            } break;
            case WM_LBUTTONUP:
            {
                W32_ProcessMouseButtonEvent(&mouse->buttons[MouseButton_L], false);
            } break;
            
            case WM_MBUTTONDOWN:
            {
                W32_ProcessMouseButtonEvent(&mouse->buttons[MouseButton_M], true);
            } break;
            case WM_MBUTTONUP:
            {
                W32_ProcessMouseButtonEvent(&mouse->buttons[MouseButton_M], false);
            } break;
            
            case WM_RBUTTONDOWN:
            {
                W32_ProcessMouseButtonEvent(&mouse->buttons[MouseButton_R], true);
            } break;
            case WM_RBUTTONUP:
            {
                W32_ProcessMouseButtonEvent(&mouse->buttons[MouseButton_R], false);
            } break;
            
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
#define ProcessKey(vk, key) if (vkCode == vk) { W32_ProcessKeyboardEvent(&keyboard->key.state, keyIsDown); }
#define ProcessASCIIKey(c) if ((s32)vkCode == *(#c)) { W32_ProcessKeyboardEvent(&keyboard->key##c.state, keyIsDown); keyboard->key##c.value = *(#c);}
#define ProcessASCIIVK(vk, key, val) if ((s32)vkCode ==  vk) { W32_ProcessKeyboardEvent(&keyboard->key.state, keyIsDown); keyboard->key.value = val;}
                    ProcessASCIIKey(A);
                    ProcessASCIIKey(B);
                    ProcessASCIIKey(C);
                    ProcessASCIIKey(D);
                    ProcessASCIIKey(E);
                    ProcessASCIIKey(F);
                    ProcessASCIIKey(G);
                    ProcessASCIIKey(H);
                    ProcessASCIIKey(I);
                    ProcessASCIIKey(J);
                    ProcessASCIIKey(K);
                    ProcessASCIIKey(L);
                    ProcessASCIIKey(M);
                    ProcessASCIIKey(N);
                    ProcessASCIIKey(O);
                    ProcessASCIIKey(P);
                    ProcessASCIIKey(Q);
                    ProcessASCIIKey(R);
                    ProcessASCIIKey(S);
                    ProcessASCIIKey(T);
                    ProcessASCIIKey(U);
                    ProcessASCIIKey(V);
                    ProcessASCIIKey(W);
                    ProcessASCIIKey(X);
                    ProcessASCIIKey(Y);
                    ProcessASCIIKey(Z);
                    ProcessASCIIKey(0);
                    ProcessASCIIKey(1);
                    ProcessASCIIKey(2);
                    ProcessASCIIKey(3);
                    ProcessASCIIKey(4);
                    ProcessASCIIKey(5);
                    ProcessASCIIKey(6);
                    ProcessASCIIKey(7);
                    ProcessASCIIKey(8);
                    ProcessASCIIKey(9);
                    ProcessASCIIVK(VK_SPACE, keySpace, ' ');
                    ProcessASCIIVK(VK_OEM_1, keySemicolon, ';');
                    ProcessASCIIVK(VK_OEM_MINUS, keyMinus, '-');
                    ProcessASCIIVK(VK_OEM_PERIOD, keyPeriod, '.');
                    ProcessKey(VK_ESCAPE, keyEsc);
                    ProcessKey(VK_BACK, keyBackspace);
                    ProcessKey(VK_RETURN, keyEnter);
                    ProcessKey(VK_OEM_3, keyTilde);
                    ProcessKey(VK_F1, keyF1);
                    ProcessKey(VK_F2, keyF2);
                    ProcessKey(VK_F3, keyF3);
                    ProcessKey(VK_F4, keyF4);
                    ProcessKey(VK_F5, keyF5);
                    ProcessKey(VK_UP, keyUp);
                    ProcessKey(VK_DOWN, keyDown);
                    ProcessKey(VK_LEFT, keyLeft);
                    ProcessKey(VK_RIGHT, keyRight);
                    ProcessKey(VK_SHIFT, keyShift);
                    ProcessKey(VK_MENU, keyAlt);
                    ProcessKey(VK_CONTROL, keyCtrl);
                    
                    if (keyIsDown)
                    {
                        if (keyboard->keyAlt.state.endedFrameDown && vkCode == VK_F4)
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
            globalRunning = false;
        }break;
        
        case WM_DESTROY:
        {
            globalRunning = false;
        }break;
        
        case WM_PAINT: { } break;
        
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

#if AST_INTERNAL
Game_Memory *debugGlobalMem;
#endif
s32 WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, s32 showCode)
{
    _chdir("..\\data");
    
#if AST_INTERNAL
    AllocConsole();
    s32 hConsole = 0;
    FILE *fp;
    fp = _fdopen(hConsole, "w");
    
    freopen_s(&fp, "CONOUT$", "w", stdout);
#endif
    
    BS842_Timing_Init();
    
    W32_State platformState = {};
    W32_GetExeFileName(&platformState);
    
    char programCodeDLLPath[W32_FILEPATH_MAX_LENGTH];
    char tempDLLPath[W32_FILEPATH_MAX_LENGTH];
    char lockPath[W32_FILEPATH_MAX_LENGTH];
    
    W32_BuildExePathFileName(&platformState, "ast.dll", programCodeDLLPath);
    W32_BuildExePathFileName(&platformState, "ast_temp.dll", tempDLLPath);
    W32_BuildExePathFileName(&platformState, "lock.tmp", lockPath);
    
    W32_BackBuffer platformBackBuffer = {};
    W32_ResizeDIBSection(&platformBackBuffer, 1000, 1000);
    
    WNDCLASSA windowClass = {};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = W32_WndProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "AsteroidsWindowClass";
    
    if (RegisterClassA(&windowClass))
    {
        RECT windowRect = {0, 0, 1000, 1000};
        u32 windowFlags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        AdjustWindowRect(&windowRect, windowFlags, FALSE);
        HWND window = CreateWindowExA(0,
                                      windowClass.lpszClassName, "Asteroids",
                                      windowFlags,
                                      CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
                                      0, 0, instance, 0);
        
        if (window)
        {
            globalDeviceContext = GetDC(window);
            W32_InitOpenGL(globalDeviceContext, &globalGLContext);
            
            W32_Thread threads[7];
            s32 threadCount = ARRAY_COUNT(threads);
            Platform_ParallelQueue parallelQueue = {};
            parallelQueue.semaphore = CreateSemaphoreEx(0, 0, threadCount, 0, 0, SEMAPHORE_ALL_ACCESS);
            for (s32 i = 0; i < threadCount; ++i)
            {
                W32_Thread *thread = &threads[i];
                thread->threadIndex = i;
                thread->queue = &parallelQueue;
                
                DWORD threadID;
                HANDLE threadHandle = CreateThread(0, 0, ThreadProc, thread, 0, &threadID);
                CloseHandle(threadHandle);
            }
            
            s32 audioLatencyFrames = 1;
            W32_AudioOutput audioOutput = {};
            audioOutput.samplesPerSecond = 48000;
            audioOutput.bytesPerSample = sizeof(s16) * 2;
            audioOutput.secondaryBufferSize = audioOutput.samplesPerSecond;
            audioOutput.latencySampleCount = audioLatencyFrames * (audioOutput.samplesPerSecond / 60);
            
            W32_InitWASAPI(&audioOutput.audioClient, &audioOutput.renderClient, audioOutput.samplesPerSecond, audioOutput.secondaryBufferSize);
            audioOutput.audioClient->Start();
            
            globalRunning = true;
            
            W32_ProgramCode programCode = W32_LoadProgramCode(programCodeDLLPath, tempDLLPath, lockPath);
            
            Game_Input inputs[2] = {};
            Game_Input *newInput = &inputs[0];
            Game_Input *oldInput = &inputs[1];
            
            s16 *samples = (s16 *)VirtualAlloc(0, audioOutput.secondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
#if AST_INTERNAL
            LPVOID baseAddress = (LPVOID)TERABYTE(2);
#else
            LPVOID baseAddress = 0;
#endif
            
            Game_Memory gameMem = {};
            PlatformAPI platform = {};
            platform.MemAlloc = W32_MemAlloc;
            platform.MemFree = W32_MemFree;
            platform.MicrosecondsSinceEpoch = W32_MicrosecondsSinceEpoch;
            platform.SecondsSinceEpoch = W32_SecondsSinceEpoch;
            platform.GetAllFilesOfExtBegin = W32_GetAllFilesOfExtBegin;
            platform.GetAllFilesOfExtEnd = W32_GetAllFilesOfExtEnd;
            platform.OpenNextFile = W32_OpenNextFile;
            platform.MarkFileError = W32_MarkFileError;
            platform.ReadDataFromFile = W32_ReadDataFromFile;
            platform.OpenFileForWrite = W32_OpenFileForWrite;
            platform.WriteIntoFile = W32_WriteIntoFile;
            platform.CloseFile = W32_CloseFile;
            platform.AddParallelEntry = W32_AddParallelEntry;
            platform.CompleteAllParallelWork = W32_CompleteAllParallelWork;
            platform.HasParallelWorkFinished = W32_HasParallelWorkFinished;
            platform.AllocTexture = WGL_AllocTexture;
            platform.FreeTexture = WGL_FreeTexture;
            
            platform.DebugSystemCommand = W32_DebugSystemCommand;
            
            gameMem.platform = platform;
            gameMem.parallelQueue = &parallelQueue;
            
            s32 cpuInfo[4];
            __cpuid(cpuInfo, 1);
            gameMem.availableInstructionSets.sse3 = cpuInfo[2] & (1 << 0) || false;
            gameMem.availableInstructionSets.sse4_2 = cpuInfo[2] & (1 << 20) || false;
            gameMem.availableInstructionSets.avx = cpuInfo[2] & (1 << 28) || false;
            
            gameMem.storage[PERMA_STORAGE_INDEX].size = MEGABYTE(16);
            gameMem.storage[TRANS_STORAGE_INDEX].size = MEGABYTE(512);
            gameMem.storage[DEBUG_STORAGE_INDEX].size = MEGABYTE(128);
            ASSERT(sizeof(DebugState) <= gameMem.storage[DEBUG_STORAGE_INDEX].size);
            
            u64 totalMemSize = 
                gameMem.storage[PERMA_STORAGE_INDEX].size + 
                gameMem.storage[TRANS_STORAGE_INDEX].size + 
                gameMem.storage[DEBUG_STORAGE_INDEX].size;
            gameMem.storage[PERMA_STORAGE_INDEX].ptr = VirtualAlloc(baseAddress, totalMemSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            gameMem.storage[TRANS_STORAGE_INDEX].ptr = (u8 *)gameMem.storage[PERMA_STORAGE_INDEX].ptr + gameMem.storage[PERMA_STORAGE_INDEX].size;
            gameMem.storage[DEBUG_STORAGE_INDEX].ptr = (u8 *)gameMem.storage[TRANS_STORAGE_INDEX].ptr + gameMem.storage[TRANS_STORAGE_INDEX].size;
            
            globalDebugState = (DebugState *)gameMem.storage[DEBUG_STORAGE_INDEX].ptr;
            if (programCode.InitialiseDebugState)
            {
                programCode.InitialiseDebugState(&gameMem);
            }
            
            if (samples && gameMem.storage[PERMA_STORAGE_INDEX].ptr && gameMem.storage[TRANS_STORAGE_INDEX].ptr)
            {
                s32 debugTimeMarkerIndex = 0;
                W32_DebugTimeMarker debugTimeMarkers[60] = {};
                
                u32 lastPlayCursor = 0;
                b32 soundValid = false;
                
                usize maxPushBufferSize = MEGABYTE(4);
                void *pushBufferBase = VirtualAlloc(0, maxPushBufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                Game_RenderCommands gameRenderCommands = InitialiseRenderCommands(maxPushBufferSize, pushBufferBase,
                                                                                  platformBackBuffer.width, platformBackBuffer.height);
                
                LARGE_INTEGER lastCounter = BS842_Timing_GetClock();
                f32 lastSecPerFrame = 0.16f;
                while (globalRunning)
                {
#if AST_INTERNAL
                    FILETIME newDLLWriteTime = W32_GetFileLastWriteTime(programCodeDLLPath);
                    if (CompareFileTime(&newDLLWriteTime, &programCode.dllLastWriteTime) != 0)
                    {
                        // NOTE(bSalmon): Flush frame debug info
                        for (u32 frameIndex = 0; frameIndex < MAX_DEBUG_FRAMES; ++frameIndex)
                        {
                            DebugFrame *frame = &globalDebugState->table->frames[frameIndex];
                            
                            for (u32 translationIndex = 0; translationIndex < TRANSLATION_UNIT_COUNT; ++translationIndex)
                            {
                                for (u32 blockIndex = 0; blockIndex < MAX_DEBUG_TRANSLATION_UNIT_INFOS; ++blockIndex)
                                {
                                    frame->blockInfos[translationIndex][blockIndex] = {};
                                }
                            }
                        }
                        
                        Sleep(100);
                        
                        W32_UnloadProgramCode(&programCode);
                        programCode = W32_LoadProgramCode(programCodeDLLPath, tempDLLPath, lockPath);
                    }
#endif
                    LARGE_INTEGER frameStartCounter = BS842_Timing_GetClock();
                    DEBUG_FRAME_START;
                    DEBUG_BLOCK_OPEN(MainLoop);
                    
                    newInput->deltaTime = lastSecPerFrame;
                    newInput->exeReloaded = false;
                    
                    Game_Keyboard *oldKeyboard = &oldInput->keyboard;
                    Game_Keyboard *newKeyboard = &newInput->keyboard;
                    *newKeyboard = {};
                    newKeyboard->isConnected = true;
                    
                    for (s32 keyIndex = 0; keyIndex < ARRAY_COUNT(newKeyboard->keys); ++keyIndex)
                    {
                        newKeyboard->keys[keyIndex].state.endedFrameDown = oldKeyboard->keys[keyIndex].state.endedFrameDown;
                    }
                    
                    Game_Mouse *oldMouse = &oldInput->mouse;
                    Game_Mouse *newMouse = &newInput->mouse;
                    *newMouse = {};
                    
                    for (s32 mbIndex = 0; mbIndex < ARRAY_COUNT(newMouse->buttons); ++mbIndex)
                    {
                        newMouse->buttons[mbIndex].endedFrameDown = oldMouse->buttons[mbIndex].endedFrameDown;
                    }
                    
                    POINT mouseLoc;
                    GetCursorPos(&mouseLoc);
                    ScreenToClient(window, &mouseLoc);
                    newInput->mouse.x = mouseLoc.x;
                    newInput->mouse.y = gameRenderCommands.height - mouseLoc.y;
                    
                    W32_ProcessPendingMessages(window, &newInput->mouse, newKeyboard, &gameRenderCommands, platform);
                    
#if AST_INTERNAL
                    debugGlobalMem = &gameMem;
                    DEBUG_FRAME_MARKER(Input);
#endif
                    s32 samplesToWrite = 0;
                    u32 audioPadding;
                    if (SUCCEEDED(audioOutput.audioClient->GetCurrentPadding(&audioPadding)))
                    {
                        samplesToWrite = (s32)(audioOutput.secondaryBufferSize - audioPadding);
                        if (samplesToWrite > audioOutput.latencySampleCount)
                        {
                            samplesToWrite = audioOutput.latencySampleCount;
                        }
                    }
                    
                    Game_AudioBuffer gameAudioBuffer = {};
                    gameAudioBuffer.samplesPerSecond = audioOutput.samplesPerSecond;
                    gameAudioBuffer.sampleCount = Align8(samplesToWrite);
                    gameAudioBuffer.samples = samples;
                    
                    W32_FillAudioBuffer(&audioOutput, samplesToWrite, &gameAudioBuffer);
                    
#if AST_INTERNAL
                    DEBUG_FRAME_MARKER(Audio);
#endif
                    
                    W32_WindowDims resizeCheck = W32_GetWindowDims(window);
                    if (resizeCheck.width != gameRenderCommands.width ||
                        resizeCheck.height != gameRenderCommands.height)
                    {
                        gameRenderCommands.width = resizeCheck.width;
                        gameRenderCommands.height = resizeCheck.height;
                    }
                    
                    if (programCode.UpdateRender)
                    {
                        programCode.UpdateRender(&gameRenderCommands, &gameMem, newInput, &gameAudioBuffer);
                    }
                    
#if AST_INTERNAL
                    DEBUG_FRAME_MARKER(GameUpdate);
#endif
                    
                    //W32_RenderAudioSyncDisplay(&platformBackBuffer, ARRAY_COUNT(debugTimeMarkers), debugTimeMarkers, &audioOutput, BS842_Timing_GetTargetSecondsPerFrame());
                    
                    W32_PresentBuffer(&gameRenderCommands, platform, globalDeviceContext);
                    
                    SWAP(newInput, oldInput);
                    
#if AST_INTERNAL
                    DEBUG_FRAME_MARKER(FrameComplete);
                    DEBUG_BLOCK_CLOSE(MainLoop);
#endif
                    
                    LARGE_INTEGER endCounter = BS842_Timing_GetClock();
                    lastSecPerFrame = BS842_Timing_GetSecondsElapsed(lastCounter, endCounter);
                    
#if AST_INTERNAL
                    DEBUG_FRAME_END(BS842_Timing_GetSecondsElapsed(frameStartCounter, endCounter));
#endif
                    lastCounter = endCounter;
                    
                    
                    if (programCode.DebugFrameEnd)
                    {
                        programCode.DebugFrameEnd(&gameMem);
                    }
                }
            }
        }
    }
    
    return 0;
}