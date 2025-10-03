#include "platform/win32_window.hpp"
#include "blackberry/log.hpp"

// #include "gl/gl.h"

namespace Blackberry {

    LRESULT CALLBACK WinProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp) {
        return DefWindowProc(hwnd, wm, wp, lp);
    }

    Window_Win32::Window_Win32(const WindowData& data)
        : Window(data) {
        m_WindowClass.cbSize = sizeof(m_WindowClass);
        m_WindowClass.style = 0;
        m_WindowClass.lpfnWndProc = WinProc;
        m_WindowClass.cbClsExtra = 0;
        m_WindowClass.cbWndExtra = 0;
        m_WindowClass.hInstance = GetModuleHandleW(0);
        m_WindowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        m_WindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        m_WindowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
        m_WindowClass.lpszMenuName = nullptr;
        m_WindowClass.lpszClassName = L"funny name";
        m_WindowClass.hIconSm = LoadIcon (NULL, IDI_APPLICATION);

        if (!RegisterClassExW(&m_WindowClass)) {
            BL_CRITICAL("Could NOT register window class for win32!");
            exit(1);
        }

        m_Handle = CreateWindowExW(WS_EX_LEFT,
                                  L"funny name",
                                  nullptr,
                                  WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  nullptr,
                                  nullptr,
                                  GetModuleHandleW(0),
                                  nullptr);

        if (!m_Handle) {
            BL_CRITICAL("Failed to create win32 window!");
            exit(1);
        }

        HDC hdc = GetDC(m_Handle);
        HGLRC hglrc;

        // hglrc = wglCreateContext(hdc);
        // wglMakeCurrent(hdc, hglrc);

        ShowWindow(m_Handle, false);
    }

    Window_Win32::~Window_Win32() {
        DestroyWindow(m_Handle);
    }

    bool Window_Win32::ShouldClose() const {
        return IsWindow(m_Handle);
    }

    void Window_Win32::OnUpdate() {
        UpdateWindow(m_Handle);

        while (GetMessage(&m_MSG, nullptr, 0, 0)) {
            TranslateMessage(&m_MSG);
            DispatchMessage(&m_MSG);
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

    void* Window_Win32::GetHandle() const {
        return m_Handle;
    }

} // namespace Blackberry
