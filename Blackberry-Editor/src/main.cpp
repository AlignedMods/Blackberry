#define BL_ENTRYPOINT
#include "blackberry.hpp"

#include "editor_layer.hpp"

Blackberry::Application* Blackberry::Application::CreateApplication(const CommandLineArgs &args) {
    Blackberry::ApplicationSpecification spec;
    spec.Name = "Blackberry Editor";
    spec.Width = 1280;
    spec.Height = 720;

    Blackberry::Application* app = new Blackberry::Application(spec);
    app->PushLayer(new BlackberryEditor::EditorLayer);

    return app;
}
