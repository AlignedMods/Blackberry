#include "platform/opengl/opengl3_renderer.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/core/log.hpp"

#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"

namespace Blackberry {

    // shaders
    static const char* s_VertexShapeShaderSource = BL_STR(
        // we cannot use '#' since the c preprocesser gets mad at us (actually only lsp cared but yknow)
        \x23version 460 core\n
        layout (location = 0) in vec2 a_Pos;
        layout (location = 1) in vec4 a_Color;
    
        out vec4 col;

        uniform mat4 u_Projection;
    
        void main() {
            gl_Position = u_Projection * vec4(a_Pos.x, a_Pos.y, 0.0, 1.0);
            col = a_Color;
        }
    );

    static const char* s_FragmentShapeShaderSource = BL_STR(
        \x23version 460 core\n

        in vec4 col;

        out vec4 FragColor;

        void main() {
            FragColor = col;
        }
    );

    static const char* s_FragmentTextureShaderSource = BL_STR(
        \x23version 330 core\n

        in vec4 col;
        in vec2 texCoord;

        out vec4 FragColor;

        uniform sampler2D u_Sampler;

        void main() {
            vec4 texColor = texture(u_Sampler, texCoord);
            FragColor = texColor * col;
        }
    );

    static const char* s_FragmentFontShaderSource = BL_STR(
        \x23version 330 core\n

        in vec4 col;
        in vec2 texCoord;

        out vec4 FragColor;

        uniform sampler2D u_FontAtlas;

        void main() {
            FragColor = vec4(0, 0, 0, 0);
        }
    );

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

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_MULTISAMPLE);

        // compile shaders
        CompileDefaultShaders();

        // generate VAOs and VBOs
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        // glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        glBindVertexArray(0);
    }

    void Renderer_OpenGL3::UpdateViewport(BlVec2 viewport) {
        m_PrevViewportSize = m_CurrentViewportSize;
        m_CurrentViewportSize = viewport;
        glViewport(0, 0, static_cast<GLsizei>(viewport.x), static_cast<GLsizei>(viewport.y));

        m_Projection = glm::ortho(
            0.0f,            // left
            (f32)viewport.x, // right
            (f32)viewport.y, // bottom
            0.0f,            // top
            -1.0f, 1.0f      // near-far
        );
    }

    void Renderer_OpenGL3::NewFrame() {}

    void Renderer_OpenGL3::AttachDefaultShader(DefaultShader shader) {
        switch (shader) {
            case DefaultShader::Shape:
                m_CurrentShader = m_ShapeShader;
                break;
            case DefaultShader::Texture:
                m_CurrentShader = m_TextureShader;
                break;
            case DefaultShader::Font:
                m_CurrentShader = m_FontShader;
                break;
        }
    }

    void Renderer_OpenGL3::EndFrame() {}

    void Renderer_OpenGL3::Clear(BlColor color) const {
        glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Renderer_OpenGL3::SetBufferLayout(const BlDrawBufferLayout& layout) {
        GLenum type = GL_FLOAT;

        switch (layout.Type)
        {
            case BlDrawBufferLayout::ElementType::Float:
                type = GL_FLOAT;
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

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, buffer.VertexCount * buffer.VertexSize, buffer.Vertices, GL_STREAM_DRAW);
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.Indices.size() * sizeof(u32), buffer.Indices.data(), GL_STREAM_DRAW);

        glBindVertexArray(0);
    }

    void Renderer_OpenGL3::DrawIndexed(u32 count) {
        glUseProgram(m_ShapeShader);
        glBindVertexArray(m_VAO);

        glUniformMatrix4fv(glGetUniformLocation(m_CurrentShader, "u_Projection"), 1, GL_FALSE, glm::value_ptr(m_Projection));

        // glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(count), GL_UNSIGNED_INT, nullptr);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        GLenum error = glGetError();

        // glBindVertexArray(0);
    }

    void Renderer_OpenGL3::AttachRenderTexture(const BlRenderTexture texture) {
        glBindFramebuffer(GL_FRAMEBUFFER, texture.ID);
        UpdateViewport(BlVec2(static_cast<f32>(texture.Texture.Width), static_cast<f32>(texture.Texture.Height)));
    }

    void Renderer_OpenGL3::DetachRenderTexture() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        UpdateViewport(m_PrevViewportSize);
    }

    void Renderer_OpenGL3::CompileDefaultShaders() {
        i32 errorCode = 0;
        char buf[512];

        u32 vertexShape = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShape, 1, &s_VertexShapeShaderSource, nullptr);
        glCompileShader(vertexShape);

        glGetShaderiv(vertexShape, GL_COMPILE_STATUS, &errorCode);

        if (!errorCode) {
            glGetShaderInfoLog(vertexShape, 512, nullptr, buf);
            BL_ERROR("Failed to compile default vertex shape shader! Error: {}", buf);
        }

        // compile shape shader
        u32 fragmentShape = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShape, 1, &s_FragmentShapeShaderSource, nullptr);
        glCompileShader(fragmentShape);

        glGetShaderiv(fragmentShape, GL_COMPILE_STATUS, &errorCode);

        if (!errorCode) {
            glGetShaderInfoLog(fragmentShape, 512, nullptr, buf);
            BL_ERROR("Failed to compile default fragment shape shader! Error: {}", buf);
        }

        m_ShapeShader = glCreateProgram();
        glAttachShader(m_ShapeShader, vertexShape);
        glAttachShader(m_ShapeShader, fragmentShape);
        glLinkProgram(m_ShapeShader);

        glGetProgramiv(m_ShapeShader, GL_LINK_STATUS, &errorCode);

        if (!errorCode) {
            glGetProgramInfoLog(m_ShapeShader, 512, nullptr, buf);
            BL_ERROR("Failed to link default shape shader program! Error: {}", buf);
        }
        glDeleteShader(fragmentShape);
        glDeleteShader(vertexShape);

#if 0

        // compile texture shader
        u32 fragmentTexture = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentTexture, 1, &s_FragmentTextureShaderSource, nullptr);
        glCompileShader(fragmentTexture);

        glGetShaderiv(fragmentTexture, GL_COMPILE_STATUS, &errorCode);

        if (!errorCode) {
            glGetShaderInfoLog(fragmentTexture, 512, nullptr, buf);
            BL_ERROR("Failed to compile default fragment texture shader! Error: {}", buf);
        }

        m_TextureShader = glCreateProgram();
        glAttachShader(m_TextureShader, vertex);
        glAttachShader(m_TextureShader, fragmentTexture);
        glLinkProgram(m_TextureShader);

        glGetProgramiv(m_TextureShader, GL_LINK_STATUS, &errorCode);

        if (!errorCode) {
            glGetProgramInfoLog(m_TextureShader, 512, nullptr, buf);
            BL_ERROR("Failed to link default texture shader program! Error: {}", buf);
        }
        glDeleteShader(fragmentTexture);



        // compile font shader
        u32 fragmentFont = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentFont, 1, &s_FragmentFontShaderSource, nullptr);
        glCompileShader(fragmentFont);

        glGetShaderiv(fragmentFont, GL_COMPILE_STATUS, &errorCode);

        if (!errorCode) {
            glGetShaderInfoLog(fragmentFont, 512, nullptr, buf);
            BL_ERROR("Failed to compile default fragment font shader! Error: {}", buf);
        }

        m_FontShader = glCreateProgram();
        glAttachShader(m_FontShader, vertex);
        glAttachShader(m_FontShader, fragmentFont);
        glLinkProgram(m_FontShader);

        glGetProgramiv(m_FontShader, GL_LINK_STATUS, &errorCode);

        if (!errorCode) {
            glGetProgramInfoLog(m_FontShader, 512, nullptr, buf);
            BL_ERROR("Failed to link default texture font program! Error: {}", buf);
        }
        glDeleteShader(fragmentTexture);

        glDeleteShader(vertex);

#endif
    }

} // namespace Blackberry
