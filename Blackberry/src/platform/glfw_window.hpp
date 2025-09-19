#pragma once

#include "blackberry/application/window.hpp"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

namespace Blackberry {

    class Window_GLFW : public Window {
    public:
        Window_GLFW(const WindowData& data);
        ~Window_GLFW();

        virtual bool ShouldClose() const override;
        virtual void OnUpdate() override;
        virtual void OnRenderStart() override;
        virtual void OnRenderFinish() override;

        virtual f64 GetTime() const override;
        virtual void SleepSeconds(f64 seconds) const override;

        virtual void* GetHandle() const override;

    private:
        GLFWwindow* m_Handle;
    };

} // namespace Blackberry