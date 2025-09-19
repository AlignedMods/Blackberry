#include "blackberry/application/entrypoint.hpp"

class ExampleLayer : public Blackberry::Layer {
public:
    virtual void OnRender() override {

    }
};

Blackberry::Application* Blackberry::Application::CreateApplication(const CommandLineArgs& args) {
    ApplicationSpecification spec;
    spec.width = 1280;
    spec.height = 720;
    spec.name = "Blackberry basic window!";

    Application* app = new Blackberry::Application(spec);
    app->PushLayer<ExampleLayer>();

    return app;
}