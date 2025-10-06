#pragma once

#include "blackberry.hpp"

class EditorLayer : public Blackberry::Layer {
public:
    virtual void OnInit() override;

    virtual void OnUpdate(f32 ts) override;
    virtual void OnRender() override;
    virtual void OnUIRender() override;

private:
    Blackberry::Scene m_EditorScene;
    Blackberry::Entity m_TestEntity;

    Blackberry::Font m_EditorFont;
};
