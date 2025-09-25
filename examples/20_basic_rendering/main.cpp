#define BL_ENTRYPOINT
#include "blackberry.hpp"

class ExampleLayer : public Blackberry::Layer {
public:
    virtual void OnInit() override {
        m_Image.LoadFromPath("Assets/cat.jpg"); // NOTE: You should call LoadFromPath instead of doing Blackberry::Image if it's a 
                                                // stack allocated object since not doing so will free image data more that once!!

        m_Texture = Blackberry::LoadTextureFromImage(m_Image);

        
        m_FontAtlas = m_Font.LoadFontFromFile("Assets/komika_axis/axis.ttf");
        // m_Texture = Blackberry::LoadTextureFromImage(m_FontAtlas);

        // Blackberry::Application::Get().SetWindowIcon(m_Image);
    }

    virtual void OnRender() override {
        Blackberry::DrawRectangle(BlVec2(100, 100), BlVec2(300, 100), BlColor(0xff, 0, 0, 0xff));

        Blackberry::DrawTriangle(BlVec2(130, 400), BlVec2(260, 250), BlVec2(390, 400), m_Color);

        Blackberry::DrawTextureEx(m_CatPosition, BlVec2(400, 300), m_Texture);
        // Blackberry::DrawTexture(BlVec2(300.0f, 400.0f), m_FontTexture);
    }

    // runs 60 times per second
    virtual void OnFixedUpdate() override {
        m_Color.r++;
        m_Color.g++;
        m_Color.b++;

        if (m_CatPosition.x > 700.0f) {
            m_CatGoingBackwards = true;
        } else if (m_CatPosition.x < 400.0f) {
            m_CatGoingBackwards = false;
        }

        if (m_CatGoingBackwards) {
            m_CatPosition.x -= 2.0f;
        } else {
            m_CatPosition.x += 2.0f;
        }
    }

private:
    BlColor m_Color;

    Blackberry::Image m_Image;
    BlTexture m_Texture;

    BlVec2 m_CatPosition = BlVec2(500, 140);
    bool m_CatGoingBackwards = false;

    Blackberry::Font m_Font;
    Blackberry::Image m_FontAtlas;
    BlTexture m_FontTexture;
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