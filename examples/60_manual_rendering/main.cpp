#define BL_ENTRYPOINT
#include "blackberry.hpp"

class ExampleLayer : public Blackberry::Layer {
public:
    virtual void OnRender() override {
        // get the renderer
        Blackberry::Renderer& renderer = BL_APP.GetRenderer();

        renderer.Begin(RenderingMode::Triangles);

        // draw the classic learn OpenGL triangle
        // renderer.VertexV({BlVec2(100.0f, 400.0f), BlColor(0xff, 0x00, 0x00, 0xff)}); // bottom left
        // renderer.VertexV({BlVec2(300.0f, 100.0f), BlColor(0x00, 0xff, 0x00, 0xff)}); // top
        // renderer.VertexV({BlVec2(500.0f, 400.0f), BlColor(0x00, 0x00, 0xff, 0xff)}); // bottom right

        renderer.End();
    }
};

Blackberry::Application* Blackberry::Application::CreateApplication(const CommandLineArgs& args) {
    ApplicationSpecification spec;
    spec.Name = "Blackberry manual rendering";
    spec.Width = 1280;
    spec.Height = 720;

    Blackberry::Application* app = new Blackberry::Application(spec);
    app->PushLayer(new ExampleLayer);

    return app;
}
