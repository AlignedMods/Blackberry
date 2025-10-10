#pragma once

#include "blackberry.hpp"

class EditorLayer : public Blackberry::Layer {
public:
    ~EditorLayer();

    virtual void OnInit() override;

    virtual void OnUpdate(f32 ts) override;
    virtual void OnRender() override;
    virtual void OnUIRender() override;
    virtual void OnEvent(const Blackberry::Event& event) override;

private:
    Blackberry::Scene m_EditorScene;
    Blackberry::Font m_EditorFont;

    Blackberry::EntityID m_SelectedEntity = 0;
    bool m_IsEntitySelected = false;

    BlRenderTexture m_RenderTexture;

    BlTexture m_BlankTexture;
};
