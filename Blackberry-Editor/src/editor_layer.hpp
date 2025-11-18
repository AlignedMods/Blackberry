#pragma once

#include "blackberry.hpp"

namespace BlackberryEditor {

    struct EditorScene {
        std::string Name;
        Blackberry::Scene Scene;
        std::filesystem::path ScenePath;
    };
    
    struct EditorProject {
        std::string Name;
        std::filesystem::path AssetDirectory;
        std::filesystem::path ProjectDirectory;
        std::filesystem::path ProjectFilePath;
    
        std::vector<EditorScene> Scenes;
    };

    enum class EditorState {
        Edit,
        Play
    };

    enum class GizmoState {
        None = 0,
        Move,
        Rotate,
        Scale
    };
    
    class EditorLayer : public Blackberry::Layer {
    public:
        virtual void OnAttach() override;
        virtual void OnDetach() override;
    
        virtual void OnUpdate(f32 ts) override;
        virtual void OnRender() override;
        virtual void OnUIRender() override;
        virtual void OnOverlayRender() override;
        virtual void OnEvent(const Blackberry::Event& event) override;
    
    private:
        void UI_Toolbar();
        void UI_FileBrowser();
        void UI_AssetManager();
        void UI_Explorer();
        void UI_Properties();
        void UI_Viewport();
        void UI_RendererStats();
    
        void UI_NewProject();
        void UI_NewScene();
    
        void LoadProject();
        void LoadProjectFromPath(const std::filesystem::path& path);
        void SaveProject();
    
        Blackberry::Scene* LoadSceneFromFile(const std::filesystem::path& path);
        void SaveSceneToFile(Blackberry::Scene& scene, const std::filesystem::path& path);
    
        void NewProject();
        void NewScene();

        void OnScenePlay();
        void OnSceneStop();
        void OnScenePause();

        // editor specific functions
        void SaveEditorState();
        void LoadEditorState();
    
    private:
        bool m_ShowNewProjectWindow = false;
        bool m_ShowNewSceneWindow = false;
    
        Blackberry::Font m_EditorFont;
        Blackberry::SceneCamera m_EditorCamera;
        Blackberry::SceneCamera m_RuntimeCamera;
    
        Blackberry::EntityID m_SelectedEntity = entt::null;
        bool m_IsEntitySelected = false;
        GizmoState m_GizmoState = GizmoState::None;
    
        BlRenderTexture m_RenderTexture;
        BlRec m_ViewportBounds;
        f32 m_ViewportScale = 1.0f;
        bool m_ViewportHovered = false;

        BlRenderTexture m_MaskTexture;

        BlShader m_OutlineShader;

        BlRenderTexture m_OutlineTexture;
    
        EditorProject m_CurrentProject;
        Blackberry::Scene* m_EditingScene = nullptr;
        Blackberry::Scene* m_RuntimeScene = nullptr;
        Blackberry::Scene* m_CurrentScene = nullptr;
        EditorState m_EditorState = EditorState::Edit;
    
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_BaseDirectory;
    
        // textures
        BlTexture m_DirectoryIcon;
        BlTexture m_FileIcon;
        BlTexture m_BackDirectoryIcon;
        BlTexture m_PlayIcon;
        BlTexture m_StopIcon;
        BlTexture m_PauseIcon;
    
        bool m_ShowDemoWindow = false;

        std::filesystem::path m_AppDataDirectory;
    };

} // namespace BlackberryEditor