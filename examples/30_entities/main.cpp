#define BL_ENTRYPOINT
#include "blackberry.hpp"

class ExampleLayer : public Blackberry::Layer {
public:
    virtual void OnInit() override {
        using namespace Blackberry::Components; // this is recommended to use in functions that do lots of work with components

        Blackberry::Image im("Assets/spoonggee.jpg");

        auto entity = m_Coordinator->CreateEntity();
        m_Coordinator->AddComponent<Drawable>(entity, { BlColor(0xff, 0, 0, 0xff) });
        m_Coordinator->AddComponent<Transform>(entity, { BlVec2(100.0f, 100.0f), BlVec2(300.0f, 75.0f) });

        // create an entity with a material (texture)
        auto other = m_Coordinator->CreateEntity();
        m_Coordinator->AddComponent<Drawable>(other);
        m_Coordinator->AddComponent<Transform>(other, { BlVec2(300.0f, 150.0f), BlVec2(700.0f, 400.0f) });
        m_Coordinator->AddComponent<Material>(other, { Blackberry::LoadTextureFromImage(im) });
    }
};

Blackberry::Application* Blackberry::Application::CreateApplication(const CommandLineArgs& args) {
    Blackberry::ApplicationSpecification spec;
    spec.Name = "Blackberry entities example";
    spec.Width = 1280;
    spec.Height = 720;

    Blackberry::Application* app = new Blackberry::Application(spec);
    app->PushLayer(new ExampleLayer);

    return app;
}