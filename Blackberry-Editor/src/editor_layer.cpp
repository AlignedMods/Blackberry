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
    
    static void DrawVec2Control(const std::string& label, BlVec2<f32>* vec, const char* fmtX = "X", const char* fmtY = "Y") {
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

    static void DrawVec3Control(const std::string& label, BlVec3<f32>* vec, const char* fmtX = "X", const char* fmtY = "Y", const char* fmtZ = "Z") {
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

        ImGui::Unindent();
    
        ImGui::PopID();
    }
    
    static void DrawRecControl(const std::string& label, BlRec* rec) {
        ImGuiIO& io = ImGui::GetIO();
    
        ImGui::PushID(label.c_str());
    
        // label
        ImGui::Text("%s", label.c_str());

        ImGui::Indent();

        ImGui::SeparatorText("Position: ");
    
        // x axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
        ImGui::Button("X", ImVec2(32.0f, 0.0f));
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        ImGui::DragFloat("##DragX", &rec->x, 1.0f);
    
        // y axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
        ImGui::Button("Y", ImVec2(32.0f, 0.0f));
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        ImGui::DragFloat("##DragY", &rec->y);

        ImGui::SeparatorText("Dimensions: ");

        // w axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
        ImGui::Button("W", ImVec2(32.0f, 0.0f));
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        ImGui::DragFloat("##DragW", &rec->w);
        
        // h axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
        ImGui::Button("H", ImVec2(32.0f, 0.0f));
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
    
        ImGui::SameLine();
        ImGui::DragFloat("##DragH", &rec->h);

        ImGui::Unindent();
    
        ImGui::PopID();
    }
    
    static void DrawColorControl(const std::string& label, BlColor* color) {
        ImGuiIO& io = ImGui::GetIO();
    
        ImVec4 imGuiColor = ImVec4(color->r / 255.0f, color->g / 255.0f, color->b / 255.0f, color->a / 255.0f);
    
        ImGui::PushID(label.c_str());

        ImGui::Text("%s", label.c_str());
        ImGui::Indent();

        ImGui::ColorEdit4("##ColorEdit", &imGuiColor.x);
    
        ImGui::Unindent();
        ImGui::PopID();
    
        // return value from ImVec4
        color->r = static_cast<u8>(imGuiColor.x * 255.0f);
        color->g = static_cast<u8>(imGuiColor.y * 255.0f);
        color->b = static_cast<u8>(imGuiColor.z * 255.0f);
        color->a = static_cast<u8>(imGuiColor.w * 255.0f);
    }

#pragma endregion
    
#pragma region OverridenFunctions

    void EditorLayer::OnAttach() {
        m_AppDataDirectory = OS::GetAppDataDirectory();
    
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF("Assets/creato_display/CreatoDisplay-Medium.otf", 18);
        io.Fonts->AddFontFromFileTTF("Assets/creato_display/CreatoDisplay-Bold.otf", 18);
    
        RenderTextureSpecification spec;
        spec.Size = BlVec2<u32>(1920, 1080);
        spec.Attachments = { {0, RenderTextureAttachmentType::ColorRGBA8},
                             {1, RenderTextureAttachmentType::Depth} };
        spec.ActiveAttachments = { 0 };

        m_RenderTexture = RenderTexture::Create(spec);
        m_MaskTexture = RenderTexture::Create(spec);
        m_OutlineTexture = RenderTexture::Create(spec);

        m_OutlineShader = Shader::Create(FS::Path("Assets/Shaders/OutlineShader.vs"), FS::Path("Assets/Shaders/OutlineShader.fs"));

        LoadEditorState();
    
        m_DirectoryIcon     = Texture2D::Create("Assets/Icons/directory.png");
        m_FileIcon          = Texture2D::Create("Assets/Icons/file.png");
        m_BackDirectoryIcon = Texture2D::Create("Assets/Icons/back_directory.png");
        m_PlayIcon          = Texture2D::Create("Assets/Icons/play.png");
        m_StopIcon          = Texture2D::Create("Assets/Icons/stop.png");
        m_PauseIcon         = Texture2D::Create("Assets/Icons/pause.png");
        m_ResumeIcon        = Texture2D::Create("Assets/Icons/resume.png");
    
        m_CurrentDirectory = Project::GetAssetDirecory();
        m_BaseDirectory = Project::GetAssetDirecory();

        m_EditorCamera.Transform.Scale = BlVec3(m_RenderTexture->Specification.Size.x, m_RenderTexture->Specification.Size.y, 1u);
        m_CurrentCamera = &m_EditorCamera;

        // gizmo styles
        ImGuizmo::Style& guizmoStyle = ImGuizmo::GetStyle();
        ImVec4* guizmoColors = guizmoStyle.Colors;

        // handles
        guizmoColors[ImGuizmo::COLOR::DIRECTION_X] = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
        guizmoColors[ImGuizmo::COLOR::DIRECTION_Y] = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
        guizmoColors[ImGuizmo::COLOR::DIRECTION_Z] = ImVec4(0.2f, 0.2f, 1.0f, 1.0f);

        guizmoColors[ImGuizmo::COLOR::SELECTION] = ImVec4(0.8f, 0.8f, 0.8f, 0.7f);

        m_EditingScene = &Project::GetStartScene().Scene;
        m_CurrentScene = m_EditingScene;

        FS::Path p("SandboxProject/");
        for (auto& file : FS::DirectoryIterator(p)) {
            BL_CORE_INFO("File name: {}", file.Path().String());
        }

        // ImGui::GetIO().IniFilename = std::filesystem::path(m_AppDataDirectory / "Blackberry-Editor" / "editor_layout.ini").string().c_str();
    }

    void EditorLayer::OnDetach() {
        Project::Save();
        SaveEditorState();
    }
    
    void EditorLayer::OnUpdate(f32 ts) {
        if (m_EditorState == EditorState::Play) {
            m_CurrentScene->OnRuntimeUpdate();
        } else {
            m_CurrentScene->OnUpdate();

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
                    f32 scale = 0.1f * Input::GetScrollLevel();
                    m_EditorCamera.Camera.Zoom = std::clamp(std::exp(std::log(m_EditorCamera.Camera.Zoom)+scale), 0.125f, 64.0f);
                }

                if (Input::IsMouseDown(MouseButton::Right)) { 
                    BlVec2 delta = Input::GetMouseDelta();
                    delta.y *= -1.0f; // invert y axis
                    
                    m_EditorCamera.Transform.Rotation += BlVec3(delta);
                    m_EditorCamera.Transform.Rotation.y = glm::clamp(m_EditorCamera.Transform.Rotation.y, -89.0f, 89.0f);

                    if (Input::IsKeyDown(KeyCode::W)) {
                        m_EditorCamera.Transform.Position += m_EditorCamera.GetForwardVector() * ts;
                    }
                    if (Input::IsKeyDown(KeyCode::S)) {
                        m_EditorCamera.Transform.Position -= m_EditorCamera.GetForwardVector() * ts;
                    }
                    if (Input::IsKeyDown(KeyCode::A)) {
                        m_EditorCamera.Transform.Position -= m_EditorCamera.GetRightVector() * ts;
                    }
                    if (Input::IsKeyDown(KeyCode::D)) {
                        m_EditorCamera.Transform.Position += m_EditorCamera.GetRightVector() * ts;
                    }
                }
            }
        }
    }
    
    void EditorLayer::OnRender() {
        m_CurrentScene->SetCamera(m_CurrentCamera);
        m_CurrentScene->OnRender(m_RenderTexture.Data());
    }

    void EditorLayer::OnUIRender() {
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
                    BL_ASSERT(0, "yes");
                    Project::Save();
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

    void EditorLayer::OnOverlayRender() {
        return;
        if (!m_IsEntitySelected) { return; }

        Entity entity = Entity(m_SelectedEntity, m_CurrentScene);

        // mask
        Renderer3D::BindRenderTexture(*m_MaskTexture);
        Renderer3D::Clear(BlColor(0, 0, 0, 255));

        Renderer3D::SetProjection(*m_CurrentCamera);

        if (entity.HasComponent<TransformComponent>()) {
            TransformComponent& transform = entity.GetComponent<TransformComponent>();
        }

        Renderer3D::Render();

        Renderer3D::ResetProjection();
        Renderer3D::UnBindRenderTexture();

        Renderer3D::BindRenderTexture(*m_OutlineTexture);
        Renderer3D::Clear(BlColor(0, 0, 0, 0));

        // outline effect
        static f32 quadVertices[] = {
            // pos      // texCoord
            -1.0f,  1.0f,  0.0f, 1.0f,   // top-left
            -1.0f, -1.0f,  0.0f, 0.0f,   // bottom-left
             1.0f, -1.0f,  1.0f, 0.0f,   // bottom-right
        
            -1.0f,  1.0f,  0.0f, 1.0f,   // top-left
             1.0f, -1.0f,  1.0f, 0.0f,   // bottom-right
             1.0f,  1.0f,  1.0f, 1.0f    // top-right
        };

        static u32 quadIndicies[] = { 0, 1, 2, 3, 4, 5 };

        DrawBuffer buffer;
        buffer.Vertices = quadVertices;
        buffer.VertexCount = 6;
        buffer.VertexSize = 4 * sizeof(f32);

        buffer.Indices = quadIndicies;
        buffer.IndexCount = 6;
        buffer.IndexSize = sizeof(u32);

        auto& renderer = BL_APP.GetRenderer();

        renderer.SubmitDrawBuffer(buffer);

        renderer.SetBufferLayout({
            { 0, ShaderDataType::Float2, "Position" },
            { 1, ShaderDataType::Float2, "TexCoord" }
        });

        renderer.BindShader(m_OutlineShader);
        // renderer.BindTexture(m_MaskTexture.Attachments[0]);

        m_OutlineShader.SetVec2("u_TexelSize", BlVec2(1.0f / m_OutlineTexture->Specification.Size.x, 1.0f / m_OutlineTexture->Specification.Size.y));
        m_OutlineShader.SetFloat("u_Thickness", 2.0f);
        m_OutlineShader.SetVec3("u_OutlineColor", BlVec3(1.0f, 0.7f, 0.2f));

        renderer.DrawIndexed(6);

        // renderer.UnBindTexture();

        Renderer3D::UnBindRenderTexture();
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

        auto playIcon = (m_EditorState == EditorState::Edit) ? m_PlayIcon : m_StopIcon;
        auto pauseIcon = (m_CurrentScene->IsPaused()) ? m_ResumeIcon : m_PauseIcon;

        f32 size = ImGui::GetWindowHeight() - 4.0f;
        f32 firstButtonPos = (ImGui::GetWindowContentRegionMax().x * 0.5f);

        if (m_EditorState == EditorState::Play) {
            firstButtonPos -= size * 0.5f;
        }

		ImGui::SetCursorPosX(firstButtonPos);
    
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.0f));

        if (ImGui::ImageButton("##PlayButton", playIcon->ID, ImVec2(size, size))) {
            if (m_EditorState == EditorState::Edit) {
                OnScenePlay();
            } else {
                OnSceneStop();
            }
        }

        ImGui::SameLine();

        if (m_EditorState == EditorState::Play) {
            if (ImGui::ImageButton("##PauseButton", pauseIcon->ID, ImVec2(size, size))) {
                m_CurrentScene->SetPaused(!m_CurrentScene->IsPaused());
            }
        }

        ImGui::PopStyleColor();
    
        ImGui::End();

        ImGui::PopStyleVar();
    }

    void EditorLayer::UI_FileBrowser() {
        ImGui::Begin("File Browser");
    
        if (m_CurrentDirectory != m_BaseDirectory) {
            if (ImGui::ImageButton("##BackDirectory", m_BackDirectoryIcon->ID, ImVec2(32.0f, 32.0f))) {
                m_CurrentDirectory = m_CurrentDirectory.ParentPath();
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

        static bool createAssetPopup = false;
        static FS::Path assetFile;

        if (ImGui::BeginTable("##FunnyTable", columnCount)) {
            for (const auto& file : FS::DirectoryIterator(m_CurrentDirectory)) {
                ImGui::TableNextColumn();

                const auto& path = file.Path();
                std::string name = path.FileName().String();
    
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    
                if (file.IsDirectory()) {
                    ImGui::ImageButton(name.c_str(), m_DirectoryIcon->ID, ImVec2(thumbnailSize, thumbnailSize));
                } else {
                    ImGui::ImageButton(name.c_str(), m_FileIcon->ID, ImVec2(thumbnailSize, thumbnailSize));
                }
    
                ImGui::PopStyleVar();
    
                if (ImGui::BeginDragDropSource()) {
                    // auto relative = fs::relative(path, m_BaseDirectory);
                    // std::string filePath = relative.string();
                    // ImGui::SetDragDropPayload("FILE_BROWSER_DRAG_DROP", filePath.c_str(), filePath.size() + 1);
                    // ImGui::Text("%s", name.c_str());
                    ImGui::EndDragDropSource();
                }
    
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    if (file.IsDirectory()) {
                        m_CurrentDirectory /= path.FileName();
                    } else {
                        if (Project::GetAssetManager().ContainsAsset(file.Path())) {
                            Asset& asset = Project::GetAssetManager().GetAssetFromPath(file.Path());
                            if (asset.Type == AssetType::Material) {
                                m_MaterialEditorPanel.SetContext(Project::GetAssetManager().GetHandleFromPath(file.Path()));
                            }
                        }
                    }
                }

                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Create Asset Out Of Item")) {
                        createAssetPopup = true;
                        assetFile = file.Path();
                    }
                    ImGui::EndPopup();
                }
    
                ImGui::TextWrapped(name.c_str());
    
                // ImGui::NextColumn();
            }
            
            ImGui::EndTable();
        }

        if (createAssetPopup) {
            ImGui::OpenPopup("CreateAssetPopup");
        }

        if (ImGui::BeginPopup("CreateAssetPopup")) {
                static const char* assetTypeNames[] = { "Texture", "Font", "Model", "Material" };
                static int currentAssetType = 0;

                ImGui::Combo("##AssetType", &currentAssetType, assetTypeNames, IM_ARRAYSIZE(assetTypeNames));

                if (ImGui::Button("Create")) {
                    // if (currentAssetType == 0) { // texture
                    //     Ref<Texture2D> tex = Texture2D::Create(assetFile);
                    //     Asset asset;
                    //     asset.Type = AssetType::Texture;
                    //     asset.FilePath = fs::relative(assetFile, m_BaseDirectory);
                    //     asset.Data = tex;
                    // 
                    //     Project::GetAssetManager().AddAsset(asset);
                    // } else if (currentAssetType == 1) { // font
                    //     Font font = Font::Create(assetFile);
                    //     Asset asset;
                    //     asset.Type = AssetType::Font;
                    //     asset.FilePath = fs::relative(assetFile, m_BaseDirectory);
                    //     asset.Data = font;
                    // 
                    //     Project::GetAssetManager().AddAsset(asset);
                    // } else if (currentAssetType == 2) { // model
                    //     Model model = Model::Create(assetFile);
                    //     Asset asset;
                    //     asset.Type = AssetType::Model;
                    //     asset.FilePath = fs::relative(assetFile, m_BaseDirectory);
                    //     asset.Data = model;
                    // 
                    //     Project::GetAssetManager().AddAsset(asset);
                    // } else if (currentAssetType == 3) { // material
                    //     Material mat = Material::Create(assetFile);
                    //     Asset asset;
                    //     asset.Type = AssetType::Material;
                    //     asset.FilePath = fs::relative(assetFile, m_BaseDirectory);
                    //     asset.Data = mat;
                    // 
                    //     Project::GetAssetManager().AddAsset(asset);
                    // }

                    ImGui::CloseCurrentPopup();
                    createAssetPopup = false;
                }

                ImGui::EndPopup();
            }

        // ImGui::Columns(1);
    
        ImGui::End();
    }

    void EditorLayer::UI_AssetManager() {
        ImGui::Begin("AssetManager");

        static f32 padding = 16.0f;
        static f32 thumbnailSize = 128.0f;
        f32 cellSize = thumbnailSize + padding;
    
        f32 panelWidth = ImGui::GetContentRegionAvail().x;
        u32 columnCount = static_cast<u32>(panelWidth / cellSize);
        if (columnCount < 1) {
            columnCount = 1;
        }
    
        if (ImGui::BeginTable("##FunnyTable", columnCount)) {
            for (const auto&[handle, asset] : Project::GetAssetManager().GetAllAssets()) {
                ImGui::TableNextColumn();

                ImGui::PushID(asset.FilePath.CString());

                // ImGui::ImageButton(asset.FilePath.string().c_str(), std::get<BlTexture>(asset.Data).ID, ImVec2(128.0f, 128.0f));
                ImGui::Button(asset.FilePath.CString(), ImVec2(128.0f, 128.0f));

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    m_MaterialEditorPanel.SetContext(handle);
                }

                if (ImGui::BeginDragDropSource()) {
                    ImGui::SetDragDropPayload("ASSET_MANAGER_DRAG_DROP", &handle, sizeof(handle));
                    ImGui::Text(asset.FilePath.CString()); // text that appears while dragging
                    ImGui::EndDragDropSource();
                }

                if (ImGui::BeginPopupContextItem("AssetManagerItemPopup")) {
                    if (ImGui::MenuItem("Copy asset handle")) {
                        std::string strHandle = std::to_string(handle);

                        ImGui::SetClipboardText(strHandle.c_str());
                    }

                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
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
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlVec3(0.0f), BlVec3(1.0f)});
                };

                if (ImGui::MenuItem("Cube")) {
                    Entity entity(m_CurrentScene->CreateEntity("Cube"), m_CurrentScene);

                    if (!Project::GetAssetManager().ContainsAsset("Meshes/Default/Cube.glb")) {
                        Model m = Model::Create(Project::GetAssetPath("Meshes/Default/Cube.glb"));

                        Project::GetAssetManager().AddAsset({"Meshes/Default/Cube.glb", AssetType::Model, m});
                    }

                    entity.AddComponent<MeshComponent>({Project::GetAssetManager().GetAssetFromPath("Meshes/Default/Cube.glb").Handle});
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlVec3(0.0f), BlVec3(1.0f)});
                };

                if (ImGui::MenuItem("Sphere")) {
                    Entity entity(m_CurrentScene->CreateEntity("Sphere"), m_CurrentScene);

                    if (!Project::GetAssetManager().ContainsAsset("Meshes/Default/Sphere.glb")) {
                        Model m = Model::Create(Project::GetAssetPath("Meshes/Default/Sphere.glb"));

                        Project::GetAssetManager().AddAsset({"Meshes/Default/Sphere.glb", AssetType::Model, m});
                    }

                    entity.AddComponent<MeshComponent>({Project::GetAssetManager().GetAssetFromPath("Meshes/Default/Sphere.glb").Handle});
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlVec3(0.0f), BlVec3(1.0f)});
                }

                if (ImGui::MenuItem("Cylinder")) {
                    Entity entity(m_CurrentScene->CreateEntity("Cylinder"), m_CurrentScene);

                    if (!Project::GetAssetManager().ContainsAsset("Meshes/Default/Cylinder.glb")) {
                        Model m = Model::Create(Project::GetAssetPath("Meshes/Default/Cylinder.glb"));

                        Project::GetAssetManager().AddAsset({"Meshes/Default/Cylinder.glb", AssetType::Model, m});
                    }

                    entity.AddComponent<MeshComponent>({Project::GetAssetManager().GetAssetFromPath("Meshes/Default/Cylinder.glb").Handle});
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlVec3(0.0f), BlVec3(1.0f)});
                }

                if (ImGui::MenuItem("Torus")) {
                    Entity entity(m_CurrentScene->CreateEntity("Torus"), m_CurrentScene);

                    if (!Project::GetAssetManager().ContainsAsset("Meshes/Default/Torus.glb")) {
                        Model m = Model::Create(Project::GetAssetPath("Meshes/Default/Torus.glb"));

                        Project::GetAssetManager().AddAsset({"Meshes/Default/Torus.glb", AssetType::Model, m});
                    }

                    entity.AddComponent<MeshComponent>({Project::GetAssetManager().GetAssetFromPath("Meshes/Default/Torus.glb").Handle});
                    entity.AddComponent<TransformComponent>({BlVec3(0.0f), BlVec3(0.0f), BlVec3(1.0f)});
                }
                
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Camera")) {
                if (ImGui::MenuItem("From view")) {
                    Entity entity(m_CurrentScene->CreateEntity("Camera"), m_CurrentScene);

                    entity.AddComponent<TransformComponent>(m_EditorCamera.Transform);
                    entity.AddComponent<CameraComponent>(m_EditorCamera.Camera);
                }

                if (ImGui::MenuItem("Default camera")) {
                
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
                    AddComponentListOption<ColliderComponent>("Collider", entity);
                    AddComponentListOption<DirectionalLightComponent>("Directional Light", entity);
                    AddComponentListOption<PointLightComponent>("Point Light", entity);
                    
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

                DrawVec3Control("Rotation: ", &transform.Rotation);
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
                        ImGui::Text("[%u]: ", i); ImGui::SameLine();

                        std::string matName;
                        if (Project::GetAssetManager().ContainsAsset(model.Meshes[i].MaterialHandle)) {
                            matName = Project::GetAssetManager().GetAsset(model.Meshes[i].MaterialHandle).FilePath.Stem().String();
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
                            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_HANDLE_DRAG_DROP");

                            if (payload) {
                                model.Meshes[i].MaterialHandle = *reinterpret_cast<u64*>(payload->Data);
                            }
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

                ImGui::Text("Mass: ");
                ImGui::Indent();
                ImGui::DragFloat("##Mass", &rigidBody.Mass);
                ImGui::Unindent();

                ImGui::Text("Enable Gravity: ");
                ImGui::Indent();
                ImGui::Checkbox("##EnableGravity", &rigidBody.EnableGravity);
                ImGui::Unindent();
            });
            DrawComponent<ColliderComponent>("Collider", entity, [](ColliderComponent& collider) {
                
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
        }
    
        ImGui::End();
    }
    
    void EditorLayer::UI_Viewport() {
        ImGuizmo::BeginFrame();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImGui::PopStyleVar();

        ImVec2 windowArea = ImGui::GetContentRegionAvail();
        f32 scale = windowArea.x / static_cast<f32>(m_RenderTexture->Specification.Size.x);
        f32 y = m_RenderTexture->Specification.Size.x * scale;

        // we want to make the viewport the smallest axis
        if (y > windowArea.y) {
            scale = windowArea.y / static_cast<f32>(m_RenderTexture->Specification.Size.y);
        }
        
        BlVec2 size = BlVec2(m_RenderTexture->Specification.Size.x * scale, m_RenderTexture->Specification.Size.y * scale);

        f32 cursorX = ImGui::GetCursorPosX() + windowArea.x / 2.0f - size.x / 2.0f;
        f32 cursorY = ImGui::GetCursorPosY() + (windowArea.y / 2.0f - size.y / 2.0f);

        ImGui::SetCursorPosX(cursorX);
        ImGui::SetCursorPosY(cursorY);
        auto& rendererState = m_CurrentScene->GetSceneRenderer()->GetState();
        ImGui::Image(m_RenderTexture->Attachments[0]->ID, ImVec2(size.x, size.y), ImVec2(0, 1), ImVec2(1, 0));

        ImGui::SetCursorPosX(cursorX);
        ImGui::SetCursorPosY(cursorY);
        // ImGui::Image(m_OutlineTexture.Attachments[0].ID, ImVec2(size.x, size.y), ImVec2(0, 1), ImVec2(1, 0));

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
            if (auto payload = ImGui::AcceptDragDropPayload("FILE_BROWSER_DRAG_DROP")) {
                bool sceneExists = false;
    
                std::string strPath = reinterpret_cast<char*>(payload->Data);
                FS::Path path(strPath);
                path = m_BaseDirectory / path;
                
                for (auto& scene : Project::GetScenes()) {
                    if (scene.Path == path) {
                        m_EditingScene = &scene.Scene;
                        sceneExists = true;
                    }
                }
    
                if (!sceneExists) {
                    m_EditingScene = &Project::LoadScene(path);
                }

                m_CurrentScene = m_EditingScene;
            }
            ImGui::EndDragDropTarget();
        };

        // gizmos
        if (m_IsEntitySelected && m_GizmoState != GizmoState::None) {
            Entity e(m_SelectedEntity, m_CurrentScene);
            if (e.HasComponent<TransformComponent>()) {
                TransformComponent& transform = e.GetComponent<TransformComponent>();
                glm::mat4 transformMatrix = transform.GetMatrix();

                glm::mat4 camProjection = m_CurrentCamera->GetCameraProjection();
                glm::mat4 camView = m_CurrentCamera->GetCameraView(); // already inversed

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
                    snapValue = 45.0f;
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
                    transform.Rotation = BlVec3(rot[0], rot[1], rot[2]);
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

        // create new camera
        m_RuntimeCamera = m_RuntimeScene->GetSceneCamera();
        m_CurrentCamera = &m_RuntimeCamera;

        m_CurrentScene->OnPlay();
    }

    void EditorLayer::OnSceneStop() {
        m_CurrentScene->OnStop();

        BL_INFO("Reverted to editing scene.");
        m_EditorState = EditorState::Edit;
        m_CurrentScene = m_EditingScene;
        delete m_RuntimeScene;
        m_RuntimeScene = nullptr;
        m_CurrentCamera = &m_EditorCamera;
    }

    void EditorLayer::OnScenePause() {
    }

    void EditorLayer::SaveEditorState() {
        if (!FS::Exists(m_AppDataDirectory / "Blackberry-Editor")) {
            // std::filesystem::create_directory(m_AppDataDirectory / "Blackberry-Editor");
        }

        json j;
        j["LastProjectPath"] = Project::GetProjectPath();

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
        Project::Load(lastProjectPath);
    }

#pragma endregion

} // namespace BlackberryEditor