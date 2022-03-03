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

#include "ast_opengl.cpp"

global b32 globalRunning;

#if AST_INTERNAL
DEBUG_PLATFORM_FREE_FILE(Debug_W32_FreeFile)
{
    if (memory)
    {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

DEBUG_PLATFORM_READ_FILE(Debug_W32_ReadFile)
{
    Debug_ReadFileResult result = {};
    
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            u32 fileSize32 = SafeTruncateU64(fileSize.QuadPart);
            result.contents = VirtualAlloc(0, fileSize32, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (result.contents)
            {
                DWORD bytesRead;
                if (ReadFile(fileHandle, result.contents, fileSize32, &bytesRead, 0) && (fileSize32 == bytesRead))
                {
                    result.contentsSize = fileSize32;
                }
                else
                {
                    Debug_W32_FreeFile(result.contents);
                    result.contents = 0;
                }
            }
        }
        
        CloseHandle(fileHandle);
    }
    else
    {
        printf("FAILED TO OPEN FILE %s FOR READING: %d\n", filename, GetLastError());
        ASSERT(false);
    }
    
    return result;
}

function void HandleCycleCounters(Game_Memory *memory)
{
    for (s32 counterIndex = 0; counterIndex < ARRAY_COUNT(memory->counters); ++counterIndex)
    {
        DebugCycleCounter *counter = &memory->counters[counterIndex];
        
        if (counter->hitCount)
        {
            counter->hitCount = 0;
            counter->cycleCount = 0;
        }
    }
}
#endif // AST_INTERNAL

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

//~ INPUT
function void W32_ProcessKeyboardEvent(Game_ButtonState *state, b32 keyIsDown)
{
    if (state->endedFrameDown != keyIsDown)
    {
        state->endedFrameDown = keyIsDown;
        ++state->halfTransitionCount;
    }
}

//~ RENDERING / OPENGL
function void W32_InitOpenGL(HDC deviceContext)
{
    PIXELFORMATDESCRIPTOR pfdReq = {};
    pfdReq.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfdReq.nVersion = 1;
    pfdReq.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfdReq.iPixelType = PFD_TYPE_RGBA;
    pfdReq.cColorBits = 32;
    pfdReq.cAlphaBits = 8;
    pfdReq.iLayerType = PFD_MAIN_PLANE;
    
    s32 pfdIndex = ChoosePixelFormat(deviceContext, &pfdReq);
    
    PIXELFORMATDESCRIPTOR pfd = {};
    DescribePixelFormat(deviceContext, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    SetPixelFormat(deviceContext, pfdIndex, &pfd);
    
    HGLRC glRC = wglCreateContext(deviceContext);
    if (wglMakeCurrent(deviceContext, glRC))
    {
        //glGenTextures(1, &globalTextureHandle);
    }
    else
    {
        ASSERT(false);
    }
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
function void W32_ProcessPendingMessages(HWND window, Game_Keyboard *keyboard, Game_RenderCommands *renderCommands, PlatformAPI platform)
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
                
                W32_PresentBuffer(renderCommands, platform, deviceContext);
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
                    if (vkCode == VK_F1)
                    {
                        W32_ProcessKeyboardEvent(&keyboard->keyF1, keyIsDown);
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
    W32_ResizeDIBSection(&platformBackBuffer, 900, 900);
    
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
                                      CW_USEDEFAULT, CW_USEDEFAULT, 900, 900,
                                      0, 0, instance, 0);
        
        if (window)
        {
            HDC deviceContext = GetDC(window);
            W32_InitOpenGL(deviceContext);
            
#define TARGET_HZ 60
            BS842_Timing_ChangeRefreshRate(deviceContext, false, 0);
            
            s32 audioLatencyFrames = 1;
            W32_AudioOutput audioOutput = {};
            audioOutput.samplesPerSecond = 48000;
            audioOutput.bytesPerSample = sizeof(s16) * 2;
            audioOutput.secondaryBufferSize = audioOutput.samplesPerSecond;
            audioOutput.latencySampleCount = audioLatencyFrames * (audioOutput.samplesPerSecond / BS842_Timing_GetRefreshRate());
            
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
            platform.Debug_FreeFile = Debug_W32_FreeFile;
            platform.Debug_ReadFile = Debug_W32_ReadFile;
            platform.MemAlloc = W32_MemAlloc;
            platform.MemFree = W32_MemFree;
            platform.MicrosecondsSinceEpoch = W32_MicrosecondsSinceEpoch;
            platform.SecondsSinceEpoch = W32_SecondsSinceEpoch;
            platform.GetAllFilesOfExtBegin = W32_GetAllFilesOfExtBegin;
            platform.GetAllFilesOfExtEnd = W32_GetAllFilesOfExtEnd;
            platform.OpenNextFile = W32_OpenNextFile;
            platform.MarkFileError = W32_MarkFileError;
            platform.ReadDataFromFile = W32_ReadDataFromFile;
            gameMem.platform = platform;
            
            s32 cpuInfo[4];
            __cpuid(cpuInfo, 1);
            gameMem.availableInstructionSets.sse3 = cpuInfo[2] & (1 << 0) || false;
            gameMem.availableInstructionSets.sse4_2 = cpuInfo[2] & (1 << 20) || false;
            gameMem.availableInstructionSets.avx = cpuInfo[2] & (1 << 28) || false;
            
            gameMem.permaStorageSize = MEGABYTE(16);
            gameMem.transStorageSize = MEGABYTE(512);
            
            u64 totalMemSize = gameMem.permaStorageSize + gameMem.transStorageSize;
            gameMem.permaStorage = VirtualAlloc(baseAddress, totalMemSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            gameMem.transStorage = (u8 *)gameMem.permaStorage + gameMem.permaStorageSize;
            
            if (samples && gameMem.permaStorage && gameMem.transStorage)
            {
                s32 debugTimeMarkerIndex = 0;
                W32_DebugTimeMarker debugTimeMarkers[TARGET_HZ] = {};
                
                u32 lastPlayCursor = 0;
                b32 soundValid = false;
                
                usize maxPushBufferSize = MEGABYTE(4);
                void *pushBufferBase = VirtualAlloc(0, maxPushBufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                Game_RenderCommands gameRenderCommands = InitialiseRenderCommands(maxPushBufferSize, pushBufferBase,
                                                                                  platformBackBuffer.width, platformBackBuffer.height);
                
                while (globalRunning)
                {
#if AST_INTERNAL
                    FILETIME newDLLWriteTime = W32_GetFileLastWriteTime(programCodeDLLPath);
                    if (CompareFileTime(&newDLLWriteTime, &programCode.dllLastWriteTime) != 0)
                    {
                        Sleep(33);
                        
                        W32_UnloadProgramCode(&programCode);
                        programCode = W32_LoadProgramCode(programCodeDLLPath, tempDLLPath, lockPath);
                    }
#endif
                    
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
                    
                    W32_ProcessPendingMessages(window, newKeyboard, &gameRenderCommands, platform);
                    
                    POINT mouseLoc;
                    GetCursorPos(&mouseLoc);
                    ScreenToClient(window, &mouseLoc);
                    b32 mouseLeftDown = (GetKeyState(VK_LBUTTON) & (1 << 15));
                    b32 mouseRightDown = (GetKeyState(VK_RBUTTON) & (1 << 15));
                    
#if AST_INTERNAL
                    debugGlobalMem = &gameMem;
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
                    
                    ASSERT(programCode.UpdateRender);
                    programCode.UpdateRender(&gameRenderCommands, &gameMem, newInput, &gameAudioBuffer);
                    
                    BS842_Timing_FrameEnd();
                    
                    //W32_RenderAudioSyncDisplay(&platformBackBuffer, ARRAY_COUNT(debugTimeMarkers), debugTimeMarkers, &audioOutput, BS842_Timing_GetTargetSecondsPerFrame());
                    
                    W32_PresentBuffer(&gameRenderCommands, platform, deviceContext);
                    
#if AST_INTERNAL
                    {
                        W32_DebugTimeMarker *marker = &debugTimeMarkers[debugTimeMarkerIndex++];
                        if (debugTimeMarkerIndex >= ARRAY_COUNT(debugTimeMarkers))
                        {
                            debugTimeMarkerIndex = 0;
                        }
                        
                        u64 positionFreq, positionUnits;
                        
                        IAudioClock *audioClock;
                        audioOutput.audioClient->GetService(IID_PPV_ARGS(&audioClock));
                        audioClock->GetFrequency(&positionFreq);
                        audioClock->GetPosition(&positionUnits, 0);
                        audioClock->Release();
                        
                        marker->playCursor = (s32)(audioOutput.samplesPerSecond * (positionUnits / positionFreq)) % audioOutput.samplesPerSecond;
                    }
#endif
                    SWAP(newInput, oldInput);
                    
                    HandleCycleCounters(&gameMem);
                    printf("%.02f / %.03f\n", BS842_Timing_GetFramesPerSecond(), BS842_Timing_GetMSPerFrame());
                }
            }
        }
    }
    
    return 0;
}