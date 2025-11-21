#define BL_ENTRYPOINT
#include "blackberry.hpp"

using namespace Blackberry;

namespace BlackberryRuntime {

    class RuntimeLayer : public Layer {
    public:
        virtual void OnAttach() override {
            auto commandLineArgs = BL_APP.GetSpecification().CommandLineArgs;
            const char* path = commandLineArgs.Args[1];
            Project::Load(path);
            m_CurrentScene = &Project::GetStartScene().Scene;

            m_CurrentScene->OnPlay();
        }

        virtual void OnDetach() override {
            m_CurrentScene->OnStop();
        }

        virtual void OnUpdate(f32 ts) override {
            m_CurrentScene->OnUpdate();
            m_CurrentScene->OnRuntimeUpdate();
        }

        virtual void OnRender() override {
            Renderer2D::Clear(BlColor(0x69, 0x69, 0x69, 0xff));

            m_CurrentScene->SetCamera(&m_Camera);
            m_CurrentScene->OnRender();
        }
    
    private:
        Scene* m_CurrentScene = nullptr;
        SceneCamera m_Camera;
    };
    
} // namespace BlackberryRuntime

namespace Blackberry {

    Application* CreateApplication(u32 argc, char** argv) {
        BL_ASSERT(argc >= 2, "Must provide at least one command line argument to runtime!");

        ApplicationSpecification spec;
        spec.EnableImGui = false;
        spec.Width = 1280;
        spec.Height = 720;
        spec.Title = "Blackberry Runtime";
        spec.CommandLineArgs = {argc, argv};

        Application* app = new Application(spec);
        app->PushLayer(new BlackberryRuntime::RuntimeLayer);
    
        return app;
    }

} // namespace Blackberry