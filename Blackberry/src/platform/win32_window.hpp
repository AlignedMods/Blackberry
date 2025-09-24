#pragma once

#include "blackberry/application/window.hpp"

#include "Windows.h"

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

        virtual void* GetHandle() const override;

    private:
        WNDCLASS m_WindowClass;
    };

} // namespace Blackberry