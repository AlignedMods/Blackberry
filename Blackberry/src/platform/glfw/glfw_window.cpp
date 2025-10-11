#include "platform/glfw/glfw_window.hpp"
#include "GLFW/glfw3.h"
#include "blackberry/log.hpp"
#include "blackberry/application/application.hpp"
#include "blackberry/event/event.hpp"
#include "blackberry/event/key_events.hpp"
#include "blackberry/event/mouse_events.hpp"
#include "blackberry/event/window_events.hpp"
#include "blackberry/input/keycodes.hpp"
#include "blackberry/input/mousecodes.hpp"

#include "glad/glad.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#ifdef BL_WINDOWS_BUILD
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include "GLFW/glfw3native.h"
#endif

#include <thread>
#include <chrono>

#define DISPATCHER Dispatcher& dispatcher = Application::Get().GetDispatcher()

namespace Blackberry {

#pragma region Callbacks

    static KeyCode GLFWKeyToBlackberry(const i32 key);
    static MouseCode GLFWMouseToBlackberry(const i32 button);

    static void CallbackKey(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) {
        DISPATCHER;

        const KeyCode keyCode = GLFWKeyToBlackberry(key);

        if (action == GLFW_PRESS) {
            dispatcher.Post(KeyPressedEvent(keyCode, false));
        }
        else if (action == GLFW_REPEAT) {
            dispatcher.Post(KeyPressedEvent(keyCode, true));
        }
        else if (action == GLFW_RELEASE) {
            dispatcher.Post(KeyReleasedEvent(keyCode));
        }
    }

    static void CallbackChar(GLFWwindow* window, u32 codepoint) {
        DISPATCHER;

        dispatcher.Post(KeyTypedEvent(codepoint));
    }

    static void CallbackMouseButton(GLFWwindow* window, i32 button, i32 action, i32 mods) {
        DISPATCHER;

        MouseCode btn = GLFWMouseToBlackberry(button);

        if (action == GLFW_PRESS) {
            dispatcher.Post(MouseButtonPressedEvent(btn));
        }
        else {
            dispatcher.Post(MouseButtonReleasedEvent(btn));
        }
    }

    static void CallbackMouseMove(GLFWwindow* window, f64 x, f64 y) {
        DISPATCHER;

        dispatcher.Post(MouseMovedEvent(static_cast<u32>(x), static_cast<u32>(y)));
    }

    static void CallbackScroll(GLFWwindow* window, f64 x, f64 y) {
        DISPATCHER;

        dispatcher.Post(MouseScrolledEvent(static_cast<f32>(x)));
    }

    static void CallbackWindowResize(GLFWwindow* window, i32 width, i32 height) {
        DISPATCHER;

        dispatcher.Post(WindowResizeEvent(width, height));
    }

    static void CallbackWindowClose(GLFWwindow* window) {
        DISPATCHER;

        dispatcher.Post(WindowCloseEvent());
    }

#pragma endregion

#pragma region Utils

    static KeyCode GLFWKeyToBlackberry(const i32 key) {
        switch (key) {
            // non-printable keys
            case GLFW_KEY_ESCAPE: return KeyCode::Escape;
            case GLFW_KEY_TAB: return KeyCode::Tab;
            case GLFW_KEY_CAPS_LOCK: return KeyCode::CapsLock;
            case GLFW_KEY_LEFT_SHIFT:
            case GLFW_KEY_RIGHT_SHIFT: return KeyCode::Shift;
            case GLFW_KEY_LEFT_CONTROL:
            case GLFW_KEY_RIGHT_CONTROL: return KeyCode::Ctrl;
            case GLFW_KEY_LEFT_SUPER:
            case GLFW_KEY_RIGHT_SUPER: return KeyCode::Win;
            case GLFW_KEY_LEFT_ALT:
            case GLFW_KEY_RIGHT_ALT: return KeyCode::Alt;
            case GLFW_KEY_SPACE: return KeyCode::Space;
            case GLFW_KEY_LEFT: return KeyCode::Left;
            case GLFW_KEY_DOWN: return KeyCode::Down;
            case GLFW_KEY_UP: return KeyCode::Up;
            case GLFW_KEY_RIGHT: return KeyCode::Right;
            case GLFW_KEY_PAGE_DOWN: return KeyCode::PageDown;
            case GLFW_KEY_PAGE_UP: return KeyCode::PageUp;
            case GLFW_KEY_DELETE: return KeyCode::Del;
            case GLFW_KEY_HOME: return KeyCode::Home;
            case GLFW_KEY_BACKSPACE: return KeyCode::Backspace;
            case GLFW_KEY_ENTER: return KeyCode::Enter;

            // printable keys
            case GLFW_KEY_0: return KeyCode::Num0;
            case GLFW_KEY_1: return KeyCode::Num1;
            case GLFW_KEY_2: return KeyCode::Num2;
            case GLFW_KEY_3: return KeyCode::Num3;
            case GLFW_KEY_4: return KeyCode::Num4;
            case GLFW_KEY_5: return KeyCode::Num5;
            case GLFW_KEY_6: return KeyCode::Num6;
            case GLFW_KEY_7: return KeyCode::Num7;
            case GLFW_KEY_8: return KeyCode::Num8;
            case GLFW_KEY_9: return KeyCode::Num9;
            case GLFW_KEY_A: return KeyCode::A;
            case GLFW_KEY_B: return KeyCode::B;
            case GLFW_KEY_C: return KeyCode::C;
            case GLFW_KEY_D: return KeyCode::D;
            case GLFW_KEY_E: return KeyCode::E;
            case GLFW_KEY_F: return KeyCode::F;
            case GLFW_KEY_G: return KeyCode::G;
            case GLFW_KEY_H: return KeyCode::H;
            case GLFW_KEY_I: return KeyCode::I;
            case GLFW_KEY_J: return KeyCode::J;
            case GLFW_KEY_K: return KeyCode::K;
            case GLFW_KEY_L: return KeyCode::L;
            case GLFW_KEY_M: return KeyCode::M;
            case GLFW_KEY_N: return KeyCode::N;
            case GLFW_KEY_O: return KeyCode::O;
            case GLFW_KEY_P: return KeyCode::P;
            case GLFW_KEY_Q: return KeyCode::Q;
            case GLFW_KEY_R: return KeyCode::R;
            case GLFW_KEY_S: return KeyCode::S;
            case GLFW_KEY_T: return KeyCode::T;
            case GLFW_KEY_U: return KeyCode::U;
            case GLFW_KEY_V: return KeyCode::V;
            case GLFW_KEY_W: return KeyCode::W;
            case GLFW_KEY_X: return KeyCode::X;
            case GLFW_KEY_Y: return KeyCode::Y;
            case GLFW_KEY_Z: return KeyCode::Z;

            // function keys
            case GLFW_KEY_F1: return KeyCode::F1;
            case GLFW_KEY_F2: return KeyCode::F2;
            case GLFW_KEY_F3: return KeyCode::F3;
            case GLFW_KEY_F4: return KeyCode::F4;
            case GLFW_KEY_F5: return KeyCode::F5;
            case GLFW_KEY_F6: return KeyCode::F6;
            case GLFW_KEY_F7: return KeyCode::F7;
            case GLFW_KEY_F8: return KeyCode::F8;
            case GLFW_KEY_F9: return KeyCode::F9;
        }

        // this shouldn't really happen but i need to shut up the compiler
        return KeyCode::None;
    }

    static MouseCode GLFWMouseToBlackberry(const i32 button) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT: return MouseCode::Left;
            case GLFW_MOUSE_BUTTON_RIGHT: return MouseCode::Right;
            case GLFW_MOUSE_BUTTON_MIDDLE: return MouseCode::Middle;

            default: return MouseCode::None;
        }

        return MouseCode::None;
    }

#pragma endregion

    Window_GLFW::Window_GLFW(const WindowData& data)
        : Window(data) {
        if (!glfwInit()) {
            BL_CRITICAL("Failed to init GLFW!");
            glfwTerminate();
            exit(1);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);
        m_Handle = glfwCreateWindow(data.Width, data.Height, data.Name.c_str(), nullptr, nullptr);

        glfwSwapInterval(0);

        if (!m_Handle) {
            BL_CRITICAL("Failed to create GLFW window, Error code {}!", glfwGetError(nullptr));
            glfwTerminate();
            exit(1);
        }

        // needed to use any opengl
        glfwMakeContextCurrent(m_Handle);
        // glfwSwapInterval(1);

        // load GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            BL_CRITICAL("Failed to init GLAD (Needed for OpenGL!)");
            glfwTerminate();
            exit(1);
        }

        // glfwSetWindowIcon();

        glfwSetKeyCallback(m_Handle, CallbackKey);
        glfwSetCharCallback(m_Handle, CallbackChar);

        glfwSetMouseButtonCallback(m_Handle, CallbackMouseButton);
        glfwSetCursorPosCallback(m_Handle, CallbackMouseMove);
        glfwSetScrollCallback(m_Handle, CallbackScroll);

        glfwSetFramebufferSizeCallback(m_Handle, CallbackWindowResize);
        glfwSetWindowCloseCallback(m_Handle, CallbackWindowClose);

        ImGui_ImplGlfw_InitForOpenGL(m_Handle, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    Window_GLFW::~Window_GLFW() {
        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(m_Handle);
        glfwTerminate();
    }

    bool Window_GLFW::ShouldClose() const {
        return glfwWindowShouldClose(m_Handle);
    }

    void Window_GLFW::OnUpdate() {
        glfwPollEvents();
    }

    void Window_GLFW::OnRenderStart() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Window_GLFW::OnRenderFinish() {
        ImGuiIO& io = ImGui::GetIO();
        // io.DeltaTime = Application::Get().GetDeltaTime();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(m_Handle);
    }

    f64 Window_GLFW::GetTime() const {
        return glfwGetTime();
    }

    void Window_GLFW::SleepSeconds(f64 seconds) const {
        // security check
        if (seconds > 0.0) {
            f64 targetTime = seconds + GetTime();

            while (GetTime() < targetTime) {}
            // std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        }
    }

    void Window_GLFW::SetWindowIcon(const Image& image) {
        GLFWimage im;
        im.pixels = static_cast<u8*>(image.GetData());
        im.width = image.GetWidth();
        im.height = image.GetHeight();

        glfwSetWindowIcon(m_Handle, 1, &im);
    }

    std::string Window_GLFW::OpenFile(const char* filter) {
        #ifdef BL_WINDOWS_BUILD

        OPENFILENAMEA ofn;
        CHAR szFile[260]{};
        CHAR currentDir[256]{};
        ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window(m_Handle);
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        if (GetCurrentDirectoryA(256, currentDir))
            ofn.lpstrInitialDir = currentDir;
        ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE)
		    return ofn.lpstrFile;

        return std::string{};

        #endif
    }

    void* Window_GLFW::GetHandle() const {
        return m_Handle;
    }

} // namespace Blackberry
