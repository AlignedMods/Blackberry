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
            m_CurrentScene = Project::GetStartScene();

            m_CurrentScene->OnRuntimeStart();
            m_CurrentScene->SetPaused(false);

            FramebufferSpecification spec;
            spec.Width = 1920;
            spec.Height = 1080;
            spec.Attachments = {
                {0, FramebufferAttachmentType::ColorRGBA8},
                {1, FramebufferAttachmentType::Depth24}
            };
            spec.ActiveAttachments = { 0 };
            m_RenderTarget = Framebuffer::Create(spec);
        }

        virtual void OnDetach() override {
            m_CurrentScene->OnRuntimeStop();
        }

        virtual void OnUpdate() override {
            auto& api = BL_APP.GetRendererAPI();

            m_CurrentScene->OnUpdateRuntime();
            m_CurrentScene->OnRenderRuntime(m_RenderTarget);

            api.ClearFramebuffer();
            m_RenderTarget->BlitToSwapchain();
        }

        virtual void OnEvent(const Event& e) {
            if (e.GetEventType() == EventType::WindowClose) {
                BL_APP.Close();
            }
        }
    
    private:
        Ref<Scene> m_CurrentScene;
        Ref<Framebuffer> m_RenderTarget;
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