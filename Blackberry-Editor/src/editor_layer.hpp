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
    void UI_AssetManager();
    void UI_Explorer();
    void UI_Properties();
    void UI_Viewport();

    void LoadProject();
    void LoadProjectFromPath(const std::filesystem::path& path);

    void LoadAssetRegistryFromFile(const std::filesystem::path& path);
    void LoadSceneFromFile(const std::filesystem::path& path);

    void SaveProject();

    void SaveAssetRegistryToFile(const std::filesystem::path& path);
    void SaveSceneToFile(const std::filesystem::path& path);

private:
    Blackberry::Scene m_EditorScene;
    Blackberry::Font m_EditorFont;

    Blackberry::EntityID m_SelectedEntity = 0;
    bool m_IsEntitySelected = false;

    BlRenderTexture m_RenderTexture;

    BlTexture m_BlankTexture;

    Blackberry::AssetManager m_AssetManager;

    std::filesystem::path m_ProjectPath;
    std::filesystem::path m_AssetDir;

    std::filesystem::path m_CurrentAssetRegistryPath;
    std::filesystem::path m_CurrentScenePath;
};
