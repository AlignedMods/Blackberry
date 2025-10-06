#define BL_ENTRYPOINT
#include "blackberry.hpp"

class ExampleLayer : public Blackberry::Layer {
public:
    virtual void OnEvent(const Blackberry::Event& event) override {
        BL_INFO("Recieved event: %s", event.ToString());

        event.Handled = true; // setting this to true will make the event not be dispatched any further

        // Note about events: if event.Handled the event still WILL be dispatched further, you must check
        // whether the event is appropriate to be used (by checking event.Handled)
    }
};

Blackberry::Application* Blackberry::Application::CreateApplication(const CommandLineArgs& args) {
    ApplicationSpecification spec;
    spec.Name = "Blackberry events!";
    spec.Width = 1280;
    spec.Height = 720;

    Application* app = new Blackberry::Application(spec);
    app->PushLayer(new ExampleLayer);

    return app;
}
