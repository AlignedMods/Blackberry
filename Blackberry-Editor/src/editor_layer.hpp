#pragma once

#include "panels/material_editor_panel.hpp"
#include "panels/scene_renderer_panel.hpp"
#include "panels/asset_manager_panel.hpp"

#include "blackberry.hpp"

namespace BlackberryEditor {

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
    
        void UI_NewProject();
        void UI_NewScene();
    
        void NewProject();
        void NewScene();

        void OnScenePlay();
        void OnSceneStop();
        void OnScenePause();

        // editor specific functions
        void SaveEditorState();
        void LoadEditorState();
    
    private:
        // panels
        MaterialEditorPanel m_MaterialEditorPanel;
        bool m_MaterialEditorPanelOpen = true;

        SceneRendererPanel m_SceneRendererPanel;
        bool m_SceneRendererPanelOpen = true;

        AssetManagerPanel m_AssetManagerPanel;
        bool m_AssetManagerPanelOpen = false;

        bool m_ShowNewProjectWindow = false;
        bool m_ShowNewSceneWindow = false;
    
        Blackberry::Font m_EditorFont;
        Blackberry::SceneCamera m_EditorCamera;
        Blackberry::SceneCamera m_RuntimeCamera;
        Blackberry::SceneCamera* m_CurrentCamera = nullptr;
    
        Blackberry::EntityID m_SelectedEntity = entt::null;
        bool m_IsEntitySelected = false;
        GizmoState m_GizmoState = GizmoState::None;
    
        Blackberry::RenderTexture m_RenderTexture;
        BlRec m_ViewportBounds;
        f32 m_ViewportScale = 1.0f;
        bool m_ViewportHovered = false;

        Blackberry::RenderTexture m_MaskTexture;

        Blackberry::Shader m_OutlineShader;

        Blackberry::RenderTexture m_OutlineTexture;
    
        Blackberry::Scene* m_EditingScene = nullptr;
        Blackberry::Scene* m_RuntimeScene = nullptr;
        Blackberry::Scene* m_CurrentScene = nullptr;
        EditorState m_EditorState = EditorState::Edit;
    
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_BaseDirectory;
    
        // icons
        Blackberry::Texture2D m_DirectoryIcon;
        Blackberry::Texture2D m_FileIcon;
        Blackberry::Texture2D m_BackDirectoryIcon;

        Blackberry::Texture2D m_PlayIcon;
        Blackberry::Texture2D m_StopIcon;
        Blackberry::Texture2D m_PauseIcon;
        Blackberry::Texture2D m_ResumeIcon;
    
        bool m_ShowDemoWindow = false;

        std::filesystem::path m_AppDataDirectory;

        Blackberry::Mesh m_Duck;
    };

} // namespace BlackberryEditor