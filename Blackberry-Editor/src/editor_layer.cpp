#include "editor_layer.hpp"

#include <fstream>

static char s_Buffer[512];

template <typename T, typename F>
static void DrawComponent(const std::string& name, Blackberry::Entity entity, F uiFunction) {
    if (entity.HasComponent<T>()) {
        T& component = entity.GetComponent<T>();

        if (ImGui::CollapsingHeader(name.c_str())) {
            uiFunction(component);
        }
    }
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

EditorLayer::~EditorLayer() {
    SaveProject();
}

void EditorLayer::OnInit() {
    m_EditorFont.CreateFont();

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("Assets/arial/arial.ttf", 16);
    io.Fonts->AddFontFromFileTTF("Assets/arial/arial-bold.ttf", 16);

    m_RenderTexture.Create(1280, 720);

    Blackberry::Image image("Assets/blank.png");
    m_BlankTexture.Create(image);

    LoadProject();
}

void EditorLayer::OnUpdate(f32 ts) {
    m_EditorScene.OnUpdate();
}

void EditorLayer::OnRender() {
    Blackberry::AttachRenderTexture(m_RenderTexture);
    
    Blackberry::Clear();
    m_EditorScene.OnRender();

    Blackberry::DetachRenderTexture();
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
            if (ImGui::MenuItem("Save Project", "CTRL+S")) {
                SaveProject();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
    
    ImGui::End();

    UI_AssetManager();
    UI_Explorer();
    UI_Properties();
    UI_Viewport();
}

void EditorLayer::OnEvent(const Blackberry::Event& event) {
    if (event.GetEventType() == Blackberry::EventType::WindowResize) {
        auto& wr = BL_EVENT_CAST(WindowResizeEvent);

        // m_RenderTexture.Delete();
        // m_RenderTexture.Create(wr.GetWidth(), wr.GetHeight());
    }
}

void EditorLayer::UI_AssetManager() {
    ImGui::Begin("Asset Manager");

    if (ImGui::Button("+")) {
        ImGui::OpenPopup("ImportAsset");
    }

    if (ImGui::BeginPopup("ImportAsset")) {
        static std::string path;
        static std::string name;

        ImGui::Text("Path: ");
        ImGui::SameLine();
        ImGui::InputText("##FilePath", &path);

        ImGui::SameLine();
        if (ImGui::Button("...")) {
            path = Blackberry::FileDialogs::OpenFile("");
        }

        ImGui::NewLine();
        ImGui::Text("Name: ");
        ImGui::SameLine();
        ImGui::InputText("##AssetName", &name);

        if (ImGui::Button("OK")) {
            m_AssetManager.AddTextureFromPath(name, path);

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    for (const auto&[handle, asset] : m_AssetManager.GetAllAssets()) {
        std::string stringHandle = std::format("{}", handle);
        
        ImGui::Text(stringHandle.c_str());
        ImGui::SameLine();
        ImGui::PushID(static_cast<int>(handle));
        if (ImGui::Button("Copy to clipboard")) {
            ImGui::SetClipboardText(stringHandle.c_str());
        }
        ImGui::PopID();
    }

    ImGui::End();
}

void EditorLayer::UI_Explorer() {
    ImGui::Begin("Explorer");

    if (ImGui::Button("Add Entity")) {
        memset(s_Buffer, 0, 512);
        ImGui::OpenPopup("EntityName");
    }

    if (ImGui::BeginPopup("EntityName")) {
        ImGui::InputText("Name: ", s_Buffer, 512);
        if (ImGui::Button("OK")) {
            Blackberry::Entity entity(m_EditorScene.CreateEntity(s_Buffer), &m_EditorScene);
            
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::NewLine();
    
    for (auto id : m_EditorScene.GetEntities()) {
        ImGui::PushID(id);

        Blackberry::Entity entity(id, &m_EditorScene);
        if (ImGui::Button(entity.GetComponent<Blackberry::Components::Tag>().Name.c_str())) {
            m_IsEntitySelected = true;
            m_SelectedEntity = id;
        }

        ImGui::PopID();
    }

    ImGui::End();
}

void EditorLayer::UI_Properties() {
    using namespace Blackberry::Components;

    ImGui::Begin("Properties");

    if (m_IsEntitySelected) {
        Blackberry::Entity entity(m_SelectedEntity, &m_EditorScene);

        if (ImGui::Button("Add Component")) {
            ImGui::OpenPopup("AddComponent");
        }

        if (ImGui::BeginPopup("AddComponent")) {
            AddComponentListOption<Transform>("Transform", entity);
            AddComponentListOption<Drawable>("Drawable", entity);
            AddComponentListOption<Text>("Text", entity, {&m_EditorFont});
            AddComponentListOption<Material>("Material", entity, {m_BlankTexture});

            ImGui::EndPopup();
        }

        DrawComponent<Tag>("Tag", entity, [](Tag& tag) {
            ImGui::Text("Name: ");
            ImGui::SameLine();
            ImGui::InputText("##Name", &tag.Name);

            ImGui::Text("UUID: ");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 0.6f), "%llu", tag.UUID);
        });
        DrawComponent<Text>("Text", entity, [](Text& text) {
            int size = text.FontSize;

            ImGui::InputText("Cotents: ", &text.Contents); 
            ImGui::InputInt("Font size", &size);

            text.FontSize = size;
        });
        DrawComponent<Transform>("Transform", entity, [](Transform& transform) {
            DrawVec2Control("Position: ", &transform.Position);
            DrawVec2Control("Dimensions: ", &transform.Dimensions);
        });
        DrawComponent<Drawable>("Drawable", entity, [](Drawable& drawable) {
            DrawColorControl("Color: ", &drawable.Color);
        });
        DrawComponent<Material>("Material", entity, [this](Material& material) {
            static std::string handle = std::to_string(material.Texture.Handle);

            if (ImGui::InputText("Handle", &handle, ImGuiInputTextFlags_EnterReturnsTrue)) {
                Blackberry::Asset asset = m_AssetManager.GetAsset(std::stoull(handle));

                BlTexture tex = std::get<BlTexture>(asset.Data);
            
                material.Texture = tex;
                material.Area = BlRec(0.0f, 0.0f, tex.Width, tex.Height);
            }

            DrawRecControl("Area", &material.Area);
        });
    }

    ImGui::End();
}

void EditorLayer::UI_Viewport() {
    static int currentViewportSize = 1;
    static const char* viewportSizes[] = { "960x540", "1280x720", "1920x1280", "2048x1080", "2560x1440", "3840x2160", "7680x4320" };
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
                        height = 1280;
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
            
                    m_RenderTexture.Rezize(width, height);
                }

                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::Image(m_RenderTexture.Texture.ID, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
}

void EditorLayer::LoadProject() {
    std::string path = Blackberry::FileDialogs::OpenFile("Blackberry Project (*.blproj)");

    LoadProjectFromPath(path);
}

void EditorLayer::LoadProjectFromPath(const std::filesystem::path& path) {
    std::string contents = Blackberry::ReadEntireFile(path);

    json j = json::parse(contents);
    m_ProjectPath = path.parent_path();
    std::string assetDir = j.at("AssetsDirectory");
    m_AssetDir = m_ProjectPath / assetDir;

    std::string assetRegistry = j.at("AssetRegistry");
    std::filesystem::path assetRegistryPath = m_AssetDir / assetRegistry;
    LoadAssetRegistryFromFile(assetRegistryPath);

    std::string startScene = j.at("StartScene");
    std::filesystem::path scenePath = m_AssetDir / startScene;
    LoadSceneFromFile(scenePath);
}

void EditorLayer::LoadAssetRegistryFromFile(const std::filesystem::path& path) {
    Blackberry::AssetManagerSerializer serializer(&m_AssetManager);
    serializer.Deserialize(path);

    m_CurrentAssetRegistryPath = path;
}

void EditorLayer::LoadSceneFromFile(const std::filesystem::path& path) {
    Blackberry::SceneSerializer serializer(&m_EditorScene, &m_AssetManager);
    serializer.Deserialize(path);

    m_CurrentScenePath = path;
}

void EditorLayer::SaveProject() {
    SaveAssetRegistryToFile(m_CurrentAssetRegistryPath);
    SaveSceneToFile(m_CurrentScenePath);
}

void EditorLayer::SaveAssetRegistryToFile(const std::filesystem::path& path) {
    Blackberry::AssetManagerSerializer serializer(&m_AssetManager);
    serializer.Serialize(path);
}

void EditorLayer::SaveSceneToFile(const std::filesystem::path& path) {
    Blackberry::SceneSerializer serializer(&m_EditorScene, &m_AssetManager);
    serializer.Serialize(path);
}