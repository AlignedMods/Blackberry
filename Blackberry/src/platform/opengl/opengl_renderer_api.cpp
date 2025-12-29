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

    static GLenum GetOpenGLRendererCapability(RendererCapability cap) {
        switch (cap) {
            case RendererCapability::Blend: return GL_BLEND;
            case RendererCapability::FaceCull: return GL_CULL_FACE;
            case RendererCapability::DepthTest: return GL_DEPTH_TEST;
            case RendererCapability::ScissorTest: return GL_SCISSOR_TEST;
            case RendererCapability::StencilTest: return GL_STENCIL_TEST;
            case RendererCapability::SeamlessCubemap: return GL_TEXTURE_CUBE_MAP_SEAMLESS;
            default: BL_ASSERT(false, "Unreachable"); return 0;
        }

        return 0;
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

    void OpenGLRendererAPI::EnableCapability(RendererCapability cap) const {
        glEnable(GetOpenGLRendererCapability(cap));
    }

    void OpenGLRendererAPI::DisableCapability(RendererCapability cap) const {
        glDisable(GetOpenGLRendererCapability(cap));
    }

    void OpenGLRendererAPI::SetBlendFunc(BlendFunc func1, BlendFunc func2) const {
        GLenum glFunc1 = 0;
        GLenum glFunc2 = 0;

        switch (func1) {
            case BlendFunc::Zero: glFunc1 = GL_ZERO; break;
            case BlendFunc::One: glFunc1 = GL_ONE; break;
            case BlendFunc::SrcColor: glFunc1 = GL_SRC_COLOR; break;
            case BlendFunc::OneMinusSrcColor: glFunc1 = GL_ONE_MINUS_SRC_COLOR; break;
            case BlendFunc::DstColor: glFunc1 = GL_DST_COLOR; break;
            case BlendFunc::OneMinusDstColor: glFunc1 = GL_ONE_MINUS_DST_COLOR; break;
            case BlendFunc::SrcAlpha: glFunc1 = GL_SRC_ALPHA; break;
            case BlendFunc::OneMinusSrcAlpha: glFunc1 = GL_ONE_MINUS_SRC_ALPHA; break;
            case BlendFunc::DstAlpha: glFunc1 = GL_DST_ALPHA; break;
            case BlendFunc::OneMinusDstAlpha: glFunc1 = GL_ONE_MINUS_DST_ALPHA; break;
        }

        switch (func2) {
            case BlendFunc::Zero: glFunc2 = GL_ZERO; break;
            case BlendFunc::One: glFunc2 = GL_ONE; break;
            case BlendFunc::SrcColor: glFunc2 = GL_SRC_COLOR; break;
            case BlendFunc::OneMinusSrcColor: glFunc2 = GL_ONE_MINUS_SRC_COLOR; break;
            case BlendFunc::DstColor: glFunc2 = GL_DST_COLOR; break;
            case BlendFunc::OneMinusDstColor: glFunc2 = GL_ONE_MINUS_DST_COLOR; break;
            case BlendFunc::SrcAlpha: glFunc2 = GL_SRC_ALPHA; break;
            case BlendFunc::OneMinusSrcAlpha: glFunc2 = GL_ONE_MINUS_SRC_ALPHA; break;
            case BlendFunc::DstAlpha: glFunc2 = GL_DST_ALPHA; break;
            case BlendFunc::OneMinusDstAlpha: glFunc2 = GL_ONE_MINUS_DST_ALPHA; break;
        }

        glBlendFunc(glFunc1, glFunc2);
    }

    void OpenGLRendererAPI::SetBlendEquation(BlendEquation eq) const {
        GLenum glEq = 0;

        switch (eq) {
            case BlendEquation::Add: glEq = GL_FUNC_ADD; break;
            case BlendEquation::Subtract: glEq = GL_FUNC_SUBTRACT; break;
            case BlendEquation::ReverseSubtract: glEq = GL_FUNC_REVERSE_SUBTRACT; break;
            case BlendEquation::Min: glEq = GL_MIN; break;
            case BlendEquation::Max: glEq = GL_MAX; break;
        }

        glBlendEquation(glEq);
    }

    void OpenGLRendererAPI::SetDepthFunc(DepthFunc func) const {
        GLenum glFunc = 0;

        switch (func) {
            case DepthFunc::Never: glFunc = GL_NEVER; break;
            case DepthFunc::Less: glFunc = GL_LESS; break;
            case DepthFunc::Equal: glFunc = GL_EQUAL; break;
            case DepthFunc::Lequal: glFunc = GL_LEQUAL; break;
            case DepthFunc::Greater: glFunc = GL_GREATER; break;
            case DepthFunc::NotEqual: glFunc = GL_NOTEQUAL; break;
            case DepthFunc::Gequal: glFunc = GL_GEQUAL; break;
            case DepthFunc::Always: glFunc = GL_ALWAYS; break;
        }

        glDepthFunc(glFunc);
    }

    void OpenGLRendererAPI::SetDepthMask(bool mask) const {
        glDepthMask(mask);
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

    void OpenGLRendererAPI::BindTexture2D(const Ref<Texture>& texture, u32 slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture->ID);
    }

    void OpenGLRendererAPI::UnBindTexture2D() const {;
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OpenGLRendererAPI::BindTextureCubemap(const Ref<Texture>& texture, u32 slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture->ID);
    }

    void OpenGLRendererAPI::UnBindTextureCubemap() const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

} // namespace Blackberry
