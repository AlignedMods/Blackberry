#include "editor_layer.hpp"
#include "sdf_generation.hpp"

#include <fstream>

namespace BlackberryEditor {

    static char s_Buffer[512];

    static const char* s_VertexOutlineShaderSource = BL_STR(
        \x23version 460 core\n
        layout(location = 0) in vec2 a_Pos;
        layout(location = 1) in vec2 a_TexCoord;
        
        out vec2 texCoord;

        uniform mat4 u_Projection; // gets set automatically through the renderer
        
        void main() {
            texCoord = a_TexCoord;
            gl_Position = u_Projection * vec4(a_Pos, 0.0, 1.0);
        }
    );

    static const char* s_FragmentOutlineShaderSource = BL_STR(
        \x23version 460 core\n

        in vec2 texCoord;
        
        uniform sampler2D u_Mask;       // mask of selected objects
        uniform vec2 u_TexelSize;       // (1.0 / textureWidth, 1.0 / textureHeight)
        uniform vec3 u_OutlineColor;    // RGB color of outline
        uniform float u_OutlineThickness; // thickness in pixels (e.g., 1.0–4.0)
        uniform float u_OutlineStrength;  // how visible (0.0–1.0)
        
        out vec4 fragColor;
        
        void main() {
            float center = texture(u_Mask, texCoord).r;
            float outline = 0.0;
            if (center < 0.5) {
                for (int y = -4; y <= 4; ++y) {
                    for (int x = -4; x <= 4; ++x) {
                        vec2 offset = vec2(x, y);
                        if (length(offset) > 5) continue; // circular radius
                        float neighbor = texture(u_Mask, texCoord + offset * u_TexelSize).r;
                        if (neighbor > 0.5) {
                            outline = 1.0;
                            break;
                        }
                    }
                    if (outline > 0.0) break;
                }
            }

            fragColor = (outline > 0.0f) ? vec4(u_OutlineColor, 1.0f) : vec4(0.0f, 0.0f, 0.0f, 0.0f);
        }
    );

#pragma region HelperFunctions
    
    template <typename T, typename F>
    static void DrawComponent(const std::string& name, Blackberry::Entity entity, F uiFunction) {
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
    static void AddComponentListOption(const std::string& name, Blackberry::Entity& entity, const T& component = T{}) {
        if (entity.HasComponent<T>()) { return; }
    
        if (ImGui::Button(name.c_str())) {
            entity.AddComponent<T>(component);
            ImGui::CloseCurrentPopup();
        }
    }
    
    static void DrawVec2Control(const std::string& label, BlVec2* vec, const char* fmtX = "X", const char* fmtY = "Y") {
        ImGuiIO& io = ImGui::GetIO();
    
        ImGui::PushID(label.c_str());
    
        // label
    
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        if (ImGui::TreeNode(label.c_str())) {
            ImGui::PopFont();
    
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
    
            ImGui::TreePop();
        } else {
            ImGui::PopFont();
        }
    
        ImGui::PopID();
    }

    static void DrawVec3Control(const std::string& label, BlVec3* vec, const char* fmtX = "X", const char* fmtY = "Y", const char* fmtZ = "Z") {
        ImGuiIO& io = ImGui::GetIO();
    
        ImGui::PushID(label.c_str());
    
        // label
    
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        if (ImGui::TreeNode(label.c_str())) {
            ImGui::PopFont();
    
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
    
            ImGui::TreePop();
        } else {
            ImGui::PopFont();
        }
    
        ImGui::PopID();
    }
    
    static void DrawRecControl(const std::string& label, BlRec* rec) {
        BlVec2 xy = BlVec2(rec->x, rec->y);
        BlVec2 wh = BlVec2(rec->w, rec->h);
    
        ImGui::PushID(label.c_str());
    
        if (ImGui::TreeNode(label.c_str())) {
            DrawVec2Control("Position: ", &xy);
            DrawVec2Control("Dimensions: ", &wh);
    
            ImGui::TreePop();
        }
    
        ImGui::PopID();
    
        // return value
        rec->x = xy.x;
        rec->y = xy.y;
        rec->w = wh.x;
        rec->h = wh.y;
    }
    
    static void DrawColorControl(const std::string& label, BlColor* color) {
        ImGuiIO& io = ImGui::GetIO();
    
        ImVec4 imGuiColor = ImVec4(color->r / 255.0f, color->g / 255.0f, color->b / 255.0f, color->a / 255.0f);
    
        ImGui::PushID(label.c_str());
    
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        if (ImGui::TreeNode(label.c_str())) {
            ImGui::PopFont();
    
            ImGui::ColorEdit4("##ColorEdit", &imGuiColor.x);
        
            ImGui::TreePop();
        } else {
            ImGui::PopFont();
        }
    
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
        m_AppDataDirectory = Blackberry::OS::GetAppDataDirectory();
    
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF("Assets/creato_display/CreatoDisplay-Medium.otf", 18);
        io.Fonts->AddFontFromFileTTF("Assets/creato_display/CreatoDisplay-Bold.otf", 18);
    
        m_RenderTexture.Create(1280, 720);
        m_OutlineShader.Create(s_VertexOutlineShaderSource, s_FragmentOutlineShaderSource);
        m_MaskTexture.Create(1280, 720);

        m_OutlineTexture.Create(1280, 720);

        LoadEditorState();
    
        m_DirectoryIcon.Create("Assets/icons/directory.png");
        m_FileIcon.Create("Assets/icons/file.png");
        m_BackDirectoryIcon.Create("Assets/icons/back_directory.png");
        m_PlayIcon.Create("Assets/icons/play.png");
        m_StopIcon.Create("Assets/icons/stop.png");
        m_PauseIcon.Create("Assets/icons/pause.png");
    
        m_CurrentDirectory = m_CurrentProject.AssetDirectory;
        m_BaseDirectory = m_CurrentProject.AssetDirectory;
    }

    void EditorLayer::OnDetach() {
        SaveProject();
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
        }
    }
    
    void EditorLayer::OnRender() {
        using namespace Blackberry::Components;

        Blackberry::Renderer2D::AttachRenderTexture(m_RenderTexture);
        Blackberry::Renderer2D::Clear(BlColor(0x69, 0x69, 0x69, 0xff));

        m_CurrentScene->OnRender();

        Blackberry::Renderer2D::DetachRenderTexture();

#if 0

        // overlay (quite a bit of stuff here!)
        Blackberry::Renderer2D::AttachRenderTexture(m_MaskTexture);
        Blackberry::Renderer2D::Clear(BlColor(0, 0, 0, 255));

        if (m_IsEntitySelected) {
            Blackberry::Entity e(m_SelectedEntity, m_CurrentScene);

            if (e.HasComponent<Transform>() && e.HasComponent<Drawable>()) {
                auto& transform = e.GetComponent<Transform>();
                auto& drawable = e.GetComponent<Drawable>();

                BlColor color = BlColor(255, 255, 255, 255);

                switch (drawable.ShapeType) {
                    case Shape::Triangle:
                        Blackberry::Renderer2D::DrawTriangle(transform.Position, transform.Dimensions, transform.Rotation, color);
                        break;
                    case Shape::Rectangle:
                        Blackberry::Renderer2D::DrawRectangle(transform.Position, transform.Dimensions, transform.Rotation, color);
                        break;
                }
            }
        }

        Blackberry::Renderer2D::Render();

        Blackberry::Renderer2D::DetachRenderTexture();

#endif
    }
    

    void EditorLayer::OnUIRender() {
        using namespace Blackberry::Components;
    
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
    
                    ImGui::EndMenu();
                }
    
                if (ImGui::MenuItem("Save Project", "CTRL+S")) {
                    BL_ASSERT(0, "yes");
                    SaveProject();
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
        UI_AssetManager();
        UI_Explorer();
        UI_Properties();
        UI_Viewport();
        UI_RendererStats();
    
        if (m_ShowDemoWindow) {
            ImGui::ShowDemoWindow(&m_ShowDemoWindow);
        }
    }

    void BlackberryEditor::EditorLayer::OnOverlayRender() {
#if 0

        // render a quad the size of the viewport which will be the outline (we must do this fully manually though)
        f32 vertices[] = {
            0.0f, m_ViewportBounds.h, 0.0f, 0.0f, // bl
            m_ViewportBounds.w, 0.0f, 1.0f, 1.0f, // tr
            m_ViewportBounds.w, m_ViewportBounds.h, 1.0f, 0.0f, // br
            0.0f, 0.0f, 0.0f, 1.0f, // tl
        };

        // BlTextureVertex vertexBL = BlTextureVertex(bl, normalizedColor, BlVec2(0.0f, 1.0f));
        // BlTextureVertex vertexTR = BlTextureVertex(tr, normalizedColor, BlVec2(1.0f, 0.0f));
        // BlTextureVertex vertexBR = BlTextureVertex(br, normalizedColor, BlVec2(1.0f, 1.0f));
        // BlTextureVertex vertexTL = BlTextureVertex(tl, normalizedColor, BlVec2(0.0f, 0.0f));

        u32 indicies[] = {
            0, 1, 2,
            0, 1, 3
        };

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
        buffer.Vertices = vertices;
        buffer.VertexSize = sizeof(f32);
        buffer.VertexCount = 16;

        buffer.Indices = indicies;
        buffer.IndexSize = sizeof(u32);
        buffer.IndexCount = 6;

        auto& renderer = BL_APP.GetRenderer();
        renderer.BindShader(m_OutlineShader);

        m_OutlineShader.SetVec2("u_TexelSize", BlVec2(1.0f / m_RenderTexture.Texture.Width, 1.0f / m_RenderTexture.Texture.Height));
        m_OutlineShader.SetVec3("u_OutlineColor", BlVec3(0.26f, 0.59f, 0.98f));
        m_OutlineShader.SetFloat("u_OutlineThickness", 5.0f);
        m_OutlineShader.SetFloat("u_OutlineStrength", 3.0f);

        renderer.AttachRenderTexture(m_OutlineTexture);
        renderer.Clear(BlColor(0, 0, 0, 255));

        renderer.AttachTexture(m_MaskTexture.Texture);

        renderer.SetBufferLayout(vertPosLayout);
        renderer.SetBufferLayout(vertTexCoordLayout);

        renderer.SubmitDrawBuffer(buffer);

        renderer.DrawIndexed(6);

        renderer.DetachRenderTexture();

#endif
    }
    
    void EditorLayer::OnEvent(const Blackberry::Event& event) {
        if (event.GetEventType() == Blackberry::EventType::WindowResize) {
            auto& wr = BL_EVENT_CAST(WindowResizeEvent);
    
            // m_RenderTexture.Delete();
            // m_RenderTexture.Create(wr.GetWidth(), wr.GetHeight());
        }
    
        if (event.GetEventType() == Blackberry::EventType::KeyPressed) {
            auto& kp = BL_EVENT_CAST(KeyPressedEvent);
            if (kp.GetKeyCode() == KeyCode::F3) {
                m_ShowDemoWindow = true;
            }

            if (kp.GetKeyCode() == KeyCode::F) {
                OnScenePlay();
            }
            if (kp.GetKeyCode() == KeyCode::G) {
                OnSceneStop();
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

    void EditorLayer::UI_AssetManager() {
        namespace fs = std::filesystem;
    
        ImGui::Begin("Asset Manager");
    
        if (m_CurrentDirectory != m_BaseDirectory) {
            if (ImGui::ImageButton("##BackDirectory", m_BackDirectoryIcon.ID, ImVec2(32.0f, 32.0f))) {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }
    
        static f32 padding = 16.0f;
        static f32 thumbnailSize = 128.0f;
        f32 cellSize = thumbnailSize + padding;
    
        f32 panelWidth = ImGui::GetContentRegionAvail().x;
        u32 columnCount = (u32)(panelWidth / cellSize);
        if (columnCount < 1) {
            columnCount = 1;
        }
    
        ImGui::Columns(columnCount, 0, false);
    
        for (const auto& file : fs::directory_iterator(m_CurrentDirectory)) {
            const auto& path = file.path();
            std::string name = path.filename().string();
    
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    
            if (file.is_directory()) {
                ImGui::ImageButton(name.c_str(), m_DirectoryIcon.ID, ImVec2(thumbnailSize, thumbnailSize));
            } else {
                ImGui::ImageButton(name.c_str(), m_FileIcon.ID, ImVec2(thumbnailSize, thumbnailSize));
            }
    
            ImGui::PopStyleVar();
    
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
                auto relative = fs::relative(path, m_BaseDirectory);
                std::string filePath = relative.string();
                ImGui::SetDragDropPayload("ASSET_DRAG_DROP", filePath.c_str(), filePath.size() + 1);
                ImGui::Text("%s", name.c_str());
                ImGui::EndDragDropSource();
            }
    
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (file.is_directory()) {
                    m_CurrentDirectory /= path.filename();
                }
            }
    
            ImGui::TextWrapped(name.c_str());
    
            ImGui::NextColumn();
        }
    
        ImGui::Columns(1);
    
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
                using namespace Blackberry::Components;

                if (ImGui::MenuItem("Rectangle")) {
                    Blackberry::Entity entity(m_CurrentScene->CreateEntity("Rectangle"), m_CurrentScene);
                    entity.AddComponent<Drawable>();
                    entity.AddComponent<Transform>({BlVec3(m_RenderTexture.Texture.Width / 2.0f - 100.0f, m_RenderTexture.Texture.Height / 2.0f - 50.0f, 0.0f), 0.0f, BlVec2(200.0f, 100.0f)});
                };

                if (ImGui::MenuItem("Triangle")) {
                    Blackberry::Entity entity(m_CurrentScene->CreateEntity("Triangle"), m_CurrentScene);
                    entity.AddComponent<Drawable>({.ShapeType = Shape::Triangle});
                    entity.AddComponent<Transform>({BlVec3(m_RenderTexture.Texture.Width / 2.0f - 100.0f, m_RenderTexture.Texture.Height / 2.0f - 50.0f, 0.0f), 0.0f, BlVec2(200.0f, 100.0f)});
                };
                
                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        };
        
        for (auto id : m_CurrentScene->GetEntities()) {
            ImGui::PushID(static_cast<u32>(id));
    
            Blackberry::Entity entity(id, m_CurrentScene);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(16, 0));

            if (m_SelectedEntity == id) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.4f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
                
                ImGui::Button(entity.GetComponent<Blackberry::Components::Tag>().Name.c_str(), ImVec2(ImGui::GetContentRegionAvail().x - 32.0f, 0));
                
                ImGui::PopStyleColor(3);
            } else if (entity.HasComponent<Blackberry::Components::Tag>()) {
                if (ImGui::Button(entity.GetComponent<Blackberry::Components::Tag>().Name.c_str(), ImVec2(ImGui::GetContentRegionAvail().x - 32.0f, 0))) {
                    m_IsEntitySelected = true;
                    m_SelectedEntity = id;
                }
            }

            if (ImGui::BeginPopupContextItem("EntityPopup")) {
                if (ImGui::MenuItem("Delete Entity")) {
                    m_CurrentScene->DestroyEntity(entity.GetComponent<Blackberry::Components::Tag>().UUID);

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
        using namespace Blackberry::Components;
    
        ImGui::Begin("Properties");
    
        if (m_IsEntitySelected) {
            Blackberry::Entity entity(m_SelectedEntity, m_CurrentScene);

            if (ImGui::BeginPopupContextWindow("PropertiesContextMenu")) {
                if (ImGui::BeginMenu("Add Component")) {
                    AddComponentListOption<Transform>("Transform", entity);
                    AddComponentListOption<Drawable>("Drawable", entity);
                    AddComponentListOption<Text>("Text", entity, {&m_EditorFont});
                    AddComponentListOption<Material>("Material", entity);
                    AddComponentListOption<Script>("Script", entity);
                    AddComponentListOption<Velocity>("Velocity", entity);
                    
                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }
    
            Tag& tag = entity.GetComponent<Tag>();

            ImGui::Text("Name: "); ImGui::SameLine();
            ImGui::InputText("##EntityName", &tag.Name);
            ImGui::TextDisabled("UUID: %llu", tag.UUID);

            ImGui::SeparatorText("Components: ");

            DrawComponent<Text>("Text", entity, [](Text& text) {
                int size = text.FontSize;
    
                ImGui::InputText("Cotents: ", &text.Contents); 
                ImGui::InputInt("Font size", &size);
    
                text.FontSize = size;
            });
            DrawComponent<Transform>("Transform", entity, [](Transform& transform) {
                DrawVec3Control("Position: ", &transform.Position);

                ImGui::DragFloat("Rotation: ", &transform.Rotation);

                DrawVec2Control("Dimensions: ", &transform.Dimensions);
            });
            DrawComponent<Drawable>("Drawable", entity, [](Drawable& drawable) {
                DrawColorControl("Color: ", &drawable.Color);

                ImGuiIO& io = ImGui::GetIO();

                ImGui::PushFont(io.Fonts->Fonts[1], 16);
                if (ImGui::TreeNode("Shape")) {
                    ImGui::PopFont();

                    const char* shapeNames[] = { "Triangle", "Rectangle", "Circle", "Polygon" };
                    int currentShape = static_cast<int>(drawable.ShapeType);
    
                    if (ImGui::Combo("Shape Type", &currentShape, shapeNames, IM_ARRAYSIZE(shapeNames))) {
                        drawable.ShapeType = static_cast<Shape>(currentShape);
                    }
    
                    ImGui::TreePop();
                } else {
                    ImGui::PopFont();
                }
            });
            DrawComponent<Material>("Material", entity, [this](Material& material) {
                ImGui::Button("Drop Texture Here!");
    
                if (ImGui::BeginDragDropTarget()) {
                    // we don't want to create a new texture if one already exists
                    if (!material.Texture.ID) {
                        if (auto payload = ImGui::AcceptDragDropPayload("ASSET_DRAG_DROP")) {
                            std::string strPath = (char*)payload->Data;
                            std::filesystem::path path(strPath);
                            path = m_BaseDirectory / path;
    
                            BlTexture tex;
                            tex.Create(path);
                            material.Texture = tex;
                            material.Area = BlRec(0.0f, 0.0f, tex.Width, tex.Height);
                            material.TexturePath = std::filesystem::relative(path, m_BaseDirectory);
                        }
                        
                        ImGui::EndDragDropTarget();
                    }
                }
    
                ImGui::SameLine();
                if (ImGui::Button("Remove Texture")) {
                    material.Texture.Delete();
                    material.TexturePath = "";
                }
    
                DrawRecControl("Area", &material.Area);
            });
            DrawComponent<Script>("Script", entity, [this](Script& script) {
                auto stringPath = script.ModulePath.string();

                ImGui::Text("Module Path: "); ImGui::SameLine();
                ImGui::InputText("##ModulePath", &stringPath);

                script.ModulePath = std::filesystem::path(stringPath);
                script.FilePath = m_BaseDirectory / script.ModulePath;
            });
            DrawComponent<Velocity>("Velocity", entity, [](Velocity& velocity) {
                DrawVec2Control("Acceleration: ", &velocity.Acceleration);
            });
        }
    
        ImGui::End();
    }
    
    void EditorLayer::UI_Viewport() {
        static int currentViewportSize = 1;
        static const char* viewportSizes[] = { "960x540", "1280x720", "1920x1080", "2048x1080", "2560x1440", "3840x2160", "7680x4320" };
        static bool showViewportOptionsWindow = false;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_MenuBar);
        ImGui::PopStyleVar();
    
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Viewport options")) {
                if (ImGui::BeginMenu("Size")) {
                    if (ImGui::Combo("Viewport size", &currentViewportSize, viewportSizes, IM_ARRAYSIZE(viewportSizes))) {
                        u32 width = 0;
                        u32 height = 0;
                
                        if (currentViewportSize == 0) {
                            width = 960;
                            height = 540;
                        } else if (currentViewportSize == 1) {
                            width = 1280;
                            height = 720;
                        } else if (currentViewportSize == 2) {
                            width = 1920;
                            height = 1080;
                        } else if (currentViewportSize == 3) {
                            width = 2048;
                            height = 1080;
                        } else if (currentViewportSize == 4) {
                            width = 2560;
                            height = 1440;
                        } else if (currentViewportSize == 5) {
                            width = 3840;
                            height = 2160;
                        } else if (currentViewportSize == 6) {
                            width = 7680;
                            height = 4320;
                        }
                
                        m_RenderTexture.Resize(width, height);
                        m_MaskTexture.Resize(width, height);
                        m_OutlineTexture.Resize(width, height);
                    }
        
                    ImGui::EndMenu();
                }
                
                ImGui::EndMenu();
            }
        
            ImGui::EndMenuBar();
        }

        auto viewportMin = ImGui::GetWindowContentRegionMin();
        auto viewportMax = ImGui::GetWindowContentRegionMax();

        ImVec2 area = ImGui::GetContentRegionAvail();

        f32 scale = area.x / static_cast<f32>(m_RenderTexture.Texture.Width);
        f32 y = m_RenderTexture.Texture.Height * scale;

        if (y > area.y) {
            scale = area.y / static_cast<f32>(m_RenderTexture.Texture.Height);
        }

        f32 sizeX = m_RenderTexture.Texture.Width * scale;
        f32 sizeY = m_RenderTexture.Texture.Height * scale;

        f32 cursorX = area.x / 2.0f - sizeX / 2.0f;
        f32 cursorY = ImGui::GetCursorPosY() + (area.y / 2.0f - sizeY / 2.0f);

        ImGui::SetCursorPosX(cursorX);
        ImGui::SetCursorPosY(cursorY);
    
        ImGui::Image(m_RenderTexture.Texture.ID, ImVec2(sizeX, sizeY), ImVec2(0, 1), ImVec2(1, 0));

        m_ViewportScale = scale;
        m_ViewportBounds.x = ImGui::GetWindowPos().x + cursorX;
        m_ViewportBounds.y = ImGui::GetWindowPos().y + cursorY;
        m_ViewportBounds.w = sizeX;
        m_ViewportBounds.h = sizeY;

        if (ImGui::BeginDragDropTarget()) {
            if (auto payload = ImGui::AcceptDragDropPayload("ASSET_DRAG_DROP")) {
                bool sceneExists = false;
    
                std::string strPath = (char*)payload->Data;
                std::filesystem::path path(strPath);
                path = m_BaseDirectory / path;
                
                for (auto& scene : m_CurrentProject.Scenes) {
                    if (scene.ScenePath == path) {
                        m_EditingScene = &scene.Scene;
                        sceneExists = true;
                    }
                }
    
                if (!sceneExists) {
                    m_EditingScene = LoadSceneFromFile(path);
                }

                m_CurrentScene = m_EditingScene;
            }
            ImGui::EndDragDropTarget();
        };
    
        ImGui::End();
    }

    void EditorLayer::UI_RendererStats() {
        ImGui::Begin("Renderer Stats");

        auto stats = Blackberry::Renderer2D::GetRenderingInfo();

        ImGui::Text("Draw Calls: %u", stats.DrawCalls);
        ImGui::Text("Verticies: %u", stats.Vertices);
        ImGui::Text("Indicies: %u", stats.Indicies);

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
            projectPath = Blackberry::OS::OpenFile("Blackberry Project (*.blproj)");
        }
    
        ImGui::Text("Asset Directory: "); ImGui::SameLine();
        ImGui::InputText("##AssetDirectory", &assetPath);
    
        if (ImGui::Button("Create")) {
            m_CurrentProject.Name = projectName;
            m_CurrentProject.ProjectDirectory = std::filesystem::path(projectPath).parent_path();
            m_CurrentProject.AssetDirectory = m_CurrentProject.ProjectDirectory / assetPath;
        }
    
        ImGui::End();
    }
    
    void EditorLayer::UI_NewScene() {
        static std::string scenePath;
    
        ImGui::Begin("New Scene", &m_ShowNewSceneWindow);
    
        ImGui::InputText("Scene Path: ", &scenePath); ImGui::SameLine();
    
        if (ImGui::Button("...")) {
            scenePath = Blackberry::OS::OpenFile("Blackberry Scene (*.blscene)");
        }
    
        if (ImGui::Button("Create")) {
            EditorScene scene;
            scene.ScenePath = scenePath;
            scene.Name = "New Scene";
            auto& newScene = m_CurrentProject.Scenes.emplace_back(scene);
            m_EditingScene = &newScene.Scene;
            m_CurrentScene = m_EditingScene;
    
            std::ofstream file(scenePath);
            file.close();
    
            m_ShowNewSceneWindow = false;
        };
    
        ImGui::End();
    }

#pragma endregion

#pragma region ProjectFunctions
    
    void EditorLayer::LoadProject() {
        std::string path = Blackberry::OS::OpenFile("Blackberry Project (*.blproj)");
    
        LoadProjectFromPath(path);
    }
    
    void EditorLayer::LoadProjectFromPath(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            LoadProject();
            return;
        }

        std::string contents = Blackberry::ReadEntireFile(path);
    
        json j = json::parse(contents);
        m_CurrentProject.ProjectDirectory = path.parent_path();
        std::string assetDir = j.at("AssetsDirectory");
        m_CurrentProject.AssetDirectory = path.parent_path() / assetDir;
    
        std::string startScene = j.at("StartScene");
        std::filesystem::path scenePath = m_CurrentProject.AssetDirectory / startScene;
        auto scene = LoadSceneFromFile(scenePath);

        m_CurrentProject.ProjectFilePath = path;
    
        m_EditingScene = scene;
        m_CurrentScene = m_EditingScene;
    } 
    
    void EditorLayer::SaveProject() {
        for (auto& scene : m_CurrentProject.Scenes) {
            SaveSceneToFile(scene.Scene, scene.ScenePath);
        }
    }
    
    Blackberry::Scene* EditorLayer::LoadSceneFromFile(const std::filesystem::path& path) {
        EditorScene scene;
        scene.ScenePath = path;
    
        Blackberry::SceneSerializer serializer(&scene.Scene, m_CurrentProject.AssetDirectory);
        serializer.Deserialize(path);
    
        m_CurrentProject.Scenes.emplace_back(scene);
    
        return &m_CurrentProject.Scenes.back().Scene;
    }
    
    void EditorLayer::SaveSceneToFile(Blackberry::Scene& scene, const std::filesystem::path& path) {
        Blackberry::SceneSerializer serializer(&scene, m_CurrentProject.AssetDirectory);
        serializer.Serialize(path);
    }
    
    void EditorLayer::NewProject() {
        SaveProject();
    
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
        m_ActiveScene = Blackberry::Scene::Copy(m_EditingScene);
        m_CurrentScene = m_ActiveScene;

        m_CurrentScene->OnPlay();
    }

    void EditorLayer::OnSceneStop() {
        m_CurrentScene->OnStop();

        BL_INFO("Reverted to editing scene.");
        m_EditorState = EditorState::Edit;
        m_CurrentScene = m_EditingScene;
        delete m_ActiveScene;
        m_ActiveScene = nullptr;
    }

    void EditorLayer::OnScenePause() {
    }

    void EditorLayer::SaveEditorState() {
        if (!std::filesystem::exists(m_AppDataDirectory / "Blackberry-Editor")) {
            std::filesystem::create_directory(m_AppDataDirectory / "Blackberry-Editor");
        }

        json j;
        j["LastProjectPath"] = m_CurrentProject.ProjectFilePath.string();
        j["ViewportDimensions"] = { m_RenderTexture.Texture.Width, m_RenderTexture.Texture.Height };

        std::ofstream file(m_AppDataDirectory / "Blackberry-Editor" / "editor_state.blsettings");
        file << j.dump(4);
    }

    void EditorLayer::LoadEditorState() {
        if (!std::filesystem::exists(m_AppDataDirectory / "Blackberry-Editor" / "editor_state.blsettings")) {
            return;
        }

        std::string contents = Blackberry::ReadEntireFile(m_AppDataDirectory / "Blackberry-Editor" / "editor_state.blsettings");
        json j = json::parse(contents);

        std::string lastProjectPath = j.at("LastProjectPath");
        LoadProjectFromPath(lastProjectPath);
        std::array<u32, 2> viewportDimensions = j.at("ViewportDimensions");
        m_RenderTexture.Resize(viewportDimensions[0], viewportDimensions[1]);
        m_MaskTexture.Resize(viewportDimensions[0], viewportDimensions[1]);
    }

#pragma endregion

} // namespace BlackberryEditor