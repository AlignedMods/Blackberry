#include "editor_layer.hpp"

void EditorLayer::OnInit() {
    using namespace Blackberry::Components;

    m_EditorFont.LoadFontFromFile("Assets/arial/arial.ttf", 36);

    m_TestEntity = Blackberry::Entity(m_EditorScene.CreateEntity("funny"), &m_EditorScene);

    m_TestEntity.AddComponent<Drawable>({ BlColor(0xff, 0x00, 0x00, 0xff) });
    m_TestEntity.AddComponent<Transform>({ BlVec2(100.0f, 50.0f), BlVec2(400.0f, 100.0f) });
    m_TestEntity.AddComponent<Text>({ &m_EditorFont, 36, "gurt__gooning" });
}

void EditorLayer::OnUpdate(f32 ts) {
    m_EditorScene.OnUpdate();
}

void EditorLayer::OnRender() {
    m_EditorScene.OnRender();
}

void EditorLayer::OnUIRender() {
    using namespace Blackberry::Components;

    // set up dockspace
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("dockspace window", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                                              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
    ImGui::PopStyleVar();

    ImGuiID dockspaceID = ImGui::GetID("dockspace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();
}
