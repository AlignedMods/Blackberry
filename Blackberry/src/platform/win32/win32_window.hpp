#pragma once

#include "blackberry/application/window.hpp"

#include "Windows.h"
#include "gl/GL.h"

namespace Blackberry {

    class Window_Win32 : public Window {
    public:
        Window_Win32(const WindowData& data);
        ~Window_Win32();

        virtual bool ShouldClose() const override;
        virtual void OnUpdate() override;
        virtual void OnRenderStart() override;
        virtual void OnRenderFinish() override;

        virtual f64 GetTime() const override;
        virtual void SleepSeconds(f64 seconds) const override;

        virtual void SetWindowIcon(const Image& image) override;

        virtual void* GetHandle() const override;

    private:
        HWND m_Handle;
        HGLRC m_OpenGLHandle;
    };

} // namespace Blackberry
