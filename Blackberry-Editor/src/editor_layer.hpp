#pragma once

#include "panels/material_editor_panel.hpp"
#include "panels/scene_renderer_panel.hpp"
#include "panels/asset_manager_panel.hpp"

#include "blackberry.hpp"

namespace BlackberryEditor {

    enum class EditorState {
        Edit,
        Simulate,
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
    
        virtual void OnUpdate() override;
        virtual void OnUIRender() override;
        virtual void OnEvent(const Blackberry::Event& event) override;
    
    private:
        void UI_Toolbar();
        void UI_FileBrowser();
        void UI_Explorer();
        void UI_Properties();
        void UI_Viewport();
    
        void UI_NewProject();
        void UI_NewScene();
    
        void NewProject();
        void NewScene();

        void OnScenePlay();
        void OnSceneSimulate();
        void OnSceneStop();
        void OnScenePause();

        // editor specific functions
        void SaveEditorState();
        void LoadEditorState();

        void SaveProject();
    
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
        f32 m_EditorCameraSpeed = 2.0f;
    
        Blackberry::EntityID m_SelectedEntity = entt::null;
        bool m_IsEntitySelected = false;
        GizmoState m_GizmoState = GizmoState::None;
    
        Blackberry::Ref<Blackberry::Framebuffer> m_RenderTexture;
        BlRec m_ViewportBounds;
        f32 m_ViewportScale = 1.0f;
        bool m_ViewportHovered = false;;

        Blackberry::Ref<Blackberry::Framebuffer> m_OutlineTexture;
    
        Blackberry::Ref<Blackberry::Scene> m_EditingScene;
        Blackberry::Ref<Blackberry::Scene> m_RuntimeScene;
        Blackberry::Ref<Blackberry::Scene> m_CurrentScene;
        EditorState m_EditorState = EditorState::Edit;
        Blackberry::FS::Path m_EditingScenePath;
    
        Blackberry::FS::Path m_CurrentDirectory;
        Blackberry::FS::Path m_BaseDirectory;

        Blackberry::FS::DirectoryIterator m_CurrentDirectoryIterator;
        bool m_DirtyCurrentDirectoryIterator = true;
    
        // icons
        Blackberry::Ref<Blackberry::Texture> m_DirectoryIcon;
        Blackberry::Ref<Blackberry::Texture> m_FileIcon;
        Blackberry::Ref<Blackberry::Texture> m_BackDirectoryIcon;
        Blackberry::Ref<Blackberry::Texture> m_ReloadDirectoryIcon;

        Blackberry::Ref<Blackberry::Texture> m_PlayIcon;
        Blackberry::Ref<Blackberry::Texture> m_SimulateIcon;
        Blackberry::Ref<Blackberry::Texture> m_StopIcon;
        Blackberry::Ref<Blackberry::Texture> m_PauseIcon;
        Blackberry::Ref<Blackberry::Texture> m_ResumeIcon;

        Blackberry::Ref<Blackberry::Framebuffer> m_SavedGBuffer;
    
        bool m_ShowDemoWindow = false;

        Blackberry::FS::Path m_AppDataDirectory;

        Blackberry::Mesh m_Duck;
    };

} // namespace BlackberryEditor