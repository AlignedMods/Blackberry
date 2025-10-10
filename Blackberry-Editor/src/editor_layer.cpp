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

static void DrawVec2Control(const std::string& label, BlVec2* vec) {
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
        ImGui::Button("X");
        ImGui::PopStyleColor(3);
        ImGui::PopFont();

        ImGui::SameLine();
        ImGui::DragFloat("##DragX", &vec->x, 1.0f);

        // y axis control
        ImGui::PushFont(io.Fonts->Fonts[1], 16);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
        ImGui::Button("Y");
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
    color->r = imGuiColor.x * 255.0f;
    color->g = imGuiColor.y * 255.0f;
    color->b = imGuiColor.z * 255.0f;
    color->a = imGuiColor.w * 255.0f;
}

EditorLayer::~EditorLayer() {}

void EditorLayer::OnInit() {
    m_EditorFont.LoadFontFromFile("Assets/arial/arial.ttf", 36);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("Assets/arial/arial.ttf", 16);
    io.Fonts->AddFontFromFileTTF("Assets/arial/arial-bold.ttf", 16);

    m_RenderTexture.Create(1280, 720);

    Blackberry::Image image("Assets/blank.png");
    m_BlankTexture.Create(image);
}

void EditorLayer::OnUpdate(f32 ts) {
    m_EditorScene.OnUpdate();
}

void EditorLayer::OnRender() {
    Blackberry::AttachRenderTexture(m_RenderTexture);
    
    Blackberry::Clear();
    m_EditorScene.OnRender();

    Blackberry::DetachRenderTexture();

    // Blackberry::DrawRenderTexture(BlVec2(0.0f, 0.0f), BlVec2(1280.0f, 720.0f), m_RenderTexture);
}

static i32 s_CurrentItem = 0;

void EditorLayer::OnUIRender() {
    using namespace Blackberry::Components;

    // ImGui::ShowDemoWindow();

    // set up dockspace
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("dockspace window", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                                              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
    ImGui::PopStyleVar();

    ImGuiID dockspaceID = ImGui::GetID("dockspace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    
    ImGui::End();

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
        if (ImGui::Button(entity.GetComponent<Tag>().Name.c_str())) {
            m_IsEntitySelected = true;
            m_SelectedEntity = id;
        }

        ImGui::PopID();
    }

    ImGui::End();

    ImGui::Begin("Properties");

    if (m_IsEntitySelected) {
        Blackberry::Entity entity(m_SelectedEntity, &m_EditorScene);

        if (ImGui::Button("Add Component")) {
            ImGui::OpenPopup("AddComponent");
        }

        if (ImGui::BeginPopup("AddComponent")) {
            AddComponentListOption<Transform>("Transform", entity);
            AddComponentListOption<Drawable>("Drawable", entity);
            AddComponentListOption<Text>("Text", entity);
            AddComponentListOption<Material>("Material", entity, {m_BlankTexture});

            ImGui::EndPopup();
        }

        DrawComponent<Tag>("Tag", entity, [](Tag& tag) {
            ImGui::Text("Name: ");
            ImGui::SameLine();
            ImGui::InputText("##Name", &tag.Name);
        });
        DrawComponent<Text>("Text", entity, [](Text& text) {
            ImGui::Text("This is a text!");    
        });
        DrawComponent<Transform>("Transform", entity, [](Transform& transform) {
            DrawVec2Control("Position: ", &transform.Position);
            DrawVec2Control("Dimensions: ", &transform.Dimensions);
        });
        DrawComponent<Drawable>("Drawable", entity, [](Drawable& drawable) {
            DrawColorControl("Color: ", &drawable.Color);
        });
        DrawComponent<Material>("Material", entity, [](Material& material) {});
    }

    ImGui::End();

    ImGui::Begin("Asset Manager");
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport");

    ImGui::Image(m_RenderTexture.Texture.ID, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorLayer::OnEvent(const Blackberry::Event& event) {
    if (event.GetEventType() == Blackberry::EventType::WindowResize) {
        const auto& wr = BL_EVENT_CAST(WindowResizeEvent);

        m_RenderTexture.Delete();
        m_RenderTexture.Create(wr.GetWidth(), wr.GetHeight());
    }
}