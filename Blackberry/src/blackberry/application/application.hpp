#pragma once

#include "blackberry/event/event.hpp"
#include "blackberry/application/layerstack.hpp"
#include "blackberry/log.hpp"
#include "blackberry/types.hpp"
#include "blackberry/application/window.hpp"
#include "blackberry/application/renderer.hpp"

#define BL_APP Blackberry::Application::Get()

namespace Blackberry {

    struct ApplicationSpecification {
        const char* Name;
        u32 Width, Height;
        u32 FPS;
    };

    class CommandLineArgs {
    public:
        inline CommandLineArgs(u32 argc, char** argv) {
            this->argc = argc;
            this->argv = argv;
        }

        inline const char* at(const u32 i) {
            if (i >= argc) {
                BL_CRITICAL("Trying to access position in command line arguments that is out of bounds! {Position {}, Size {}}", i, argc);
                exit(1);
            }

            return argv[i];
        }

        inline u32 size() const {
            return argc;
        }

    private:
        u32 argc;
        char** argv;
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

        f32 GetDeltaTime() const { return m_dt; }
        u32 GetFPS() const { return static_cast<u32>(1.0f / m_dt); }

        // To be implemented by client!
        static Application* CreateApplication(const CommandLineArgs& args);

    private:
        void OnUpdate();
        void OnRender();
        void OnUIRender();
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