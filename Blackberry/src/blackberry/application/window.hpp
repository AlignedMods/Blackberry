#pragma once

#include "blackberry/event/event.hpp"
#include "blackberry/image/image.hpp"

#include <functional>

namespace Blackberry {

    enum class CursorMode {
        Nomral,
        Hidden,
        Disabled
    };

    using EventCallbackFn = std::function<void(const Event&)>;

    struct WindowData {
        std::string Name;
        u32 Width = 720, Height = 1280;
    };

    // an abstract window class so we can use multiple windowing and input backends (GLFW, win32...)
    // we also don't want to directly include any platform specific code in application.hpp
    class Window {
    public:
        Window(const WindowData& data, bool imguiEnabled)
            : m_WindowData(data), m_ImGuiEnabled(imguiEnabled) {}
        virtual ~Window() = default;

        virtual BlVec2 GetWindowDims() const = 0;

        virtual bool ShouldClose() const = 0;
        virtual void OnUpdate() = 0;
        virtual void OnRenderStart() = 0;
        virtual void OnRenderFinish() = 0;

        virtual f64 GetTime() const = 0;
        virtual void SleepSeconds(f64 seconds) const = 0;
        void SleepMilli(f64 milliseconds) const { SleepSeconds(milliseconds / 1000.0); }

        virtual void SetWindowIcon(const Image& image) = 0;
        virtual void SetCursorMode(CursorMode mode) = 0;

        virtual void* GetHandle() const = 0;
        virtual void* GetNativeHandle() const = 0;

    protected:
        EventCallbackFn m_EventCallback;
        WindowData m_WindowData;
        bool m_ImGuiEnabled = true;
    };

} // namespace Blackberry