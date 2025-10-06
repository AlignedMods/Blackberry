#include "platform/win32/win32_window.hpp"
#include "blackberry/log.hpp"
#include "blackberry/application/application.hpp"
#include "blackberry/event/window_events.hpp"

#include "glad/glad.h"

#define WGL_DRAW_TO_WINDOW_ARB              0x2001
#define WGL_ACCELERATION_ARB                0x2003
#define WGL_SUPPORT_OPENGL_ARB              0x2010
#define WGL_DOUBLE_BUFFER_ARB               0x2011
#define WGL_PIXEL_TYPE_ARB                  0x2013
#define WGL_COLOR_BITS_ARB                  0x2014
#define WGL_RED_BITS_ARB                    0x2015
#define WGL_RED_SHIFT_ARB                   0x2016
#define WGL_GREEN_BITS_ARB                  0x2017
#define WGL_GREEN_SHIFT_ARB                 0x2018
#define WGL_BLUE_BITS_ARB                   0x2019
#define WGL_BLUE_SHIFT_ARB                  0x201a
#define WGL_ALPHA_BITS_ARB                  0x201b
#define WGL_ALPHA_SHIFT_ARB                 0x201c
#define WGL_DEPTH_BITS_ARB                  0x2022
#define WGL_STENCIL_BITS_ARB                0x2023
#define WGL_TYPE_RGBA_ARB                   0x202b

#define WGL_NO_ACCELERATION_ARB             0x2025      // OpenGL 1.1 GDI software rasterizer
#define WGL_GENERIC_ACCELERATION_ARB        0x2026
#define WGL_FULL_ACCELERATION_ARB           0x2027      // OpenGL hardware-accelerated, using GPU-drivers provided by vendor

#define WGL_NUMBER_PIXEL_FORMATS_ARB        0x2000
#define WGL_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB       0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB        0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB    0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

// Define WGL function pointer types (no wglext.h needed)
typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int *);
typedef BOOL (WINAPI *PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC, const int *, const FLOAT *, UINT, int *, UINT *);
typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int);

namespace Blackberry {

    static bool s_ShouldClose = false;

    static void* GLGetProcAddr(const char* name) {
        PROC proc = wglGetProcAddress(name);

        if (!proc) {
            HMODULE module = LoadLibraryA("opengl32.dll");
            proc = GetProcAddress(module, name);
        }

        return proc;
    }

    static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
    static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        switch (msg) {
            case WM_CLOSE: s_ShouldClose = true; break;
            case WM_PAINT
        }

        return DefWindowProcW(hwnd, msg, wp, lp);
    }

    Window_Win32::Window_Win32(const WindowData& data)
        : Window(data) {
        WNDCLASSEXW windowClass{};

        windowClass.cbSize = sizeof(WNDCLASSEXW),
        windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        windowClass.lpfnWndProc = WndProc,
        windowClass.cbWndExtra = sizeof(LONG_PTR),
        windowClass.hInstance = GetModuleHandleW(0),
        windowClass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW),
        windowClass.lpszClassName = L"Window Class";

        if (!RegisterClassExW(&windowClass)) { BL_CRITICAL("Could NOT register window class for win32, error: {}!", GetLastError()); }

        m_Handle = CreateWindowExW(0,
                                  L"Window Class",
                                  L"gurting",
                                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  data.Width,
                                  data.Height,
                                  nullptr,
                                  nullptr,
                                  GetModuleHandleW(0),
                                  nullptr);

        if (!m_Handle) {
            BL_CRITICAL("Failed to create win32 window!");
            exit(1);
        }

        // dummy context
        PIXELFORMATDESCRIPTOR pixelFormatDesc = {
            .nSize = sizeof(PIXELFORMATDESCRIPTOR),
            .nVersion = 1,
            .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
            .iPixelType = PFD_TYPE_RGBA,
            .cColorBits = 32,
            .cAlphaBits = 8,
            .cDepthBits = 24,
            .iLayerType = PFD_MAIN_PLANE
        };

        HDC hdc = GetDC(m_Handle);

        i32 pixelFormat = ChoosePixelFormat(hdc, &pixelFormatDesc);
        SetPixelFormat(hdc, pixelFormat, &pixelFormatDesc);

        HGLRC dummyCtx = wglCreateContext(hdc);
        wglMakeCurrent(hdc, dummyCtx);

        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

        if (wglChoosePixelFormatARB) {
                int pixelFormatAttribs[] = {
                    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                    WGL_COLOR_BITS_ARB, 32,
                    //WGL_RED_BITS_ARB, 8,
                    //WGL_GREEN_BITS_ARB, 8,
                    //WGL_BLUE_BITS_ARB, 8,
                    //WGL_ALPHA_BITS_ARB, 8,
                    WGL_DEPTH_BITS_ARB, 24,
                    WGL_STENCIL_BITS_ARB, 8,
                    0 // Terminator
                };

            int format = 0;
            UINT numFormats = 0;
            if (wglChoosePixelFormatARB(hdc, pixelFormatAttribs, NULL, 1, &format, &numFormats) && (numFormats > 0))
            {
                PIXELFORMATDESCRIPTOR newPixelFormatDescriptor = { 0 };
                DescribePixelFormat(hdc, format, sizeof(newPixelFormatDescriptor), &newPixelFormatDescriptor);
                SetPixelFormat(hdc, format, &newPixelFormatDescriptor);
            }
        }

        if (wglCreateContextAttribsARB)
        {
            int contextAttribs[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, // WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, WGL_CONTEXT_ES_PROFILE_BIT_EXT (if supported)
                //WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB [glDebugMessageCallback()]
                0 // Terminator
            };

            // NOTE: We are not sharing context resources so, second parameters is NULL
            m_OpenGLHandle = wglCreateContextAttribsARB(hdc, NULL, contextAttribs);
        }

        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(dummyCtx);

        if (m_OpenGLHandle) { wglMakeCurrent(hdc, m_OpenGLHandle); }

        if (!gladLoadGLLoader(GLGetProcAddr)) {
            BL_CRITICAL("Failed to load GLAD!");
        }

        Dispatcher& dispatcher = BL_APP.GetDispatcher();

        ShowWindow(m_Handle, SW_RESTORE);
    }

    Window_Win32::~Window_Win32() {
        DestroyWindow(m_Handle);
    }

    bool Window_Win32::ShouldClose() const {
        return s_ShouldClose;
    }

    void Window_Win32::OnUpdate() {
        UpdateWindow(m_Handle);

        MSG msg;

        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void Window_Win32::OnRenderStart() {
    }

    void Window_Win32::OnRenderFinish() {
    }

    f64 Window_Win32::GetTime() const {
        return 1.0;
    }

    void Window_Win32::SleepSeconds(f64 seconds) const {
        Sleep(static_cast<DWORD>(seconds * 1000));
    }

    void Window_Win32::SetWindowIcon(const Image& image) {
    }

    void* Window_Win32::GetHandle() const {
        return m_Handle;
    }

} // namespace Blackberry
