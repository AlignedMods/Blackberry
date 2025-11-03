#include "platform/opengl/opengl3_renderer.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/core/log.hpp"

#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"

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

    Renderer_OpenGL3::Renderer_OpenGL3(BlVec2 viewport) {
        UpdateViewport(viewport);

        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugCallbackFunction, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }

        // blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_MULTISAMPLE);

        // depth
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthRange(0.0, 1.0);

        BL_CORE_INFO("Using OpenGL backend");
        BL_CORE_INFO("    Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
        BL_CORE_INFO("    Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        BL_CORE_INFO("    Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

        int maxUnits;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxUnits);
        printf("Max texture units: %d\n", maxUnits);

        // generate VAOs and VBOs
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

        glBindVertexArray(0);
    }

    void Renderer_OpenGL3::UpdateViewport(BlVec2 viewport) {
        m_PrevViewportSize = m_CurrentViewportSize;
        m_CurrentViewportSize = viewport;
        glViewport(0, 0, static_cast<GLsizei>(viewport.x), static_cast<GLsizei>(viewport.y));

        m_DefaultProjection = glm::ortho(
            0.0f,                               // left
            static_cast<f32>(viewport.x),       // right
            static_cast<f32>(viewport.y),       // bottom
            0.0f,                               // top
            -1000.0f, 1000.0f                   // near-far
        );

        m_Projection = m_DefaultProjection;
    }

    void Renderer_OpenGL3::NewFrame() {}

    void Renderer_OpenGL3::BindShader(BlShader shader) {
        m_CurrentShader = shader;
        glUseProgram(shader.ID);
    }

    void Renderer_OpenGL3::AttachTexture(BlTexture texture, u32 slot) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture.ID);
    }

    void Renderer_OpenGL3::DetachTexture() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Renderer_OpenGL3::EndFrame() {}

    void Renderer_OpenGL3::Clear(BlColor color) const {
        glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    BlVec2 Renderer_OpenGL3::GetViewportSize() const {
        int size[4];
        glGetIntegerv(GL_VIEWPORT, size);

        return BlVec2(size[2], size[3]);
    }

    void Renderer_OpenGL3::SetBufferLayout(const BlDrawBufferLayout& layout) {
        GLenum type = GL_FLOAT;

        switch (layout.Type)
        {
            case BlDrawBufferLayout::ElementType::Float:
                type = GL_FLOAT;
                break;
            case BlDrawBufferLayout::ElementType::UInt:
                type = GL_UNSIGNED_INT;
                break;
            default:
                type = GL_FLOAT;
                break;
        }

        glBindVertexArray(m_VAO);

        glVertexAttribPointer(layout.Index, layout.Count, type, GL_FALSE, layout.Stride, (void*)layout.Offset);
        glEnableVertexAttribArray(layout.Index);

        glBindVertexArray(0);
    }

    void Renderer_OpenGL3::SubmitDrawBuffer(const BlDrawBuffer& buffer) {
        glBindVertexArray(m_VAO);

        glBufferData(GL_ARRAY_BUFFER, buffer.VertexCount * buffer.VertexSize, buffer.Vertices, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.IndexCount * buffer.IndexSize, buffer.Indices, GL_STREAM_DRAW);

        glBindVertexArray(0);
    }

    void Renderer_OpenGL3::DrawIndexed(u32 count) {
        if (count == 0) {
            BL_CORE_WARN("Submiting draw call with ZERO elements!");
        }

        // glUseProgram(m_CurrentShader.ID);
        glBindVertexArray(m_VAO);

        m_CurrentShader.SetMatrix("u_Projection", glm::value_ptr(m_Projection));

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(count), GL_UNSIGNED_INT, nullptr);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void Renderer_OpenGL3::AttachRenderTexture(const BlRenderTexture texture) {
        glBindFramebuffer(GL_FRAMEBUFFER, texture.ID);
        UpdateViewport(BlVec2(static_cast<f32>(texture.Texture.Width), static_cast<f32>(texture.Texture.Height)));
    }

    void Renderer_OpenGL3::DetachRenderTexture() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        UpdateViewport(m_PrevViewportSize);
    }

    void Renderer_OpenGL3::SetProjection(glm::mat4 projection) {
        m_Projection = projection;
    }

    void Renderer_OpenGL3::ResetProjection() {
        m_Projection = m_DefaultProjection;
    }

} // namespace Blackberry
