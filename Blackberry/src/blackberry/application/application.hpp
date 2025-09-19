#pragma once

#include "blackberry/event/event.hpp"
#include "blackberry/application/layerstack.hpp"
#include "blackberry/log.hpp"
#include "blackberry/types.hpp"
#include "blackberry/application/window.hpp"
#include "blackberry/application/renderer.hpp"

namespace Blackberry {

    struct ApplicationSpecification {
        const char* name;
        u32 width, height;
        u32 min_width, min_height;
        bool enable_audio;
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
                Log(Log_Critical, "Trying to access position in command line arguments that is out of bounds! {Position %lu, Size %lu}", i, argc);
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

        template<typename T>
        inline void PushLayer() {
            m_Stack.PushLayer<T>();
        }

        static Application& Get();
        LayerStack& GetLayerStack() { return m_Stack; }
        Dispatcher& GetDispatcher() { return m_Dispatcher; }
        Renderer& GetRenderer() { return *m_Renderer; }

        f32 GetDeltaTime() const { return m_dt; }

        // To be implemented by client!
        static Application* CreateApplication(const CommandLineArgs& args);

    private:
        void OnUpdate();
        void OnRender();
        void OnUIRender();
        void OnEvent(const Event& event);

    private:
        ApplicationSpecification m_Specification;

        u32 m_TargetFPS = 0;

        bool m_Running = false;
        bool m_Initalized = false;

        f64 m_CurrentTime = 0.0;
        f64 m_LastTime = 0.0;
        f32 m_dt = 0.0f;

        f64 m_FixedUpdateTime = 0.0;

        LayerStack m_Stack;
        Dispatcher m_Dispatcher;

        Window* m_Window = nullptr;
        Renderer* m_Renderer = nullptr;

        friend class Dispatcher;
    };

} // namespace Blackberry