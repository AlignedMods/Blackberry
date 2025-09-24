#define BL_ENTRYPOINT
#include "blackberry.hpp"

class ExampleLayer : public Blackberry::Layer {
public:
    virtual void OnEvent(const Blackberry::Event& event) override {
        Log(Log_Info, "Recieved event: %s", event.ToString().c_str());

        event.Handled = true; // setting this to true will make the event not be dispatched any further

        // Note about events: if event.Handled the event still WILL be dispatched further, you must check
        // whether the event is appropriate to be used (by checking event.Handled)
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