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
    
        m_RenderTexture = RenderTexture::Create(1920, 1080);
        m_MaskTexture = RenderTexture::Create(1920, 1080);
        m_OutlineTexture = RenderTexture::Create(1920, 1080);

        std::string vs = ReadEntireFile("Assets/shaders/OutlineShader.vs");
        std::string fs = ReadEntireFile("Assets/shaders/OutlineShader.fs");
        m_OutlineShader.Create(vs, fs);

        LoadEditorState();
    
        m_DirectoryIcon     = Texture2D::Create("Assets/icons/directory.png");
        m_FileIcon          = Texture2D::Create("Assets/icons/file.png");
        m_BackDirectoryIcon = Texture2D::Create("Assets/icons/back_directory.png");
        m_PlayIcon          = Texture2D::Create("Assets/icons/play.png");
        m_StopIcon          = Texture2D::Create("Assets/icons/stop.png");
        m_PauseIcon         = Texture2D::Create("Assets/icons/pause.png");
    
        m_CurrentDirectory = Project::GetAssetDirecory();
        m_BaseDirectory = Project::GetAssetDirecory();

        m_EditorCamera.Offset = BlVec2<f32>(m_RenderTexture.Size.x / 2.0f, m_RenderTexture.Size.y / 2.0f);
        m_EditorCamera.Position = BlVec2<f32>(m_RenderTexture.Size.x / 2.0f, m_RenderTexture.Size.y / 2.0f);
        m_EditorCamera.Type = CameraType::Orthographic;
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

        // ImGui::GetIO().IniFilename = std::filesystem::path(m_AppDataDirectory / "Blackberry-Editor" / "editor_layout.ini").string().c_str();
    }

    void EditorLayer::OnDetach() {
        Project::Save();
        SaveEditorState();

        m_RenderTexture.Delete();
    
        m_DirectoryIcon.Delete();
        m_FileIcon.Delete();
        m_BackDirectoryIcon.Delete();
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
                if (Input::GetScrollLevel() != 0.0f) {
                    // BlVec2 screenPos = Input::GetMousePosition() - BlVec2(m_ViewportBounds.x, m_ViewportBounds.y);
                    // screenPos *= BlVec2(m_EditorCamera.Size.x / m_ViewportBounds.w, m_EditorCamera.Size.y / m_ViewportBounds.h);
                    // BlVec2 worldPos = m_EditorCamera.GetScreenPosToWorld(screenPos);
                    // 
                    // BL_INFO("Screen pos: {}, {}", screenPos.x, screenPos.y);
                    // 
                    // m_EditorCamera.Offset = BlVec2(1920.0f / 2.0f, 1080.0f / 2.0f);
                
                    f32 scale = 0.1f * Input::GetScrollLevel();
                    m_EditorCamera.Zoom = std::clamp(std::exp(std::log(m_EditorCamera.Zoom)+scale), 0.125f, 64.0f);
                }

                if (Input::IsMouseDown(MouseButton::Right)) { 
                    BlVec2 delta = Input::GetMouseDelta();
                    delta.x *= -1.0f;

                    m_EditorCamera.Position -= delta * BlVec2(m_EditorCamera.Zoom);
                }
            }
        }
    }
    
    void EditorLayer::OnRender() {
        Renderer2D::AttachRenderTexture(m_RenderTexture);
        Renderer2D::Clear(BlColor(0x69, 0x69, 0x69, 0xff));

        if (m_EditorState == EditorState::Edit) {  
            m_CurrentScene->SetCamera(&m_EditorCamera);
        } else {
            m_CurrentScene->SetCamera(&m_RuntimeCamera);
        }

        m_CurrentScene->OnRender();

        Renderer2D::DetachRenderTexture();
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
        UI_AssetManager();
        UI_Explorer();
        UI_Properties();
        UI_Viewport();
        UI_RendererStats();
    
        if (m_ShowDemoWindow) {
            ImGui::ShowDemoWindow(&m_ShowDemoWindow);
        }
    }

    void EditorLayer::OnOverlayRender() {
        if (!m_IsEntitySelected) { return; }

        Entity entity = Entity(m_SelectedEntity, m_CurrentScene);

        // mask
        Renderer2D::AttachRenderTexture(m_MaskTexture);
        Renderer2D::Clear(BlColor(0, 0, 0, 255));

        Renderer2D::SetProjection(*m_CurrentCamera);

        if (entity.HasComponent<Transform2DComponent>()) {
            Transform2DComponent& transform = entity.GetComponent<Transform2DComponent>();

            auto drawWhiteEntity = [&]<typename T>() -> bool {
                if (!entity.HasComponent<T>()) { return false; }

                T& ren = entity.GetComponent<T>();

                switch (ren.Shape) {
                    case ShapeType::Triangle:
                        Renderer2D::DrawTriangle(transform.GetMatrix(), Colors::White);
                        break;
                    case ShapeType::Rectangle:
                        Renderer2D::DrawRectangle(transform.GetMatrix(), Colors::White);
                        break;
                    case ShapeType::Circle:
                        Renderer2D::DrawElipse(transform.GetMatrix(), Colors::White);
                        break;
                }
            };

            // wierd syntax honestly
            if (!drawWhiteEntity.template operator()<SpriteRendererComponent>() && !drawWhiteEntity.template operator()<ShapeRendererComponent>()) {
                Renderer2D::DrawRectangle(transform.GetMatrix(), Colors::White);
            };

        }

        Renderer2D::Render();

        Renderer2D::ResetProjection();
        Renderer2D::DetachRenderTexture();

        Renderer2D::AttachRenderTexture(m_OutlineTexture);
        Renderer2D::Clear(BlColor(0, 0, 0, 0));

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

        BlDrawBufferLayout vertPosLayout;
        vertPosLayout.Index = 0;
        vertPosLayout.Count = 2;
        vertPosLayout.Type = BlDrawBufferLayout::ElementType::Float;
        vertPosLayout.Stride = 4 * sizeof(f32);
        vertPosLayout.Offset = 0;

        BlDrawBufferLayout vertTexCoordLayout;
        vertTexCoordLayout.Index = 1;
        vertTexCoordLayout.Count = 2;
        vertTexCoordLayout.Type = BlDrawBufferLayout::ElementType::Float;
        vertTexCoordLayout.Stride = 4 * sizeof(f32);
        vertTexCoordLayout.Offset = 2 * sizeof(f32);

        BlDrawBuffer buffer;
        buffer.Vertices = quadVertices;
        buffer.VertexCount = 6;
        buffer.VertexSize = 4 * sizeof(f32);

        buffer.Indices = quadIndicies;
        buffer.IndexCount = 6;
        buffer.IndexSize = sizeof(u32);

        auto& renderer = BL_APP.GetRenderer();

        renderer.SubmitDrawBuffer(buffer);

        renderer.SetBufferLayout(vertPosLayout);
        renderer.SetBufferLayout(vertTexCoordLayout);

        renderer.BindShader(m_OutlineShader);
        renderer.AttachTexture(m_MaskTexture.ColorAttachment);

        m_OutlineShader.SetVec2("u_TexelSize", BlVec2(1.0f / m_OutlineTexture.Size.x, 1.0f / m_OutlineTexture.Size.y));
        m_OutlineShader.SetFloat("u_Thickness", 2.0f);
        m_OutlineShader.SetVec3("u_OutlineColor", BlVec3(1.0f, 0.7f, 0.2f));

        renderer.DrawIndexed(6);

        renderer.DetachTexture();

        Renderer2D::DetachRenderTexture();
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
        auto pauseIcon = m_PauseIcon;

        f32 size = ImGui::GetWindowHeight() - 4.0f;
        f32 firstButtonPos = (ImGui::GetWindowContentRegionMax().x * 0.5f);

        if (m_EditorState == EditorState::Play) {
            firstButtonPos -= size * 0.5f;
        }

		ImGui::SetCursorPosX(firstButtonPos);
    
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.0f));

        if (ImGui::ImageButton("##PlayButton", playIcon.ID, ImVec2(size, size))) {
            if (m_EditorState == EditorState::Edit) {
                OnScenePlay();
            } else {
                OnSceneStop();
            }
        }

        ImGui::SameLine();

        if (m_EditorState == EditorState::Play) {
            if (ImGui::ImageButton("##PauseButton", pauseIcon.ID, ImVec2(size, size))) {
                // OnScenePause();
            }
        }

        ImGui::PopStyleColor();
    
        ImGui::End();

        ImGui::PopStyleVar();
    }

    void EditorLayer::UI_FileBrowser() {
        namespace fs = std::filesystem;
    
        ImGui::Begin("File Browser");
    
        if (m_CurrentDirectory != m_BaseDirectory) {
            if (ImGui::ImageButton("##BackDirectory", m_BackDirectoryIcon.ID, ImVec2(32.0f, 32.0f))) {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
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
        static fs::path assetFile;

        if (ImGui::BeginTable("##FunnyTable", columnCount)) {
            for (const auto& file : fs::directory_iterator(m_CurrentDirectory)) {
                ImGui::TableNextColumn();

                const auto& path = file.path();
                std::string name = path.filename().string();
    
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    
                if (file.is_directory()) {
                    ImGui::ImageButton(name.c_str(), m_DirectoryIcon.ID, ImVec2(thumbnailSize, thumbnailSize));
                } else {
                    ImGui::ImageButton(name.c_str(), m_FileIcon.ID, ImVec2(thumbnailSize, thumbnailSize));
                }
    
                ImGui::PopStyleVar();
    
                if (ImGui::BeginDragDropSource()) {
                    auto relative = fs::relative(path, m_BaseDirectory);
                    std::string filePath = relative.string();
                    ImGui::SetDragDropPayload("FILE_BROWSER_DRAG_DROP", filePath.c_str(), filePath.size() + 1);
                    ImGui::Text("%s", name.c_str());
                    ImGui::EndDragDropSource();
                }
    
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    if (file.is_directory()) {
                        m_CurrentDirectory /= path.filename();
                    }
                }

                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Create asset handle")) {
                        createAssetPopup = true;
                        assetFile = file;
                        ImGui::OpenPopup("CreateAssetPopup");
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
                static bool s_IsTexture = false;
                ImGui::Checkbox("Is texture?", &s_IsTexture);

                if (ImGui::Button("Create")) {
                    if (s_IsTexture) {
                        Texture2D tex = Texture2D::Create(assetFile);
                        Asset asset;
                        asset.Type = AssetType::Texture;
                        asset.FilePath = fs::relative(assetFile, m_BaseDirectory);
                        asset.Data = tex;

                        m_CurrentScene->GetAssetManager().AddAsset("cart", asset);
                    } else {
                        Font font = Font::Create(assetFile);
                        Asset asset;
                        asset.Type = AssetType::Font;
                        asset.FilePath = fs::relative(assetFile, m_BaseDirectory);
                        asset.Data = font;

                        m_CurrentScene->GetAssetManager().AddAsset("cart", asset);
                    }

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
            for (const auto&[handle, asset] : m_EditingScene->GetAssetManager().GetAllAssets()) {
                ImGui::TableNextColumn();

                ImGui::PushID(asset.FilePath.string().c_str());

                // ImGui::ImageButton(asset.FilePath.string().c_str(), std::get<BlTexture>(asset.Data).ID, ImVec2(128.0f, 128.0f));
                ImGui::Button(asset.FilePath.string().c_str(), ImVec2(128.0f, 128.0f));

                if (ImGui::BeginDragDropSource()) {
                    ImGui::SetDragDropPayload("ASSET_MANAGER_DRAG_DROP", &handle, sizeof(handle));
                    ImGui::Text(asset.FilePath.string().c_str()); // text that appears while dragging
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

            if (ImGui::BeginMenu("Add")) {
                if (ImGui::MenuItem("Rectangle")) {
                    Entity entity(m_CurrentScene->CreateEntity("Rectangle"), m_CurrentScene);

                    entity.AddComponent<ShapeRendererComponent>();
                    entity.AddComponent<Transform2DComponent>({BlVec3<f32>(m_RenderTexture.Size.x / 2.0f - 100.0f, m_RenderTexture.Size.y / 2.0f - 50.0f, 0.0f), 0.0f, BlVec2<f32>(200.0f, 100.0f)});
                };

                if (ImGui::MenuItem("Triangle")) {
                    Entity entity(m_CurrentScene->CreateEntity("Triangle"), m_CurrentScene);
                    entity.AddComponent<ShapeRendererComponent>({.Shape = ShapeType::Triangle});
                    entity.AddComponent<Transform2DComponent>({BlVec3<f32>(m_RenderTexture.Size.x / 2.0f - 100.0f, m_RenderTexture.Size.y / 2.0f - 50.0f, 0.0f), 0.0f, BlVec2<f32>(200.0f, 100.0f)});
                };
                
                
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
                    AddComponentListOption<Transform2DComponent>("Transform2D", entity);
                    AddComponentListOption<TextComponent>("Text", entity);
                    AddComponentListOption<ShapeRendererComponent>("Shape Renderer", entity);
                    AddComponentListOption<SpriteRendererComponent>("Sprite Renderer", entity);
                    AddComponentListOption<CameraComponent>("Camera", entity);
                    AddComponentListOption<ScriptComponent>("Script", entity);
                    AddComponentListOption<RigidBodyComponent>("Rigid Body", entity);
                    
                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }
    
            TagComponent& tag = entity.GetComponent<TagComponent>();

            ImGui::Text("Name: "); ImGui::SameLine();
            ImGui::InputText("##EntityName", &tag.Name);
            ImGui::TextDisabled("UUID: %llu", tag.UUID);

            ImGui::SeparatorText("Components: ");

            DrawComponent<Transform2DComponent>("Transform2D", entity, [](Transform2DComponent& transform) {
                DrawVec3Control("Position: ", &transform.Position);
                ImGui::Separator();

                ImGui::Text("Rotation: ");
                ImGui::Indent();
                ImGui::DragFloat("##Rotation", &transform.Rotation);
                ImGui::Unindent();
                ImGui::Separator();

                DrawVec2Control("Dimensions: ", &transform.Dimensions);
            });
            DrawComponent<ShapeRendererComponent>("Shape Renderer", entity, [](ShapeRendererComponent& shapeRenderer) {
                DrawColorControl("Color: ", &shapeRenderer.Color);
                ImGui::Separator();

                ImGuiIO& io = ImGui::GetIO();

                static const char* shapeNames[] = { "Triangle", "Rectangle", "Circle", "Polygon" };
                int currentShape = static_cast<int>(shapeRenderer.Shape);

                ImGui::Text("Shape: ");
                ImGui::Indent();

                if (ImGui::Combo("##Shape Type", &currentShape, shapeNames, IM_ARRAYSIZE(shapeNames))) {
                    shapeRenderer.Shape = static_cast<ShapeType>(currentShape);
                }

                ImGui::Unindent();
            });
            DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [this](SpriteRendererComponent& spriteRenderer) {
                DrawColorControl("Color: ", &spriteRenderer.Color);
                ImGui::Separator();

                ImGuiIO& io = ImGui::GetIO();

                static const char* shapeNames[] = { "Triangle", "Rectangle", "Circle", "Polygon" };
                int currentShape = static_cast<int>(spriteRenderer.Shape);

                ImGui::Text("Shape: ");
                ImGui::Indent();
    
                if (ImGui::Combo("##Shape Type", &currentShape, shapeNames, IM_ARRAYSIZE(shapeNames))) {
                    spriteRenderer.Shape = static_cast<ShapeType>(currentShape);
                }

                ImGui::Unindent();
                ImGui::Separator();
                ImGui::Text("Texture: ");
                ImGui::Indent();

                std::string mat;
                if (m_CurrentScene->GetAssetManager().ContainsAsset(spriteRenderer.TextureHandle)) {
                    mat = m_CurrentScene->GetAssetManager().GetAsset(spriteRenderer.TextureHandle).FilePath.stem().string();
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

                if (ImGui::BeginPopupContextItem("TextureHandlePopup")) {
                    if (ImGui::MenuItem("Remove Texture")) {
                        spriteRenderer.TextureHandle = 0;
                    }

                    ImGui::EndPopup();
                }

                if (ImGui::BeginDragDropTarget()) {
                    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_DRAG_DROP");

                    if (payload) {
                        spriteRenderer.TextureHandle = *reinterpret_cast<u64*>(payload->Data); // seems sus
                    }
                }

                ImGui::Unindent();

                DrawRecControl("Area", &spriteRenderer.Area);
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
                if (m_CurrentScene->GetAssetManager().ContainsAsset(text.FontHandle)) {
                    mat = m_CurrentScene->GetAssetManager().GetAsset(text.FontHandle).FilePath.stem().string();
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
                    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_DRAG_DROP");

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
                SceneCamera& cam = camera.Camera;

                DrawVec2Control("Position: ", &cam.Position);
                ImGui::Separator();

                DrawVec2Control("Offset: ", &cam.Offset);
                ImGui::Separator();

                DrawVec2Control("Size: ", &cam.Size);
                ImGui::Separator();

                ImGui::Text("Rotation: ");
                ImGui::Indent();
                ImGui::DragFloat("##Rotation", &cam.Rotation);
                ImGui::Unindent();
                ImGui::Separator();

                ImGui::Text("Zoom: ");
                ImGui::Indent();
                ImGui::DragFloat("##Zoom", &cam.Zoom);
                ImGui::Unindent();
                ImGui::Separator();

                ImGui::Text("Near: ");
                ImGui::Indent();
                ImGui::DragFloat("##CameraNear", &cam.Near);
                ImGui::Unindent();
                ImGui::Separator();

                ImGui::Text("Far: ");
                ImGui::Indent();
                ImGui::DragFloat("##CameraFar", &cam.Far);
                ImGui::Unindent();
            });
            DrawComponent<ScriptComponent>("Script", entity, [this](ScriptComponent& script) {
                auto stringPath = script.ModulePath.string();

                ImGui::Text("Module Path: "); ImGui::SameLine();
                ImGui::InputText("##ModulePath", &stringPath);

                script.ModulePath = std::filesystem::path(stringPath);
                script.FilePath = m_BaseDirectory / script.ModulePath;
            });
            DrawComponent<RigidBodyComponent>("Rigid Body", entity, [](RigidBodyComponent& rigidBody) {
                DrawVec2Control("Velocity: ", &rigidBody.Velocity);
                DrawVec2Control("Acceleration: ", &rigidBody.Acceleration);
                DrawVec2Control("Force: ", &rigidBody.Force);

                ImGui::DragFloat("Mass", &rigidBody.Mass);
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
        f32 scale = windowArea.x / static_cast<f32>(m_RenderTexture.Size.x);
        f32 y = m_RenderTexture.Size.x * scale;

        // we want to make the viewport the smallest axis
        if (y > windowArea.y) {
            scale = windowArea.y / static_cast<f32>(m_RenderTexture.Size.y);
        }
        
        BlVec2 size = BlVec2(m_RenderTexture.Size.x * scale, m_RenderTexture.Size.y * scale);

        f32 cursorX = ImGui::GetCursorPosX() + windowArea.x / 2.0f - size.x / 2.0f;
        f32 cursorY = ImGui::GetCursorPosY() + (windowArea.y / 2.0f - size.y / 2.0f);

        ImGui::SetCursorPosX(cursorX);
        ImGui::SetCursorPosY(cursorY);
        ImGui::Image(m_RenderTexture.ColorAttachment.ID, ImVec2(size.x, size.y), ImVec2(0, 1), ImVec2(1, 0));

        ImGui::SetCursorPosX(cursorX);
        ImGui::SetCursorPosY(cursorY);
        ImGui::Image(m_OutlineTexture.ColorAttachment.ID, ImVec2(size.x, size.y), ImVec2(0, 1), ImVec2(1, 0));

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
                std::filesystem::path path(strPath);
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
            if (e.HasComponent<Transform2DComponent>()) {
                Transform2DComponent& transform = e.GetComponent<Transform2DComponent>();
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

                bool snap = Input::IsKeyDown(KeyCode::Ctrl);
                f32 snapValue = 40.0f;

                if (m_GizmoState == GizmoState::Rotate) {
                    snapValue = 45.0f;
                }

                f32 snapValues[3] = { snapValue, snapValue, snapValue };

                ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
                ImGuizmo::Enable(true);
                ImGuizmo::SetOrthographic(true);
                ImGuizmo::SetRect(min.x, min.y, max.x - min.x, max.y - min.y);
                ImGuizmo::Manipulate(glm::value_ptr(camView), glm::value_ptr(camProjection), operation, ImGuizmo::MODE::LOCAL, glm::value_ptr(transformMatrix), nullptr, snap ? snapValues : nullptr);

                if (ImGuizmo::IsUsing()) {
                    f32 pos[3], rot[3], scale[3];
                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transformMatrix), pos, rot, scale);

                    transform.Position = BlVec3(pos[0], pos[1], pos[2]);
                    transform.Rotation = rot[2];
                    transform.Dimensions = BlVec2(scale[0], scale[1]);
                }
            }
        }
    
        ImGui::End();
    }

    void EditorLayer::UI_RendererStats() {
        ImGui::Begin("Renderer Stats");

        auto stats = Renderer2D::GetRenderingInfo();

        ImGui::Text("Draw Calls: %u", stats.DrawCalls);
        ImGui::Text("Verticies: %u", stats.Vertices);
        ImGui::Text("Indicies: %u", stats.Indicies);
        ImGui::Text("Active Textures: %u", stats.ActiveTextures);
        ImGui::Text("Reserved Textures: %u", stats.ReservedTextures);

        ImGui::End();

        ImGui::Begin("Font");

        ImGui::Image(m_EditorFont.TextureAtlas.ID, ImVec2(m_EditorFont.TextureAtlas.Size.x, m_EditorFont.TextureAtlas.Size.y));

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
        m_RuntimeScene = Scene::Copy(m_EditingScene);
        m_CurrentScene = m_RuntimeScene;

        for (auto entityID : m_CurrentScene->GetEntities()) {
            Entity entity(entityID, m_CurrentScene);
            
            if (entity.HasComponent<CameraComponent>()) {
                m_RuntimeCamera = entity.GetComponent<CameraComponent>().Camera;
            }
        }
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
        if (!std::filesystem::exists(m_AppDataDirectory / "Blackberry-Editor")) {
            std::filesystem::create_directory(m_AppDataDirectory / "Blackberry-Editor");
        }

        json j;
        j["LastProjectPath"] = Project::GetProjectPath();

        std::ofstream file(m_AppDataDirectory / "Blackberry-Editor" / "editor_state.blsettings");
        file << j.dump(4);
    }

    void EditorLayer::LoadEditorState() {
        if (!std::filesystem::exists(m_AppDataDirectory / "Blackberry-Editor" / "editor_state.blsettings")) {
            std::string path = OS::OpenFile("Blackberry Project (*.blproj)");
            Project::Load(path);
            return;
        }

        std::string contents = ReadEntireFile(m_AppDataDirectory / "Blackberry-Editor" / "editor_state.blsettings");
        json j = json::parse(contents);

        std::string lastProjectPath = j.at("LastProjectPath");
        Project::Load(lastProjectPath);
    }

#pragma endregion

} // namespace BlackberryEditor