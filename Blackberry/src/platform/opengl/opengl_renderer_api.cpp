#include "platform/opengl/opengl_renderer_api.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/core/timer.hpp"

#include "glad/gl.h"

#include <iostream>

namespace Blackberry {

    static void GLAPIENTRY glDebugCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
        // Filter out notifications or spammy messages if you like
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
            return;

        std::cerr << "OpenGL Debug Message (" << id << "): " << message << std::endl;

        std::cerr << "Source: ";
        switch (source) {
            case GL_DEBUG_SOURCE_API:             std::cerr << "API"; break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Window System"; break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Shader Compiler"; break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Third Party"; break;
            case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Application"; break;
            case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Other"; break;
        }
        std::cerr << "\nType: ";
        switch (type) {
            case GL_DEBUG_TYPE_ERROR:               std::cerr << "Error"; break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Deprecated Behavior"; break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Undefined Behavior"; break;
            case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Portability"; break;
            case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Performance"; break;
            case GL_DEBUG_TYPE_MARKER:              std::cerr << "Marker"; break;
            case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Push Group"; break;
            case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Pop Group"; break;
            case GL_DEBUG_TYPE_OTHER:               std::cerr << "Other"; break;
        }
        std::cerr << "\nSeverity: ";
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "HIGH"; break;
            case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "MEDIUM"; break;
            case GL_DEBUG_SEVERITY_LOW:          std::cerr << "LOW"; break;
            case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "NOTIFICATION"; break;
        }
        std::cerr << "\n\n";
    }

    OpenGLRendererAPI::OpenGLRendererAPI() {
        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugCallbackFunction, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }

        // depth
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthRange(0.0, 1.0);

        // backface-culling
        // glEnable(GL_CULL_FACE);

        // seamless cubemaps
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        BL_CORE_INFO("Using OpenGL backend");
        BL_CORE_INFO("    Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
        BL_CORE_INFO("    Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        BL_CORE_INFO("    Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

        int maxUnits;
        int maxCombinedTexUnits;
        int maxTexSize;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxUnits);
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedTexUnits);
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);

        BL_CORE_INFO("OpenGL Capabilities:");
        BL_CORE_INFO("    Max Texture Size: {0}x{0}", maxTexSize);

        if (!GLAD_GL_ARB_bindless_texture) {
            BL_CORE_CRITICAL("Bindless textures are NOT supported!");
            exit(1);
        }
    }

    void OpenGLRendererAPI::SetViewportSize(BlVec2 size) const {
        glViewport(0, 0, size.x, size.y);
        m_PreviousFramebufferSize = m_CurrentFramebufferSize;
        m_CurrentFramebufferSize = size;
    }

    void OpenGLRendererAPI::ClearFramebuffer(const BlVec4& color) const {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void OpenGLRendererAPI::DrawVertexArray(const Ref<VertexArray>& vertexArray) const {
        glBindVertexArray(vertexArray->ID);

        if (vertexArray->HasIndexBuffer()) {
            glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->Count, GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, vertexArray->GetVertexBuffer()->Count);
        }

        glBindVertexArray(0);
    }

    void OpenGLRendererAPI::BindShader(const Ref<Shader>& shader) const {
        glUseProgram(shader->ID);
    }

    void OpenGLRendererAPI::BindFramebuffer(const Ref<Framebuffer>& framebuffer) const {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->ID);
        SetViewportSize(BlVec2(framebuffer->Specification.Width, framebuffer->Specification.Height));
    }

    void OpenGLRendererAPI::UnBindFramebuffer() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        SetViewportSize(m_PreviousFramebufferSize);
    }

    void OpenGLRendererAPI::BindTexture2D(const Ref<Texture2D>& texture, u32 slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture->ID);
    }

    void OpenGLRendererAPI::UnBindTexture2D() const {;
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OpenGLRendererAPI::BindTextureCubemap(const Ref<Texture2D>& texture, u32 slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture->ID);
    }

    void OpenGLRendererAPI::UnBindTextureCubemap() const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

} // namespace Blackberry
