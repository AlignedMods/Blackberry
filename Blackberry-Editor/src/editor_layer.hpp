#pragma once

#include "blackberry.hpp"

struct EditorScene {
    std::string Name;
    Blackberry::Scene Scene;
    std::filesystem::path ScenePath;
};

struct EditorProject {
    std::string Name;
    std::filesystem::path AssetDirectory;
    std::filesystem::path ProjectDirectory;

    std::vector<EditorScene> Scenes;
};

class EditorLayer : public Blackberry::Layer {
public:
    ~EditorLayer();

    virtual void OnInit() override;

    virtual void OnUpdate(f32 ts) override;
    virtual void OnRender() override;
    virtual void OnUIRender() override;
    virtual void OnEvent(const Blackberry::Event& event) override;

private:
    void UI_AssetManager();
    void UI_Explorer();
    void UI_Properties();
    void UI_Viewport();

    void UI_NewProject();

    void LoadProject();
    void LoadProjectFromPath(const std::filesystem::path& path);
    void SaveProject();

    Blackberry::Scene* LoadSceneFromFile(const std::filesystem::path& path);
    void SaveSceneToFile(Blackberry::Scene& scene, const std::filesystem::path& path);

    void NewProject();
    void NewScene();

private:
    bool m_ShowNewProjectWindow = false;

    Blackberry::Font m_EditorFont;

    Blackberry::EntityID m_SelectedEntity = 0;
    bool m_IsEntitySelected = false;

    BlRenderTexture m_RenderTexture;

    EditorProject m_CurrentProject;
    Blackberry::Scene* m_EditingScene = nullptr;

    std::filesystem::path m_CurrentDirectory;
    std::filesystem::path m_BaseDirectory;

    // textures
    BlTexture m_DirectoryIcon;
    BlTexture m_FileIcon;
    BlTexture m_BackDirectoryIcon;

    bool m_ShowDemoWindow = false;
};
