#define BL_ENTRYPOINT
#include "blackberry.hpp"

struct Rabbit {
    BlVec2 Pos;
    BlVec2 Speed;
    BlColor Color;
};

class StressTestLayer : public Blackberry::Layer {
public:
    virtual void OnAttach() override {
        m_RabbitTexture.Create("Assets/rabbit.png");

        srand(BL_APP.GetWindow().GetTime() * 1000);
    }

    virtual void OnUpdate(f32 ts) override {
        BlVec2 windowSize = BL_APP.GetWindow().GetWindowDims();

        for (auto& rabbit : m_Rabbits) {
            if (rabbit.Pos.x + m_RabbitTexture.Width > windowSize.x || rabbit.Pos.x < 0.0f) {
                rabbit.Speed.x *= -1;
            }

            if (rabbit.Pos.y + m_RabbitTexture.Height > windowSize.y || rabbit.Pos.y < 0.0f) {
                rabbit.Speed.y *= -1;
            }

            rabbit.Pos.x += rabbit.Speed.x * ts;
            rabbit.Pos.y += rabbit.Speed.y * ts;
        }

        if (Blackberry::Input::IsKeyDown(KeyCode::Space)) {
            // generate some bunnies

            for (u32 i = 0; i < 1000; i++) {
                Rabbit rabbit;
                rabbit.Pos.x = windowSize.x / 2.0f - m_RabbitTexture.Width / 2.0f;
                rabbit.Pos.y = windowSize.y / 2.0f - m_RabbitTexture.Height / 2.0f;

                rabbit.Speed.x = rand() % 1000 - 500;
                rabbit.Speed.y = rand() % 1000 - 500;

                rabbit.Color = BlColor( rand() % 170 + 80, rand() % 170 + 80, rand() % 170 + 80, 0xff);

                m_Rabbits.emplace_back(rabbit);
            }
        }
    }

    virtual void OnRender() override {
        Blackberry::Renderer2D::Clear();

        for (auto& rabbit : m_Rabbits) {
            Blackberry::Renderer2D::DrawTexture(BlVec3(rabbit.Pos.x, rabbit.Pos.y, 0.0f), m_RabbitTexture, 0.0f, rabbit.Color);
        }
    }

    virtual void OnUIRender() override {
        ImGui::Begin("Stats");

        ImGui::Text("Bunnies: %llu", m_Rabbits.size());
        ImGui::Text("FPS: %f", 1.0f / BL_APP.GetDeltaTime());

        ImGui::End();
    }

private:
    BlTexture m_RabbitTexture;

    std::vector<Rabbit> m_Rabbits;
};


Blackberry::Application* Blackberry::CreateApplication(u32 argc, char** argv) {
    ApplicationSpecification spec;
    spec.Name = "Blackberry stress test";
    spec.Width = 1280;
    spec.Height = 720;
    spec.EnableImGui = false;

    Application* app = new Application(spec);
    app->PushLayer(new StressTestLayer);

    return app;
}