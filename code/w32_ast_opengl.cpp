/*
Project: Asteroids
File: w32_ast_opengl.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

// NOTE(bSalmon): WGL defines
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_FULL_ACCELERATION_ARB               0x2027
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB        0x20A9
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

global s32 globalWGLContextAttribs[] = 
{
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 0,
    WGL_CONTEXT_FLAGS_ARB, 0
#if AST_INTERNAL
    | WGL_CONTEXT_DEBUG_BIT_ARB
#endif
    ,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
    0,
};

typedef BOOL WINAPI wgl_swapIntervalEXT(s32 interval);
typedef int WINAPI wgl_getSwapIntervalEXT();
typedef HGLRC WINAPI wgl_createContextAttribsARB(HDC deviceContext, HGLRC shareContext, const s32 *attribs);
typedef BOOL WINAPI wgl_choosePixelFormatARB(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats,
                                             int *piFormats, UINT *nNumFormats);
typedef char *WINAPI wgl_getExtensionsStringEXT();

global wgl_swapIntervalEXT *wglSwapInterval;
global wgl_getSwapIntervalEXT *wglGetSwapInterval;
global wgl_createContextAttribsARB *wglCreateContextAttribsARB;
global wgl_choosePixelFormatARB *wglChoosePixelFormatARB;
global wgl_getExtensionsStringEXT *wglGetExtensionsStringEXT;

typedef GLubyte *WINAPI type_glGetStringi(GLenum name, GLuint index);

#include "ast_opengl.cpp"

#define WGL_GetGLFunction(name) glInfo->##name = (type_##name *)wglGetProcAddress(#name)
function void WGL_LoadGLFunctions(OpenGL_Info *glInfo)
{
    WGL_GetGLFunction(glGetStringi);
}

function void WGL_SetPixelFormat(HDC deviceContext, b32 support_SRGB)
{
    s32 pfdIndex = 0;
    GLuint extPick = 0;
    if (wglChoosePixelFormatARB)
    {
        s32 attribList[] = 
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
            0,
        };
        
        if (!support_SRGB)
        {
            attribList[10] = 0;
        }
        
        wglChoosePixelFormatARB(deviceContext, attribList, 0, 1, &pfdIndex, &extPick);
    }
    
    if (!extPick)
    {
        PIXELFORMATDESCRIPTOR pfdReq = {};
        pfdReq.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfdReq.nVersion = 1;
        pfdReq.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfdReq.iPixelType = PFD_TYPE_RGBA;
        pfdReq.cColorBits = 32;
        pfdReq.cAlphaBits = 8;
        pfdReq.cDepthBits = 24;
        pfdReq.iLayerType = PFD_MAIN_PLANE;
        
        pfdIndex = ChoosePixelFormat(deviceContext, &pfdReq);
    }
    
    PIXELFORMATDESCRIPTOR pfd = {};
    DescribePixelFormat(deviceContext, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    SetPixelFormat(deviceContext, pfdIndex, &pfd);
}

function void WGL_LoadWGLExtensions(b32 *support_SRGB)
{
    WNDCLASSA windowClass = {};
    windowClass.lpfnWndProc = DefWindowProcA;
    windowClass.hInstance = GetModuleHandle(0);
    windowClass.lpszClassName = "GLSetupWindowClass";
    if (RegisterClassA(&windowClass))
    {
        HWND window = CreateWindowExA(0,
                                      windowClass.lpszClassName, "GL Setup Window",
                                      0,
                                      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                      0, 0, windowClass.hInstance, 0);
        ASSERT(window);
        HDC setupDC = GetDC(window);
        WGL_SetPixelFormat(setupDC, *support_SRGB);
        HGLRC glRC = wglCreateContext(setupDC);
        if (wglMakeCurrent(setupDC, glRC))
        {
            wglCreateContextAttribsARB = (wgl_createContextAttribsARB *)wglGetProcAddress("wglCreateContextAttribsARB");
            wglChoosePixelFormatARB = (wgl_choosePixelFormatARB *)wglGetProcAddress("wglChoosePixelFormatARB");
            wglGetExtensionsStringEXT = (wgl_getExtensionsStringEXT *)wglGetProcAddress("wglGetExtensionsStringEXT");
            
            if (wglGetExtensionsStringEXT)
            {
                char *token = wglGetExtensionsStringEXT();
                
                while (*token)
                {
                    while (IsCharWhitespace(*token)) { token++; }
                    char *end = token;
                    while (*end && !IsCharWhitespace(*end)) { end++; }
                    
                    s32 charCount = (s32)(end - token);
                    
                    if (StringsAreSame(token, "GL_EXT_framebuffer_sRGB", charCount)) { *support_SRGB = true; }
                    else if (StringsAreSame(token, "GL_ARB_framebuffer_sRGB", charCount)) { *support_SRGB = true; }
                    
                    token = end;
                }
            }
        }
        else
        {
            ASSERT(false);
        }
        
        wglDeleteContext(glRC);
        ReleaseDC(window, setupDC);
        DestroyWindow(window);
    }
}
