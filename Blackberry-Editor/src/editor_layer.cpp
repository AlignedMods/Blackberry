#include "editor_layer.hpp"
#include "sdf_generation.hpp"

#include "ImGuizmo.h"
#include "glm/gtx/string_cast.hpp"

#include <fstream>
#include <algorithm>

using namespace Blackberry; // don't care about your personal opinion, this is fine

namespace BlackberryEditor {

    static char s_Buffer[512];

#pragma region HelperFunctions
    
    template <typename T, typename F>
    static void DrawComponent(const std::string& name, Entity entity, F uiFunction) {
        bool useUIFunction = false;

        ImGui::PushID(name.c_str());

        if (entity.HasComponent<T>()) {
            T& component = entity.GetComponent<T>();
    
            if (ImGui::CollapsingHeader(name.c_str())) {
                useUIFunction = true;
            }

            if (ImGui::BeginPopupContextItem("ComponentPopup")) {
                if (ImGui::MenuItem("Remove Component")) {
                    entity.RemoveComponent<T>();
                }

                ImGui::EndPopup();
            }

            if (useUIFunction) {
                uiFunction(component);
            }
        }

        ImGui::PopID();
    }
    
    template <typename T>
    static void AddComponentListOption(const std::string& name, Entity& entity, const T& component = T{}) {
        if (entity.HasComponent<T>()) { return; }
    
        if (ImGui::Button(name.c_str())) {
            entity.AddComponent<T>(component);
            ImGui::CloseCurrentPopup();
        }
    }
    
    static void DrawVec2Control(const std::string& label, BlVec2* vec, const char* fmtX = "X", const char* fmtY = "Y") {
        ImGuiIO& io = ImGui::GetIO();
    
        ImGui::PushID(label.c_str());

        ImGui::Text("%s", label.c_str());

        ImGui::Indent();
    
        // x axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
        ImGui::Button(fmtX);
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        ImGui::DragFloat("##DragX", &vec->x, 1.0f);
    
        // y axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
        ImGui::Button(fmtY);
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        ImGui::DragFloat("##DragY", &vec->y);

        ImGui::Unindent();
    
        ImGui::PopID();
    }

    static bool DrawVec3Control(const std::string& label, BlVec3* vec, const char* fmtX = "X", const char* fmtY = "Y", const char* fmtZ = "Z") {
        ImGuiIO& io = ImGui::GetIO();

        bool used = false;
    
        ImGui::PushID(label.c_str());
    
        // label
        ImGui::Text("%s", label.c_str());

        ImGui::Indent();
    
        // x axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
        ImGui::Button(fmtX);
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        used = (used || ImGui::DragFloat("##DragX", &vec->x, 1.0f));
    
        // y axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
        ImGui::Button(fmtY);
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        used = (used || ImGui::DragFloat("##DragY", &vec->y));

        // z axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 1.0f, 0.4f));
        ImGui::Button(fmtZ);
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        used = (used || ImGui::DragFloat("##DragZ", &vec->z));

        ImGui::Unindent();
    
        ImGui::PopID();

        return used;
    }

    static void DrawVec4Control(const std::string& label, BlVec4* vec, const char* fmtX = "X", const char* fmtY = "Y", const char* fmtZ = "Z", const char* fmtW = "W") {
        ImGuiIO& io = ImGui::GetIO();
    
        ImGui::PushID(label.c_str());
    
        // label
        ImGui::Text("%s", label.c_str());

        ImGui::Indent();
    
        // x axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
        ImGui::Button(fmtX);
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        ImGui::DragFloat("##DragX", &vec->x, 1.0f);
    
        // y axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
        ImGui::Button(fmtY);
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        ImGui::DragFloat("##DragY", &vec->y);

        // z axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 1.0f, 0.4f));
        ImGui::Button(fmtZ);
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        ImGui::DragFloat("##DragZ", &vec->z);

         // w axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 1.0f, 0.4f));
        ImGui::Button(fmtW);
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        ImGui::DragFloat("##DragW", &vec->w);

        ImGui::Unindent();
    
        ImGui::PopID();
    }

    static void DrawQuatControl(const std::string& label, BlQuat* quat) {
        BlVec4 vec = BlVec4(quat->x, quat->y, quat->z, quat->w);

        DrawVec4Control(label, &vec);

        quat->x = vec.x;
        quat->y = vec.y;
        quat->z = vec.z;
        quat->w = vec.w;
    }

    static void DrawEulerFromQuatControl(const std::string& label, BlQuat* quat) {
        BlVec3 euler = glm::degrees(glm::eulerAngles(*quat));

        bool used = DrawVec3Control(label, &euler);

        if (used) {
            BlQuat outQuat = BlQuat(glm::radians(euler));

            quat->x = outQuat.x;
            quat->y = outQuat.y;
            quat->z = outQuat.z;
            quat->w = outQuat.w;
        }
    }

#pragma endregion
    
#pragma region OverridenFunctions

    void EditorLayer::OnAttach() {
        m_AppDataDirectory = OS::GetAppDataDirectory();
    
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF("Assets/creato_display/CreatoDisplay-Medium.otf", 18);
        io.Fonts->AddFontFromFileTTF("Assets/creato_display/CreatoDisplay-Bold.otf", 18);
    
        FramebufferSpecification spec;
        spec.Width = 1920;
        spec.Height = 1080;
        spec.Attachments = { {0, FramebufferAttachmentType::ColorRGBA8},
                             {1, FramebufferAttachmentType::Depth} };
        spec.ActiveAttachments = { 0 };

        m_RenderTexture = Framebuffer::Create(spec);
        m_OutlineTexture = Framebuffer::Create(spec);

        LoadEditorState();
    
        m_DirectoryIcon     = Texture2D::Create("Assets/Icons/directory.png");
        m_FileIcon          = Texture2D::Create("Assets/Icons/file.png");
        m_BackDirectoryIcon = Texture2D::Create("Assets/Icons/back_directory.png");

        m_PlayIcon          = Texture2D::Create("Assets/Icons/play.png");
        m_SimulateIcon      = Texture2D::Create("Assets/Icons/simulate.png");
        m_StopIcon          = Texture2D::Create("Assets/Icons/stop.png");
        m_PauseIcon         = Texture2D::Create("Assets/Icons/pause.png");
        m_ResumeIcon        = Texture2D::Create("Assets/Icons/resume.png");
    
        m_CurrentDirectory = Project::GetAssetDirecory();
        m_BaseDirectory = Project::GetAssetDirecory();
        m_CurrentDirectoryIterator = m_CurrentDirectory;

        m_EditorCamera.Transform.Scale = BlVec3(m_RenderTexture->Specification.Width, m_RenderTexture->Specification.Height, 1u);

        // gizmo styles
        ImGuizmo::Style& guizmoStyle = ImGuizmo::GetStyle();
        ImVec4* guizmoColors = guizmoStyle.Colors;

        // handles
        guizmoColors[ImGuizmo::COLOR::DIRECTION_X] = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
        guizmoColors[ImGuizmo::COLOR::DIRECTION_Y] = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
        guizmoColors[ImGuizmo::COLOR::DIRECTION_Z] = ImVec4(0.2f, 0.2f, 1.0f, 1.0f);

        guizmoColors[ImGuizmo::COLOR::SELECTION] = ImVec4(0.8f, 0.8f, 0.8f, 0.7f);

        m_EditingScene = Project::GetStartScene();
        m_EditingScenePath = Project::GetSpecification().StartScene;
        m_CurrentScene = m_EditingScene;

        // ImGui::GetIO().IniFilename = std::filesystem::path(m_AppDataDirectory / "Blackberry-Editor" / "editor_layout.ini").string().c_str();
    }

    void EditorLayer::OnDetach() {
        Project::Save();
        SaveEditorState();
    }
    
    void EditorLayer::OnUpdate() {
        m_SavedGBuffer = m_CurrentScene->GetSceneRenderer()->GetState().GBuffer;

        switch (m_EditorState) {
            case EditorState::Edit:
                m_CurrentScene->OnUpdateEditor();
                m_CurrentScene->OnRenderEditor(m_RenderTexture, m_EditorCamera);
                break;
            case EditorState::Simulate:
                m_CurrentScene->OnUpdateRuntime();
                m_CurrentScene->OnRenderEditor(m_RenderTexture, m_EditorCamera);
                break;
            case EditorState::Play:
                m_CurrentScene->OnUpdateRuntime();
                m_CurrentScene->OnRenderRuntime(m_RenderTexture);
                break;
        }

        if (m_EditorState == EditorState::Edit || m_EditorState == EditorState::Simulate) {
            static bool acceptInput = false;

            // viewport specific things
            if (m_ViewportHovered && Input::IsMousePressed(MouseButton::Right)) {
                BL_APP.GetWindow().SetCursorMode(CursorMode::Disabled);
                acceptInput = true;
            }
            
            if (!Input::IsMouseDown(MouseButton::Right) && acceptInput) {
                BL_APP.GetWindow().SetCursorMode(CursorMode::Normal);
                acceptInput = false;
            }
            
            if (acceptInput || m_ViewportHovered) {
                // camera controller
                if (Input::GetScrollLevel() != 0.0f) {
                    m_EditorCameraSpeed += Input::GetScrollLevel();
                }
            
                if (Input::IsMouseDown(MouseButton::Right)) { 
                    BlVec2 delta = Input::GetMouseDelta();

                    f32 sensitivity = 0.015f;
                    
                    f32 yawDelta = -delta.x * sensitivity;
                    f32 pitchDelta = -delta.y * sensitivity;

                    BlQuat yawRot = glm::angleAxis(yawDelta, BlVec3(0, 1, 0));

                    BlVec3 right = m_EditorCamera.Transform.Rotation * BlVec3(1, 0, 0);
                    BlQuat pitchRot = glm::angleAxis(pitchDelta, right);

                    BlQuat newRot = glm::normalize(yawRot * pitchRot * m_EditorCamera.Transform.Rotation);
                    BlVec3 forward = newRot * BlVec3(0, 0, -1);

                    if (glm::abs(forward.y) < 0.99f) {
                        m_EditorCamera.Transform.Rotation = newRot;
                    } else {
                        m_EditorCamera.Transform.Rotation = glm::normalize(yawRot * m_EditorCamera.Transform.Rotation);
                    }
            
                    if (Input::IsKeyDown(KeyCode::W)) {
                        m_EditorCamera.Transform.Position += m_EditorCamera.GetForwardVector() * m_EditorCameraSpeed * BL_APP.GetDeltaTime();
                    }
                    if (Input::IsKeyDown(KeyCode::S)) {
                        m_EditorCamera.Transform.Position -= m_EditorCamera.GetForwardVector() * m_EditorCameraSpeed * BL_APP.GetDeltaTime();
                    }
                    if (Input::IsKeyDown(KeyCode::A)) {
                        m_EditorCamera.Transform.Position -= m_EditorCamera.GetRightVector() * m_EditorCameraSpeed * BL_APP.GetDeltaTime();
                    }
                    if (Input::IsKeyDown(KeyCode::D)) {
                        m_EditorCamera.Transform.Position += m_EditorCamera.GetRightVector() * m_EditorCameraSpeed * BL_APP.GetDeltaTime();
                    }
                }

                if (Input::IsMousePressed(MouseButton::Left) && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver()) {
                    BlVec2 pos = Input::GetMousePosition();
                    
                    pos.x -= m_ViewportBounds.x;
                    pos.y -= m_ViewportBounds.y;

                    f32 u = pos.x / m_ViewportBounds.w;
                    f32 v = 1.0 - pos.y / m_ViewportBounds.h; // NOTE: The ImGui image is technically being rendered "upside down" so we need to flip the y axis

                    u32 fbX = static_cast<u32>(u * 1920);
                    u32 fbY = static_cast<u32>(v * 1080);

                    float pixel = m_SavedGBuffer->ReadPixelFloat(4, fbX, fbY);
                    int id = static_cast<int>(pixel);
                    
                    if (id != -1) {
                        m_SelectedEntity = static_cast<EntityID>(id);
                        m_IsEntitySelected = true;
                    } else {
                        m_SelectedEntity = entt::null;
                        m_IsEntitySelected = false;
                    }

                    BL_CORE_INFO("press, pos: {}, {}, id {}", fbX, fbY, id);
                }
            }
        }

        // Outlines
        if (m_IsEntitySelected) {
            DebugRenderer::SetRenderTarget(m_OutlineTexture);
            DebugRenderer::DrawEntityOutline(Entity(m_SelectedEntity, m_CurrentScene));
        }
    }

    void EditorLayer::OnUIRender() {
        BL_PROFILE_SCOPE("EditorLayer::OnUIRender");

        // set up dockspace
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
        ImGuiViewport* viewport = ImGui::GetMainViewport();
    
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::Begin("dockspace window", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                                                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar);
        ImGui::PopStyleVar();
    
        ImGuiID dockspaceID = ImGui::GetID("dockspace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::BeginMenu("New")) {
                    if (ImGui::MenuItem("Project")) {
                        NewProject();
                    }
    
                    if (ImGui::MenuItem("Scene")) {
                        NewScene();
                    }

                    ImGui::MenuItem(std::format("{}", 1.0f / BL_APP.GetDeltaTime()).c_str());
    
                    ImGui::EndMenu();
                }
    
                if (ImGui::MenuItem("Save Project", "CTRL+S")) {
                    SaveProject();
                }
    
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Material Editor", nullptr, &m_MaterialEditorPanelOpen);
                ImGui::MenuItem("Scene Renderer", nullptr, &m_SceneRendererPanelOpen);
                ImGui::MenuItem("Asset Manager", nullptr, &m_AssetManagerPanelOpen);

                ImGui::EndMenu();
            }
    
            ImGui::EndMenuBar();
        }
        
        ImGui::End();
    
        if (m_ShowNewProjectWindow) {
            UI_NewProject();
        }
    
        if (m_ShowNewSceneWindow) {
            UI_NewScene();
        }
    
        UI_Toolbar();
        UI_FileBrowser();
        // UI_AssetManager();
        UI_Explorer();
        UI_Properties();
        UI_Viewport();

        m_MaterialEditorPanel.OnUIRender(m_MaterialEditorPanelOpen);
        m_SceneRendererPanel.SetContext(m_CurrentScene);
        m_SceneRendererPanel.OnUIRender(m_SceneRendererPanelOpen);
        m_AssetManagerPanel.OnUIRender(m_AssetManagerPanelOpen);
    
        if (m_ShowDemoWindow) {
            ImGui::ShowDemoWindow(&m_ShowDemoWindow);
        }
    }
    
    void EditorLayer::OnEvent(const Event& event) {
        if (event.GetEventType() == EventType::WindowResize) {
            auto& wr = BL_EVENT_CAST(WindowResizeEvent);
    
            // m_RenderTexture.Delete();
            // m_RenderTexture.Create(wr.GetWidth(), wr.GetHeight());
        }
    
        if (event.GetEventType() == EventType::KeyPressed) {
            auto& kp = BL_EVENT_CAST(KeyPressedEvent);
            if (kp.GetKeyCode() == KeyCode::F3) {
                m_ShowDemoWindow = true;
            }

            // ImGui keybinds
            if (kp.GetKeyCode() == KeyCode::S && Input::IsKeyDown(KeyCode::Ctrl)) {
                SaveProject();
            }

            if (ImGui::GetIO().WantCaptureKeyboard) {
                return;
            }

            if (kp.GetKeyCode() == KeyCode::Num0) {
                m_GizmoState = GizmoState::None;
            }
            if (kp.GetKeyCode() == KeyCode::Num1) {
                m_GizmoState = GizmoState::Move;
            }
            if (kp.GetKeyCode() == KeyCode::Num2) {
                m_GizmoState = GizmoState::Rotate;
            }
            if (kp.GetKeyCode() == KeyCode::Num3) {
                m_GizmoState = GizmoState::Scale;
            }

            if (kp.GetKeyCode() == KeyCode::Del) {
                if (m_IsEntitySelected) {
                    Entity e(m_SelectedEntity, m_CurrentScene);

                    u64 uuid = e.GetComponent<TagComponent>().UUID;
                    m_CurrentScene->DestroyEntity(uuid);

                    m_IsEntitySelected = false;
                    m_SelectedEntity = entt::null;
                }
            }

            if (kp.GetKeyCode() == KeyCode::D && Input::IsKeyDown(KeyCode::Ctrl)) {
                if (m_IsEntitySelected) {
                    Entity e(m_SelectedEntity, m_CurrentScene);

                    m_CurrentScene->CopyEntity(m_SelectedEntity);
                }
            }

            if (kp.GetKeyCode() == KeyCode::F) {
                if (m_IsEntitySelected) {
                    Entity e(m_SelectedEntity, m_CurrentScene);

                    if (e.HasComponent<TransformComponent>()) {
                        auto& transform = e.GetComponent<TransformComponent>();

                        f32 size = glm::max(transform.Scale.x, glm::max(transform.Scale.y, transform.Scale.z));
                        f32 distance = size * 3.0f;

                        m_EditorCamera.Transform.Position = transform.Position - m_EditorCamera.GetForwardVector() * distance;
                    }
                }
            }
        }
    }

#pragma endregion

#pragma region UIFunctions
    
    void EditorLayer::UI_Toolbar() {
        ImGuiWindowClass windowClass;
        windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoUndocking;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::SetNextWindowClass(&windowClass);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::Begin("##Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::PopStyleColor();

        auto playIcon = (m_EditorState == EditorState::Play) ? m_StopIcon : m_PlayIcon;
        auto simulateIcon = (m_EditorState == EditorState::Simulate) ? m_StopIcon : m_SimulateIcon;
        auto pauseIcon = (m_CurrentScene->IsPaused()) ? m_ResumeIcon : m_PauseIcon;

        f32 size = ImGui::GetWindowHeight() - 4.0f;
        f32 firstButtonPos = (ImGui::GetWindowContentRegionMax().x * 0.5f - size * 0.5f);
        firstButtonPos -= size;

		ImGui::SetCursorPosX(firstButtonPos);
    
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.0f));

        if (ImGui::ImageButton("##PlayButton", playIcon->ID, ImVec2(size, size))) {
            if (m_EditorState == EditorState::Edit) {
                OnScenePlay();
            } else if (m_EditorState == EditorState::Play) {
                OnSceneStop();
            }
        }

        ImGui::SameLine();

        if (ImGui::ImageButton("##SimulateButton", simulateIcon->ID, ImVec2(size, size))) {
            if (m_EditorState == EditorState::Edit) {
                OnSceneSimulate();
            } else if (m_EditorState == EditorState::Simulate) {
                OnSceneStop();
            }
        }

        ImGui::SameLine();

        if (ImGui::ImageButton("##PauseButton", pauseIcon->ID, ImVec2(size, size))) {
            m_CurrentScene->SetPaused(!m_CurrentScene->IsPaused());
        }

        ImGui::PopStyleColor();
    
        ImGui::End();

        ImGui::PopStyleVar();
    }

    void EditorLayer::UI_FileBrowser() {
        if (m_DirtyCurrentDirectoryIterator) {
            m_CurrentDirectoryIterator = FS::DirectoryIterator(m_CurrentDirectory);
            m_DirtyCurrentDirectoryIterator = false;
        }

        ImGui::Begin("File Browser");
    
        if (m_CurrentDirectory != m_BaseDirectory) {
            if (ImGui::ImageButton("##BackDirectory", m_BackDirectoryIcon->ID, ImVec2(32.0f, 32.0f))) {
                m_CurrentDirectory = m_CurrentDirectory.ParentPath();
                m_DirtyCurrentDirectoryIterator = true;
            }
        }
    
        static f32 padding = 16.0f;
        static f32 thumbnailSize = 128.0f;
        f32 cellSize = thumbnailSize + padding;
    
        f32 panelWidth = ImGui::GetContentRegionAvail().x;
        u32 columnCount = static_cast<u32>(panelWidth / cellSize);
        if (columnCount < 1) {
            columnCount = 1;
        }
    
        // ImGui::Columns(columnCount, 0, false);

        static bool s_CreateAssetPopup = false;
        static FS::Path assetFile;

        static std::string s_FileName;
        static bool s_CreateMaterialPopup = false;
        static bool s_CreateScenePopup = false;

        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::BeginMenu("Create")) {
                if (ImGui::MenuItem("Material")) {
                    s_FileName.clear();
                    s_CreateMaterialPopup = true;
                }

                if (ImGui::MenuItem("Scene")) {
                    s_FileName.clear();
                    s_CreateScenePopup = true;
                }

                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        }

        if (s_CreateMaterialPopup) {
            ImGui::OpenPopup("Create Material");
            s_CreateMaterialPopup = false;
        }

        if (s_CreateScenePopup) {
            ImGui::OpenPopup("Create Scene");
            s_CreateScenePopup = false;
        }

        bool _cmopen = true;
        if (ImGui::BeginPopupModal("Create Material", &_cmopen)) {
            ImGui::InputText("File Name", &s_FileName);

            ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - 30.0f);
            if (ImGui::Button("Create")) {
                FS::Path p = m_CurrentDirectory / s_FileName;

                Material mat = Material::Create();
                Material::Save(mat, p);

                u64 handle = Project::GetAssetManager().AddAsset({FS::Relative(p, m_BaseDirectory), AssetType::Material, mat});

                m_MaterialEditorPanel.SetContext(handle);

                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        bool _csopen = true;
        if (ImGui::BeginPopupModal("Create Scene", &_csopen)) {
            ImGui::InputText("File Name", &s_FileName);

            ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - 30.0f);
            if (ImGui::Button("Create")) {
                FS::Path p = m_CurrentDirectory / s_FileName;

                std::ofstream file(p.String());
                file.close();

                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (ImGui::BeginTable("##FunnyTable", columnCount)) {
            for (const auto& file : m_CurrentDirectoryIterator) { // NOTE: slow
                ImGui::TableNextColumn();
            
                const auto& path = file.Path();
                const auto relative = FS::Relative(path, m_BaseDirectory);
                std::string name = path.FileName().String();
            
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            
                if (file.IsDirectory()) {
                    ImGui::ImageButton(name.c_str(), m_DirectoryIcon->ID, ImVec2(thumbnailSize, thumbnailSize));
                } else {
                    ImGui::ImageButton(name.c_str(), m_FileIcon->ID, ImVec2(thumbnailSize, thumbnailSize));
                }
            
                ImGui::PopStyleVar();
            
                if (file.IsFile()) {
                    if (ImGui::BeginDragDropSource()) {
                        std::string filePath = relative.String();
                        ImGui::SetDragDropPayload("FILE_BROWSER_FILE_DRAG_DROP", filePath.c_str(), filePath.size() + 1);
                        ImGui::Text("%s", name.c_str());
                        ImGui::EndDragDropSource();
                    }
                }
            
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    if (file.IsDirectory()) {
                        m_CurrentDirectory /= path.FileName();
                        m_DirtyCurrentDirectoryIterator = true;
                    } else {
                        // Handle specific behavior about an asset
                        if (Project::GetAssetManager().ContainsAsset(relative)) {
                            Asset& asset = Project::GetAssetManager().GetAssetFromPath(relative);
                            if (asset.Type == AssetType::Material) {
                                m_MaterialEditorPanel.SetContext(asset.Handle);
                            }
                        }
                    }
                }
            
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Create Asset Out Of Item")) {
                        s_CreateAssetPopup = true;
                        assetFile = file.Path();
                    }
                    ImGui::EndPopup();
                }
            
                ImGui::TextWrapped(name.c_str());
            }
            
            ImGui::EndTable();
        }

        if (s_CreateAssetPopup) {
            ImGui::OpenPopup("Create Asset?");
            s_CreateAssetPopup = false;
        }

        static bool s_CreateAssetPopupOpen = true;

        if (ImGui::BeginPopupModal("Create Asset?")) {
            static const char* assetTypeNames[] = { "Texture", "Font", "Model", "Material", "Enviroment Map" };
            static int currentAssetType = 0;

            // ImGui::Combo("##AssetType", &currentAssetType, assetTypeNames, IM_ARRAYSIZE(assetTypeNames));

            ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - 25.0f);

            if (ImGui::Button("Create")) {
                // if (currentAssetType == 0) { // texture
                //     Project::GetAssetManager().AddTextureFromPath(FS::Relative(assetFile, m_BaseDirectory));
                // } else if (currentAssetType == 1) { // font
                //     Project::GetAssetManager().AddFontFromPath(FS::Relative(assetFile, m_BaseDirectory));
                // } else if (currentAssetType == 2) { // model
                //     Project::GetAssetManager().AddModelFromPath(FS::Relative(assetFile, m_BaseDirectory));
                // } else if (currentAssetType == 3) { // material
                //     Project::GetAssetManager().AddMaterialFromPath(FS::Relative(assetFile, m_BaseDirectory));
                // } else if (currentAssetType == 4) { // env map
                //     Project::GetAssetManager().AddEnviromentMapFromPath(FS::Relative(assetFile, m_BaseDirectory));
                // }

                Project::GetAssetManager().LoadAssetFromPath(FS::Relative(assetFile, m_BaseDirectory));
                
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    
        ImGui::End();
    }

    void EditorLayer::UI_Explorer() {
        ImGui::Begin("Explorer");

        ImGui::SeparatorText("Entities: ");

        if (ImGui::BeginPopupContextWindow("ExplorerContextMenu")) {
            if (ImGui::MenuItem("Add Empty Entity")) {
                m_CurrentScene->CreateEntity("Blank Entity");
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Mesh")) {
                if (ImGui::MenuItem("Plane")) {
                    Entity entity(m_CurrentScene->CreateEntity("Plane"), m_CurrentScene);

                    if (!Project::GetAssetManager().ContainsAsset("Meshes/Default/Plane.glb")) {
                        Model m = Model::Create(Project::GetAssetPath("Meshes/Default/Plane.glb"));

                        Project::GetAssetManager().AddAsset({"Meshes/Default/Plane.glb", AssetType::Model, m});
                    }

                    entity.AddComponent<MeshComponent>({Project::GetAssetManager().GetAssetFromPath("Meshes/Default/Plane.glb").Handle});
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlQuat(), BlVec3(1.0f, 1.0f, 1.0f)});
                    entity.AddComponent<RigidBodyComponent>({RigidBodyType::Static});
                    entity.AddComponent<BoxColliderComponent>({BlVec3(1.0f, 0.0f, 1.0f)});
                };

                if (ImGui::MenuItem("Cube")) {
                    Entity entity(m_CurrentScene->CreateEntity("Cube"), m_CurrentScene);

                    if (!Project::GetAssetManager().ContainsAsset("Meshes/Default/Cube.glb")) {
                        Model m = Model::Create(Project::GetAssetPath("Meshes/Default/Cube.glb"));

                        Project::GetAssetManager().AddAsset({"Meshes/Default/Cube.glb", AssetType::Model, m});
                    }

                    entity.AddComponent<MeshComponent>({Project::GetAssetManager().GetAssetFromPath("Meshes/Default/Cube.glb").Handle});
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlQuat(), BlVec3(1.0f)});
                    entity.AddComponent<RigidBodyComponent>({RigidBodyType::Static});
                    entity.AddComponent<BoxColliderComponent>({BlVec3(1.0f, 1.0f, 1.0f)});
                };

                if (ImGui::MenuItem("Sphere")) {
                    Entity entity(m_CurrentScene->CreateEntity("Sphere"), m_CurrentScene);

                    if (!Project::GetAssetManager().ContainsAsset("Meshes/Default/Sphere.glb")) {
                        Model m = Model::Create(Project::GetAssetPath("Meshes/Default/Sphere.glb"));

                        Project::GetAssetManager().AddAsset({"Meshes/Default/Sphere.glb", AssetType::Model, m});
                    }

                    entity.AddComponent<MeshComponent>({Project::GetAssetManager().GetAssetFromPath("Meshes/Default/Sphere.glb").Handle});
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlQuat(), BlVec3(1.0f)});
                    entity.AddComponent<RigidBodyComponent>({RigidBodyType::Static});
                    entity.AddComponent<SphereColliderComponent>();
                }

                if (ImGui::MenuItem("Cylinder")) {
                    Entity entity(m_CurrentScene->CreateEntity("Cylinder"), m_CurrentScene);

                    if (!Project::GetAssetManager().ContainsAsset("Meshes/Default/Cylinder.glb")) {
                        Model m = Model::Create(Project::GetAssetPath("Meshes/Default/Cylinder.glb"));

                        Project::GetAssetManager().AddAsset({"Meshes/Default/Cylinder.glb", AssetType::Model, m});
                    }

                    entity.AddComponent<MeshComponent>({Project::GetAssetManager().GetAssetFromPath("Meshes/Default/Cylinder.glb").Handle});
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlQuat(), BlVec3(1.0f)});
                }

                if (ImGui::MenuItem("Torus")) {
                    Entity entity(m_CurrentScene->CreateEntity("Torus"), m_CurrentScene);

                    if (!Project::GetAssetManager().ContainsAsset("Meshes/Default/Torus.glb")) {
                        Model m = Model::Create(Project::GetAssetPath("Meshes/Default/Torus.glb"));

                        Project::GetAssetManager().AddAsset({"Meshes/Default/Torus.glb", AssetType::Model, m});
                    }

                    entity.AddComponent<MeshComponent>({Project::GetAssetManager().GetAssetFromPath("Meshes/Default/Torus.glb").Handle});
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlQuat(), BlVec3(1.0f)});
                }
                
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Camera")) {
                if (ImGui::MenuItem("From view")) {
                    Entity entity(m_CurrentScene->CreateEntity("Camera"), m_CurrentScene);

                    entity.AddComponent<TransformComponent>(m_EditorCamera.Transform);
                    entity.AddComponent<CameraComponent>(m_EditorCamera.Camera);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Light")) {
                if (ImGui::MenuItem("Directional Light")) {
                    Entity entity(m_CurrentScene->CreateEntity("Directional Light"), m_CurrentScene);

                    entity.AddComponent<DirectionalLightComponent>();
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlQuat(), BlVec3(1.0f)});
                }

                if (ImGui::MenuItem("Point Light")) {
                    Entity entity(m_CurrentScene->CreateEntity("Point Light"), m_CurrentScene);

                    entity.AddComponent<PointLightComponent>();
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlQuat(), BlVec3(1.0f)});
                }

                if (ImGui::MenuItem("Spot Light")) {
                    Entity entity(m_CurrentScene->CreateEntity("Spot Light"), m_CurrentScene);

                    entity.AddComponent<SpotLightComponent>();
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlQuat(), BlVec3(1.0f)});
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Enviroment")) {
                if (ImGui::MenuItem("Enviroment")) {
                    Entity entity(m_CurrentScene->CreateEntity("Enviroment"), m_CurrentScene);

                    entity.AddComponent<EnviromentComponent>();
                }

                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        };
        
        for (auto id : m_CurrentScene->GetEntities()) {
            ImGui::PushID(static_cast<u32>(id));
    
            Entity entity(id, m_CurrentScene);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(16, 0));

            if (m_SelectedEntity == id) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.4f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
                
                ImGui::Button(entity.GetComponent<TagComponent>().Name.c_str(), ImVec2(ImGui::GetContentRegionAvail().x - 32.0f, 0));
                
                ImGui::PopStyleColor(3);
            } else if (entity.HasComponent<TagComponent>()) {
                if (ImGui::Button(entity.GetComponent<TagComponent>().Name.c_str(), ImVec2(ImGui::GetContentRegionAvail().x - 32.0f, 0))) {
                    m_IsEntitySelected = true;
                    m_SelectedEntity = id;
                }
            }

            if (ImGui::BeginPopupContextItem("EntityPopup")) {
                if (ImGui::MenuItem("Delete Entity")) {
                    m_CurrentScene->DestroyEntity(entity.GetComponent<TagComponent>().UUID);

                    if (m_SelectedEntity == id) {
                        m_IsEntitySelected = false;
                        m_SelectedEntity = entt::null;
                    }
                }

                if (ImGui::MenuItem("Duplicate Entity")) {
                    // NOTE: calling Scene::DuplicateEntity causes a lot of issues regarding UUIDs
                    // it is fine to call it when duplicating entire scenes but not when duplicating one entity!
                    m_CurrentScene->CopyEntity(m_SelectedEntity);
                }

                ImGui::EndPopup();
            }

            ImGui::PopStyleVar();
    
            ImGui::PopID();
        }

        ImGui::End();
    }
    
    void EditorLayer::UI_Properties() {
        ImGui::Begin("Properties");
    
        if (m_IsEntitySelected) {
            Entity entity(m_SelectedEntity, m_CurrentScene);

            if (ImGui::BeginPopupContextWindow("PropertiesContextMenu")) {
                if (ImGui::BeginMenu("Add Component")) {
                    AddComponentListOption<TransformComponent>("Transform", entity);
                    AddComponentListOption<TextComponent>("Text", entity);
                    AddComponentListOption<MeshComponent>("Mesh", entity);
                    AddComponentListOption<CameraComponent>("Camera", entity);
                    AddComponentListOption<ScriptComponent>("Script", entity);
                    AddComponentListOption<RigidBodyComponent>("Rigid Body", entity);
                    AddComponentListOption<BoxColliderComponent>("Box Collider", entity);
                    AddComponentListOption<SphereColliderComponent>("Sphere Collider", entity);
                    AddComponentListOption<DirectionalLightComponent>("Directional Light", entity);
                    AddComponentListOption<PointLightComponent>("Point Light", entity);
                    AddComponentListOption<SpotLightComponent>("Spot Light", entity);
                    AddComponentListOption<EnviromentComponent>("Enviroment", entity);
                    
                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }
    
            TagComponent& tag = entity.GetComponent<TagComponent>();

            ImGui::Text("Name: "); ImGui::SameLine();
            ImGui::InputText("##EntityName", &tag.Name);
            ImGui::TextDisabled("UUID: %llu", tag.UUID);

            ImGui::SeparatorText("Components: ");

            DrawComponent<TransformComponent>("Transform2D", entity, [](TransformComponent& transform) {
                DrawVec3Control("Position: ", &transform.Position);
                ImGui::Separator();

                DrawEulerFromQuatControl("Rotation: ", &transform.Rotation);
                ImGui::Separator();

                DrawVec3Control("Scale: ", &transform.Scale);
            });
            DrawComponent<MeshComponent>("Mesh", entity, [this](MeshComponent& mesh) {
                ImGui::Separator();
                ImGui::Text("Mesh: ");
                ImGui::Indent();

                std::string meshName;
                if (Project::GetAssetManager().ContainsAsset(mesh.MeshHandle)) {
                    meshName = Project::GetAssetManager().GetAsset(mesh.MeshHandle).FilePath.Stem().String();
                } else {
                    meshName = "NULL";
                }

                f32 size = ImGui::GetContentRegionAvail().x;
                if (meshName == "NULL") {
                    ImGui::PushStyleColor(ImGuiCol_Text, 0xff0000ff);
                    ImGui::Button(meshName.c_str(), ImVec2(size, 0.0f));
                    ImGui::PopStyleColor();
                } else {
                    ImGui::Button(meshName.c_str(), ImVec2(size, 0.0f));
                }

                if (ImGui::BeginPopupContextItem("MeshHandlePopup")) {
                    if (ImGui::MenuItem("Remove Mesh")) {
                        mesh.MeshHandle = 0;
                    }

                    ImGui::EndPopup();
                }

                if (ImGui::BeginDragDropTarget()) {
                    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_HANDLE_DRAG_DROP");

                    if (payload) {
                        mesh.MeshHandle = *reinterpret_cast<u64*>(payload->Data); // seems sus
                    }

                    ImGui::EndDragDropTarget();
                }

                ImGui::Unindent();

                ImGui::Text("Materials: ");
                ImGui::Indent();
                
                if (Project::GetAssetManager().ContainsAsset(mesh.MeshHandle)) {
                    Model& model = std::get<Model>(Project::GetAssetManager().GetAsset(mesh.MeshHandle).Data);
                
                    for (u32 i = 0; i < model.MeshCount; i++) {
                        f32 remainingSpace = ImGui::GetContentRegionAvail().x;
                        ImGui::Text("Material [%u]: ", i); ImGui::SameLine();
                
                        std::string matName;
                        if (Project::GetAssetManager().ContainsAsset(mesh.MaterialHandles[i])) {
                            matName = Project::GetAssetManager().GetAsset(mesh.MaterialHandles[i]).FilePath.Stem().String();
                        } else {
                            matName = "NULL";
                        }
                
                        ImGui::PushID(i);
                        size = ImGui::GetContentRegionAvail().x;
                        if (matName == "NULL") {
                            ImGui::PushStyleColor(ImGuiCol_Text, 0xff0000ff);
                            ImGui::Button(matName.c_str(), ImVec2(size, 0.0f));
                            ImGui::PopStyleColor();
                        } else {
                            ImGui::Button(matName.c_str(), ImVec2(size, 0.0f));
                        }
                
                        if (ImGui::BeginDragDropTarget()) {
                            const ImGuiPayload* assetManagerPayload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_HANDLE_DRAG_DROP");
                            const ImGuiPayload* fileBrowserPayload = ImGui::AcceptDragDropPayload("FILE_BROWSER_FILE_DRAG_DROP");
                
                            if (assetManagerPayload) {
                                mesh.MaterialHandles[i] = *reinterpret_cast<u64*>(assetManagerPayload->Data);
                            } else if (fileBrowserPayload) {
                                FS::Path p = reinterpret_cast<char*>(fileBrowserPayload->Data);
                
                                if (Project::GetAssetManager().ContainsAsset(p)) {
                                    auto& asset = Project::GetAssetManager().GetAssetFromPath(p);
                                    if (asset.Type == AssetType::Material) {
                                        mesh.MaterialHandles[i] = asset.Handle;
                                    }
                                }
                            }
                
                            ImGui::EndDragDropTarget();
                        }
                
                        ImGui::PopID();
                    }
                }
                
                ImGui::Unindent();
            });
            DrawComponent<TextComponent>("Text", entity, [this](TextComponent& text) {
                ImGui::Text("Contents: ");
                ImGui::Indent();
                ImGui::InputTextMultiline("##Contents", &text.Contents);
                ImGui::Unindent();

                ImGui::Separator();
                ImGui::Text("Font: ");
                ImGui::Indent();

                std::string mat;
                if (Project::GetAssetManager().ContainsAsset(text.FontHandle)) {
                    mat = Project::GetAssetManager().GetAsset(text.FontHandle).FilePath.Stem().String();
                } else {
                    mat = "NULL";
                }

                f32 size = ImGui::GetContentRegionAvail().x;
                if (mat == "NULL") {
                    ImGui::PushStyleColor(ImGuiCol_Text, 0xff0000ff);
                    ImGui::Button(mat.c_str(), ImVec2(size, 0.0f));
                    ImGui::PopStyleColor();
                } else {
                    ImGui::Button(mat.c_str(), ImVec2(size, 0.0f));
                }

                if (ImGui::BeginPopupContextItem("FontHandlePopup")) {
                    if (ImGui::MenuItem("Remove Font")) {
                        text.FontHandle = 0;
                    }

                    ImGui::EndPopup();
                }

                if (ImGui::BeginDragDropTarget()) {
                    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_HANDLE_DRAG_DROP");

                    if (payload) {
                        text.FontHandle = *reinterpret_cast<u64*>(payload->Data); // seems sus
                    }
                }

                ImGui::Unindent();

                ImGui::Separator();
                ImGui::Text("Kerning: ");
                ImGui::Indent();
                ImGui::DragFloat("##Kerning", &text.Kerning, 0.005f);
                ImGui::Unindent();
                
                ImGui::Separator();
                ImGui::Text("Line Spacing: ");
                ImGui::Indent();
                ImGui::DragFloat("##LineSpacing", &text.LineSpacing, 0.125f);
                ImGui::Unindent();
            });
            DrawComponent<CameraComponent>("Camera", entity, [this](CameraComponent& camera) {
                ImGui::Text("Zoom: ");
                ImGui::Indent();
                ImGui::DragFloat("##Zoom", &camera.Zoom);
                ImGui::Unindent();
                ImGui::Separator();

                ImGui::Text("Near: ");
                ImGui::Indent();
                ImGui::DragFloat("##CameraNear", &camera.Near);
                ImGui::Unindent();
                ImGui::Separator();

                ImGui::Text("Far: ");
                ImGui::Indent();
                ImGui::DragFloat("##CameraFar", &camera.Far);
                ImGui::Unindent();
            });
            DrawComponent<ScriptComponent>("Script", entity, [this](ScriptComponent& script) {
                auto stringPath = script.ModulePath.String();

                ImGui::Text("Module Path: "); ImGui::SameLine();
                ImGui::InputText("##ModulePath", &stringPath);

                script.ModulePath = stringPath;
                script.FilePath = m_BaseDirectory / script.ModulePath;
            });
            DrawComponent<RigidBodyComponent>("Rigid Body", entity, [](RigidBodyComponent& rigidBody) {
                static const char* const types[] = { "Static", "Dynamic", "Kinematic" };
                static int currentType = static_cast<int>(rigidBody.Type);

                ImGui::Text("Type: ");
                ImGui::Indent();
                if (ImGui::Combo("##Type", &currentType, types, IM_ARRAYSIZE(types))) {
                    rigidBody.Type = static_cast<RigidBodyType>(currentType);
                }
                ImGui::Unindent();

                ImGui::DragFloat("Restitution", &rigidBody.Resitution, 0.05f);
                ImGui::DragFloat("Friction", &rigidBody.Friction, 0.05f);
            });
            DrawComponent<BoxColliderComponent>("Box Collider", entity, [](BoxColliderComponent& collider) {
                DrawVec3Control("Scale", &collider.Scale);
            });
            DrawComponent<SphereColliderComponent>("Sphere Collider", entity, [](SphereColliderComponent& collider) {
                ImGui::DragFloat("Radius", &collider.Radius, 0.05f);
            });
            DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](DirectionalLightComponent& light) {
                ImGui::ColorEdit3("Color", &light.Color.x);
                ImGui::DragFloat("Intensity", &light.Intensity, 0.5f);
            });
            DrawComponent<PointLightComponent>("Point Light", entity, [](PointLightComponent& light) {
                ImGui::ColorEdit3("Color", &light.Color.x);

                ImGui::DragFloat("Radius", &light.Radius, 0.1f);
                ImGui::DragFloat("Intensity", &light.Intensity, 0.5f);
            });
            DrawComponent<SpotLightComponent>("Point Light", entity, [](SpotLightComponent& light) {
                ImGui::ColorEdit3("Color", &light.Color.x);

                ImGui::DragFloat("Cutoff", &light.Cutoff, 0.1f);
                ImGui::DragFloat("Intensity", &light.Intensity, 0.5f);
            });
            DrawComponent<EnviromentComponent>("Enviroment", entity, [](EnviromentComponent& env) {
                ImGui::Text("Enviroment Map: ");
                ImGui::Indent();

                std::string meshName;
                if (Project::GetAssetManager().ContainsAsset(env.EnviromentMap)) {
                    meshName = Project::GetAssetManager().GetAsset(env.EnviromentMap).FilePath.Stem().String();
                } else {
                    meshName = "NULL";
                }

                f32 size = ImGui::GetContentRegionAvail().x;
                if (meshName == "NULL") {
                    ImGui::PushStyleColor(ImGuiCol_Text, 0xff0000ff);
                    ImGui::Button(meshName.c_str(), ImVec2(size, 0.0f));
                    ImGui::PopStyleColor();
                } else {
                    ImGui::Button(meshName.c_str(), ImVec2(size, 0.0f));
                }

                if (ImGui::BeginPopupContextItem("EnvMapHandlePopup")) {
                    if (ImGui::MenuItem("Remove Enviroment Map")) {
                        env.EnviromentMap = 0;
                    }

                    ImGui::EndPopup();
                }

                if (ImGui::BeginDragDropTarget()) {
                    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_HANDLE_DRAG_DROP");

                    if (payload) {
                        env.EnviromentMap = *reinterpret_cast<u64*>(payload->Data); // seems sus
                    }

                    ImGui::EndDragDropTarget();
                }

                ImGui::Unindent();

                ImGui::SliderFloat("Level Of Detail", &env.LevelOfDetail, 0.0f, 4.0f);

                ImGui::Checkbox("Enable Bloom", &env.EnableBloom);
                ImGui::DragFloat("Bloom Threshold", &env.BloomThreshold);
            });
        }
    
        ImGui::End();
    }
    
    void EditorLayer::UI_Viewport() {
        ImGuizmo::BeginFrame();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImGui::PopStyleVar();

        ImVec2 windowArea = ImGui::GetContentRegionAvail();
        f32 scale = windowArea.x / static_cast<f32>(m_RenderTexture->Specification.Width);
        f32 y = m_RenderTexture->Specification.Height * scale;

        // we want to make the viewport the smallest axis
        if (y > windowArea.y) {
            scale = windowArea.y / static_cast<f32>(m_RenderTexture->Specification.Height);
        }
        
        BlVec2 size = BlVec2(m_RenderTexture->Specification.Width * scale, m_RenderTexture->Specification.Height * scale);

        f32 cursorX = ImGui::GetCursorPosX() + windowArea.x / 2.0f - size.x / 2.0f;
        f32 cursorY = ImGui::GetCursorPosY() + (windowArea.y / 2.0f - size.y / 2.0f);

        ImGui::SetCursorPosX(cursorX);
        ImGui::SetCursorPosY(cursorY);
        auto& rendererState = m_CurrentScene->GetSceneRenderer()->GetState();
        ImGui::Image(m_RenderTexture->Attachments[0]->ID, ImVec2(size.x, size.y), ImVec2(0, 1), ImVec2(1, 0));

        if (m_IsEntitySelected) {
            Entity e(m_SelectedEntity, m_CurrentScene);

            if (e.HasComponent<TransformComponent>() && e.HasComponent<MeshComponent>()) {
                ImGui::SetCursorPosX(cursorX);
                ImGui::SetCursorPosY(cursorY);
                ImGui::Image(m_OutlineTexture->Attachments[0]->ID, ImVec2(size.x, size.y), ImVec2(0, 1), ImVec2(1, 0));
            }
        }

        if (ImGui::IsItemHovered()) {
            m_ViewportHovered = true;
        } else {
            m_ViewportHovered = false;
        }

        // m_ViewportScale = scale;
        m_ViewportBounds.x = ImGui::GetWindowPos().x + cursorX;
        m_ViewportBounds.y = ImGui::GetWindowPos().y + cursorY;
        m_ViewportBounds.w = size.x;
        m_ViewportBounds.h = size.y;

        // After drawing the image:
        ImVec2 min = ImGui::GetItemRectMin();  // absolute top-left of the rendered image
        ImVec2 max = ImGui::GetItemRectMax();  // absolute bottom-right

        if (ImGui::BeginDragDropTarget()) {
            if (auto payload = ImGui::AcceptDragDropPayload("FILE_BROWSER_FILE_DRAG_DROP")) {
                bool sceneExists = false;
    
                std::string strPath = reinterpret_cast<char*>(payload->Data);
                FS::Path path(strPath);

                const Asset& asset = Project::GetAssetManager().GetAssetFromPath(path);

                if (asset.Type == AssetType::Scene) {
                    Ref<Scene> scene = std::get<Ref<Scene>>(asset.Data);
                    m_EditingScene = scene;
                    m_CurrentScene = m_EditingScene;
                }
            }
            ImGui::EndDragDropTarget();
        };

        // gizmos
        if (m_IsEntitySelected && m_GizmoState != GizmoState::None) {
            Entity e(m_SelectedEntity, m_CurrentScene);
            if (e.HasComponent<TransformComponent>()) {
                TransformComponent& transform = e.GetComponent<TransformComponent>();
                glm::mat4 transformMatrix = transform.GetMatrix();

                SceneCamera currentCamera = m_CurrentScene->GetSceneRenderer()->GetCamera();

                glm::mat4 camProjection = currentCamera.GetCameraProjection();
                glm::mat4 camView = currentCamera.GetCameraView(); // already inversed

                // prevent imgui from taxing inputs (you are not the irs buddy)
                if (ImGuizmo::IsOver()) {
                    ImGui::GetIO().WantCaptureMouse = false;
                }

                ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;
                
                switch (m_GizmoState) {
                    case GizmoState::Move: operation = ImGuizmo::OPERATION::TRANSLATE; break;
                    case GizmoState::Rotate: operation = ImGuizmo::OPERATION::ROTATE; break;
                    case GizmoState::Scale: operation = ImGuizmo::OPERATION::SCALE; break;
                }

                f32 snapValue = 0.05f;

                if (m_GizmoState == GizmoState::Rotate) {
                    snapValue = 0.5f;
                }

                f32 snapValues[3] = { snapValue, snapValue, snapValue };

                ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
                ImGuizmo::Enable(true);
                ImGuizmo::SetRect(min.x, min.y, max.x - min.x, max.y - min.y);
                ImGuizmo::Manipulate(glm::value_ptr(camView), glm::value_ptr(camProjection), operation, ImGuizmo::MODE::LOCAL, glm::value_ptr(transformMatrix), nullptr, snapValues);

                if (ImGuizmo::IsUsing()) {
                    f32 pos[3], rot[3], scale[3];
                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transformMatrix), pos, rot, scale);

                    transform.Position = BlVec3(pos[0], pos[1], pos[2]);
                    transform.Rotation = BlQuat(BlVec3(glm::radians(rot[0]), glm::radians(rot[1]), glm::radians(rot[2])));
                    transform.Scale    = BlVec3(scale[0], scale[1], scale[2]);
                }
            }
        }
    
        ImGui::End();
    }
    
    void EditorLayer::UI_NewProject() {
        static std::string projectName = "Example";
        static std::string projectPath;
        static std::string assetPath = "Assets/";
    
        ImGui::Begin("New Project", &m_ShowNewProjectWindow);
    
        ImGui::Text("Name: "); ImGui::SameLine();
        ImGui::InputText("##ProjectName", &projectName);
    
        ImGui::Text("Root Directory: "); ImGui::SameLine();
        ImGui::InputText("##FilePath", &projectPath); ImGui::SameLine();
        if (ImGui::SmallButton("...")) {
            projectPath = OS::OpenFile("Blackberry Project (*.blproj)");
        }
    
        ImGui::Text("Asset Directory: "); ImGui::SameLine();
        ImGui::InputText("##AssetDirectory", &assetPath);
    
        if (ImGui::Button("Create")) {
            // m_CurrentProject.Name = projectName;
            // m_CurrentProject.ProjectDirectory = std::filesystem::path(projectPath).parent_path();
            // m_CurrentProject.AssetDirectory = m_CurrentProject.ProjectDirectory / assetPath;
        }
    
        ImGui::End();
    }
    
    void EditorLayer::UI_NewScene() {
        static std::string scenePath;
    
        ImGui::Begin("New Scene", &m_ShowNewSceneWindow);
    
        ImGui::InputText("Scene Path: ", &scenePath); ImGui::SameLine();
    
        if (ImGui::Button("...")) {
            scenePath = OS::OpenFile("Blackberry Scene (*.blscene)");
        }
    
        if (ImGui::Button("Create")) {
            // EditorScene scene;
            // scene.ScenePath = scenePath;
            // scene.Name = "New Scene";
            // auto& newScene = m_CurrentProject.Scenes.emplace_back(scene);
            // m_EditingScene = &newScene.Scene;
            // m_CurrentScene = m_EditingScene;
            // 
            // std::ofstream file(scenePath);
            // file.close();
            // 
            // m_ShowNewSceneWindow = false;
        };
    
        ImGui::End();
    }

#pragma endregion

#pragma region ProjectFunctions

    void EditorLayer::NewProject() {
        m_ShowNewProjectWindow = true;
    }
    
    void EditorLayer::NewScene() {
        m_ShowNewSceneWindow = true;
    }

#pragma endregion

#pragma region SceneFunctions

    void EditorLayer::OnScenePlay() {
        BL_INFO("Switched to playing scene.");
        m_EditorState = EditorState::Play;

        // create new scene
        m_RuntimeScene = Scene::Copy(m_EditingScene);
        m_CurrentScene = m_RuntimeScene;

        m_CurrentScene->OnRuntimeStart();
    }

    void EditorLayer::OnSceneSimulate() {
        BL_INFO("Switched to simulating scene.");
        m_EditorState = EditorState::Simulate;

        // create new scene
        m_RuntimeScene = Scene::Copy(m_EditingScene);
        m_CurrentScene = m_RuntimeScene;

        m_CurrentScene->OnRuntimeStart();
    }

    void EditorLayer::OnSceneStop() {
        m_CurrentScene->OnRuntimeStop();

        BL_INFO("Reverted to editing scene.");
        m_EditorState = EditorState::Edit;
        m_CurrentScene = m_EditingScene;
    }

    void EditorLayer::OnScenePause() {
    }

    void EditorLayer::SaveEditorState() {
        if (!FS::Exists(m_AppDataDirectory / "Blackberry-Editor")) {
            // std::filesystem::create_directory(m_AppDataDirectory / "Blackberry-Editor");
        }

        json j;
        j["LastProjectPath"] = Project::GetProjectPath();
        j["CameraTransform"] = {
            {"Position", { m_EditorCamera.Transform.Position.x, m_EditorCamera.Transform.Position.y, m_EditorCamera.Transform.Position.z }},
            {"Rotation", { m_EditorCamera.Transform.Rotation.x, m_EditorCamera.Transform.Rotation.y, m_EditorCamera.Transform.Rotation.z }},
            {"Scale", { m_EditorCamera.Transform.Scale.x, m_EditorCamera.Transform.Scale.y, m_EditorCamera.Transform.Scale.z }},
        };

        std::ofstream file(m_AppDataDirectory / "Blackberry-Editor" / "editor_state.blsettings");
        file << j.dump(4);
    }

    void EditorLayer::LoadEditorState() {
        if (!FS::Exists(m_AppDataDirectory / "Blackberry-Editor" / "editor_state.blsettings")) {
            std::string path = OS::OpenFile("Blackberry Project (*.blproj)");
            Project::Load(path);
            return;
        }

        std::string contents = Util::ReadEntireFile(m_AppDataDirectory / "Blackberry-Editor" / "editor_state.blsettings");
        json j = json::parse(contents);

        std::string lastProjectPath = j.at("LastProjectPath");
        auto camTransform = j.at("CameraTransform");

        m_EditorCamera.Transform.Position = BlVec3(camTransform.at("Position")[0], camTransform.at("Position")[1], camTransform.at("Position")[2]);
        // m_EditorCamera.Transform.Rotation = BlVec3(camTransform.at("Rotation")[0], camTransform.at("Rotation")[1], camTransform.at("Rotation")[2]);
        m_EditorCamera.Transform.Scale = BlVec3(camTransform.at("Scale")[0], camTransform.at("Scale")[1], camTransform.at("Scale")[2]);

        Project::Load(lastProjectPath);
    }

    void EditorLayer::SaveProject() {
        Project::Save();
        Project::SaveScene(m_EditingScene, Project::GetAssetPath(m_EditingScenePath));
    }

#pragma endregion

} // namespace BlackberryEditor