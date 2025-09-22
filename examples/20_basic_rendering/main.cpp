#define BL_ENTRYPOINT
#include "blackberry.hpp"

class ExampleLayer : public Blackberry::Layer {
public:
    virtual void OnRender() override {
        Blackberry::DrawRectangle(BlVec2(100, 100), BlVec2(300, 100), BlColor(0xff, 0, 0, 0xff));

        Blackberry::DrawTriangle(BlVec2(130, 400), BlVec2(260, 250), BlVec2(390, 400), color);
    }

    // runs 60 times per second
    virtual void OnFixedUpdate() override {
        color.r++;
        color.g++;
        color.b++;
    }

private:
    BlColor color;
};

Blackberry::Application* Blackberry::Application::CreateApplication(const CommandLineArgs& args) {
    ApplicationSpecification spec;
    spec.width = 1280;
    spec.height = 720;
    spec.name = "Blackberry basic rendering!";

    Application* app = new Blackberry::Application(spec);
    app->PushLayer(new ExampleLayer);

    return app;
}