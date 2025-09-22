#define BL_ENTRYPOINT
#include "blackberry.hpp"

class ExampleLayer : public Blackberry::Layer {
public:
    virtual void OnEvent(const Blackberry::Event& event) override {
        Log(Log_Info, "Recieved event: %s", event.ToString().c_str());
    }
};

Blackberry::Application* Blackberry::Application::CreateApplication(const CommandLineArgs& args) {
    ApplicationSpecification spec;
    spec.width = 1280;
    spec.height = 720;
    spec.name = "Blackberry events!";

    Application* app = new Blackberry::Application(spec);
    app->PushLayer(new ExampleLayer);

    return app;
}