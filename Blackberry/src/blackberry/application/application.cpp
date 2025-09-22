#include "application.hpp"
#include "blackberry/event/event.hpp"
#include "blackberry/event/key_events.hpp"
#include "blackberry/event/window_events.hpp"
#include "blackberry/event/mouse_events.hpp"
#include "blackberry/input/keycodes.hpp"
#include "blackberry/input/mousecodes.hpp"
#include "platform/glfw_window.hpp"
#include "platform/opengl3_renderer.hpp"

#include "imgui.h"
#include "glad/glad.h"
#include "stb_image.h"

namespace Blackberry {

    static Application* s_Instance = nullptr;

    Application::Application(const ApplicationSpecification& spec) 
        : m_Specification(spec) {
        WindowData data;
        data.name = spec.name;
        data.width = spec.width;
        data.height = spec.height;

        stbi_set_flip_vertically_on_load(true);

        m_Window = new Window_GLFW(data); // also sets up opengl

        BlVec2 viewport = BlVec2(static_cast<f32>(data.width), static_cast<f32>(data.height));
        m_Renderer = new Renderer_OpenGL3(viewport);

        m_TargetFPS = spec.FPS;
        m_LastTime = m_Window->GetTime();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        auto& colors = style.Colors;

        // buttons
        colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);

        // headers
        colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);

        // checkboxes
        colors[ImGuiCol_CheckMark] = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);

        // frame backgrounds (checkbox, radio)
        colors[ImGuiCol_FrameBg] = ImVec4(0.6f, 0.3f, 0.3f, 0.7f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.7f, 0.3f, 0.3f, 0.8f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.8f, 0.4f, 0.4f, 0.9f);

        // sliders
        colors[ImGuiCol_SliderGrab] = ImVec4(0.7f, 0.4f, 0.4f, 0.7f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.7f, 0.4f, 0.4f, 0.9f);

        // docking
        colors[ImGuiCol_DockingPreview] = ImVec4(0.6f, 0.3f, 0.3f, 0.7f);

        // tabs
        colors[ImGuiCol_Tab] = ImVec4(0.3f, 0.1f, 0.1f, 0.3f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.3f, 0.1f, 0.1f, 0.5f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.3f, 0.1f, 0.1f, 0.6f);
        colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.4f, 0.1f, 0.1f, 1.0f);
        colors[ImGuiCol_TabDimmed] = ImVec4(0.2f, 0.1f, 0.1f, 0.2f);
        colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.2f, 0.1f, 0.1f, 0.4f);

        // windows
        colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.5f, 0.2f, 0.2f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.4f, 0.2f, 0.2f, 0.8f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.5f, 0.2f, 0.2f, 0.9f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.6f, 0.2f, 0.2f, 1.0f);

        // the resize thingy on the edge on windows!
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.8f, 0.25f, 0.25f, 1.0f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.7f, 0.3f, 0.3f, 1.0f);

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        m_Initalized = true;

        Log(Log_Info, "Constructed application!");

        for (auto& layer : m_LayerStack.GetAllLayers()) {
            layer->OnInit();
        }

        s_Instance = this;
    }

    Application::~Application() {
        delete m_Window;
    }

    void Application::Run() {
        m_Running = true;

        while (m_Running) {
            m_Running = m_Running && !m_Window->ShouldClose();

            m_Window->OnUpdate();
            OnUpdate();

            m_Window->OnRenderStart();

            OnRender();
            OnUIRender();

            m_Window->OnRenderFinish();

            // Calculating the deltatime and timing
            m_CurrentTime = m_Window->GetTime();

            f64 drawRenderTime = m_CurrentTime - m_LastTime;

            if (m_TargetFPS > 0) {
                f64 waitTime = (1.0 / static_cast<f64>(m_TargetFPS)) - drawRenderTime;
                m_Window->SleepSeconds(waitTime);
                m_CurrentTime = m_Window->GetTime();
                m_dt = static_cast<f32>(m_CurrentTime - m_LastTime);
            } else {
                m_dt = static_cast<f32>(drawRenderTime);
            }

            m_LastTime = m_CurrentTime;
        }
    }

    bool Application::IsInitialized() {
        return m_Initalized;
    }

    void Application::SetTargetFPS(u32 fps) {
        m_TargetFPS = fps;
    }

    void Application::SetWindowIcon(const Image& image) {
        GLFWimage im{};

        im.width = image.width;
        im.height = image.height;
        im.pixels = (u8*)image.data;

        glfwSetWindowIcon((GLFWwindow*)m_Window->GetHandle(), 1, &im);
    }

    void Application::Close() {
        m_Running = false;
    }

    void Application::PushLayer(Layer* layer) {
        m_LayerStack.PushLayer(layer);
    }

    void Application::PopLayer() {
        m_LayerStack.PopLayer();
    }

    void Application::PopLayer(const std::string& name) {
        m_LayerStack.PopLayer(name);
    }

    void Application::OnUpdate() {
        if (m_Window->GetTime() - m_FixedUpdateTime > 0.0167) {
            m_FixedUpdateTime += 0.0167;

            for (auto layer : m_LayerStack.GetAllLayers()) {
                layer->OnFixedUpdate();
            }
        }

        for (auto layer : m_LayerStack.GetAllLayers()) {
            layer->OnUpdate(m_dt);
        }
    }

    static bool usingTex = false;

    void Application::OnRender() {
        m_Renderer->NewFrame();
        m_Renderer->Clear();

        for (auto layer : m_LayerStack.GetAllLayers()) {
            layer->OnRender();
        }
    }

    void Application::OnUIRender() {
        ImGuiIO& io = ImGui::GetIO();

        for (auto layer : m_LayerStack.GetAllLayers()) {
            layer->OnUIRender();
        }
    }

    void Application::OnEvent(const Event& event) {
        const auto type = event.GetEventType();

        if (type == EventType::WindowResize) {
            const auto& wr = BL_EVENT_CAST(WindowResizeEvent);
            m_Renderer->UpdateViewport(BlVec2((f32)wr.GetWidth(), (f32)wr.GetHeight()));
        }

        auto& stack = m_LayerStack.GetAllLayers();

        for (auto it = stack.rbegin(); it < stack.rend(); it++) {
            (*it)->OnEvent(event);
        }
    }

    Application& Application::Get() {
        return *s_Instance;
    }

} // namespace Blackberry