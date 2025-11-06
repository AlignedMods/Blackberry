#pragma once

#include "blackberry/event/event.hpp"
#include "blackberry/application/layerstack.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/core/types.hpp"
#include "blackberry/application/window.hpp"
#include "blackberry/application/renderer.hpp"

#define BL_APP Blackberry::Application::Get()

namespace Blackberry {

    struct ApplicationSpecification {
        const char* Name;
        u32 Width = 0, Height = 0;
        u32 FPS = 0;

        bool EnableImGui = true;
    };

    class Application {
    public:
        Application(const ApplicationSpecification& spec);
        ~Application();

        void Run();

        bool IsInitialized();

        void SetTargetFPS(u32 fps);
        void SetWindowIcon(const Image& image);

        void Close();

        void PushLayer(Layer* layer);
        void PopLayer();
        void PopLayer(const std::string& name);

        static Application& Get();
        LayerStack& GetLayerStack() { return m_LayerStack; }
        Dispatcher& GetDispatcher() { return m_Dispatcher; }
        Renderer& GetRenderer() { return *m_Renderer; }
        Window& GetWindow() { return *m_Window; }

        f32 GetDeltaTime() const { return m_dt; }
        u32 GetFPS() const { return static_cast<u32>(1.0f / m_dt); }

    private:
        void OnUpdate();
        void OnRender();
        void OnUIRender();
        void OnOverlayRender();
        void OnEvent(const Event& event);

        void InitImGui();

    private:
        ApplicationSpecification m_Specification;

        u32 m_TargetFPS = 0;

        bool m_Running = false;
        bool m_Initalized = false;

        f64 m_CurrentTime = 0.0;
        f64 m_LastTime = 0.0;
        f32 m_dt = 0.0f;

        f64 m_FixedUpdateTime = 0.0;

        LayerStack m_LayerStack;
        Dispatcher m_Dispatcher;

        Window* m_Window = nullptr;
        Renderer* m_Renderer = nullptr;

        friend class Dispatcher;
    };

} // namespace Blackberry