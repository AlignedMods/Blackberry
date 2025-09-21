#include "platform/opengl3_renderer.hpp"
#include "blackberry/util.hpp"
#include "blackberry/log.hpp"

#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"

namespace Blackberry {

    // shaders
    static const char* VertexShaderSource = MULTILINE_STR(
        // we cannot use '#' since the c preprocesser gets mad at us (actually only lsp cared but yknow)
        \x23version 330 core\n
        layout (location = 0) in vec3 a_Pos;
        layout (location = 1) in vec4 a_Color;
        layout (location = 2) in vec2 a_TexCoord;
    
        out vec4 col;
        out vec2 texCoord;

        uniform mat4 projection;
    
        void main() {
            gl_Position = projection * vec4(a_Pos, 1.0);
            col = a_Color;
            texCoord = a_TexCoord;
        }
    );

    static const char* FragmentShaderSource = MULTILINE_STR(
        \x23version 330 core\n

        in vec4 col;
        in vec2 texCoord;

        out vec4 FragColor;

        uniform sampler2D sampler;
        uniform int useTexture;

        void main() {
            if (useTexture == 1) {
                vec2 invert = texCoord;
                invert.y = 1.0 - invert.y;

                vec4 texelColor = texture(sampler, invert);
                FragColor = col * texelColor;
            } else {
                FragColor = col;
            }
        }
    );

    Renderer_OpenGL3::Renderer_OpenGL3(BlVec2 viewport) {
        UpdateViewport(viewport);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_MULTISAMPLE);

        // compile shaders
        i32 errorCode = 0;
        char buf[512];

        u32 vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &VertexShaderSource, nullptr);
        glCompileShader(vertex);

        glGetShaderiv(vertex, GL_COMPILE_STATUS, &errorCode);

        if (!errorCode) {
            glGetShaderInfoLog(vertex, 512, nullptr, buf);
            Log(Log_Error, "Failed to compile default vertex shader! Error: %s", buf);
        }

        u32 fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &FragmentShaderSource, nullptr);
        glCompileShader(fragment);

        glGetShaderiv(fragment, GL_COMPILE_STATUS, &errorCode);

        if (!errorCode) {
            glGetShaderInfoLog(fragment, 512, nullptr, buf);
            Log(Log_Error, "Failed to compile default fragment shader! Error: %s", buf);
        }

        m_Shader = glCreateProgram();
        glAttachShader(m_Shader, vertex);
        glAttachShader(m_Shader, fragment);
        glLinkProgram(m_Shader);

        glGetProgramiv(m_Shader, GL_LINK_STATUS, &errorCode);

        if (!errorCode) {
            glGetProgramInfoLog(m_Shader, 512, nullptr, buf);
            Log(Log_Error, "Failed to link default shader program! Error: %s", buf);
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        // generate VAOs and VBOs
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
    }

    void Renderer_OpenGL3::UpdateViewport(BlVec2 viewport) {
        glViewport(0, 0, static_cast<GLsizei>(viewport.x), static_cast<GLsizei>(viewport.y));

        m_Projection = glm::ortho(
            0.0f,            // left
            (f32)viewport.x, // right
            (f32)viewport.y, // bottom
            0.0f,            // top
            -1.0f, 1.0f      // near-far
        );
    }

    void Blackberry::Renderer_OpenGL3::NewFrame() {
        m_DrawCalls = 0;
    }

    void Renderer_OpenGL3::VertexV(const BlVertex& v) {
        m_Vertices[m_VertexIndex++] = v.pos.x;
        m_Vertices[m_VertexIndex++] = v.pos.y;
        m_Vertices[m_VertexIndex++] = 0.0f;

        m_Vertices[m_VertexIndex++] = v.color.r / 255.0f;
        m_Vertices[m_VertexIndex++] = v.color.g / 255.0f;
        m_Vertices[m_VertexIndex++] = v.color.b / 255.0f;
        m_Vertices[m_VertexIndex++] = v.color.a / 255.0f;

        m_Vertices[m_VertexIndex++] = m_CurrentTexCoord.x;
        m_Vertices[m_VertexIndex++] = m_CurrentTexCoord.y;

        m_CurrentVertex++;
    }

    void Renderer_OpenGL3::Begin(RenderingMode mode) {
        assert(m_CurrentVertex == 0 && "Renderer::Begin() called but vertex count is NOT 0, did you forget to call Renderer::End()?");
        m_Mode = mode;
    }

    void Renderer_OpenGL3::End() {
        Render();

        m_VertexIndex = 0; // reset the vertex counter
        m_CurrentVertex = 0;
    }

    void Renderer_OpenGL3::Clear() const {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Renderer_OpenGL3::TexCoord2f(f32 x, f32 y) {
        m_CurrentTexCoord.x = x;
        m_CurrentTexCoord.y = y;
    }

    void Renderer_OpenGL3::UseTexture(const Texture& tex) {
        if (tex.id == 0) { m_UsingTexture = false; return; }

        m_UsingTexture = true;

        m_CurrentTexture = &tex;

        // glBindTexture(GL_TEXTURE_2D, tex.id);
    }

    Texture Blackberry::Renderer_OpenGL3::GenTexture(const Image& image)
    {
        Texture tex{};

        tex.width = image.width;
        tex.height = image.height;

        glGenTextures(1, &tex.id);
        glBindTexture(GL_TEXTURE_2D, tex.id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        return tex;
    }

    void Renderer_OpenGL3::Render() {
        if (m_CurrentVertex == 0) { return; } // nothing to render

        GLenum glMode;

        switch (m_Mode) {
            case RenderingMode::Points: glMode = GL_POINTS; break;
            case RenderingMode::Lines: glMode = GL_LINES; break;
            case RenderingMode::Triangles: glMode = GL_TRIANGLES; break;
            default: glMode = GL_POINTS; break;
        }

        glUseProgram(m_Shader);
        u32 uni = glGetUniformLocation(m_Shader, "projection");
        glUniformMatrix4fv(uni, 1, GL_FALSE, glm::value_ptr(m_Projection));

        if (m_UsingTexture) {
            u32 useTex = glGetUniformLocation(m_Shader, "useTexture");
            glUniform1i(useTex, 1);

            glBindTexture(GL_TEXTURE_2D, m_CurrentTexture->id);
        } else {
            u32 useTex = glGetUniformLocation(m_Shader, "useTexture");
            glUniform1i(useTex, 0);
        }

        glBindVertexArray(m_VAO);

        // upload vertex data to funny gpu
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_VertexIndex * sizeof(f32), m_Vertices, GL_STREAM_DRAW);

        // vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(0));
        glEnableVertexAttribArray(0);

        // vertex color
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(3 * sizeof(f32)));
        glEnableVertexAttribArray(1);

        // tex coord
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(7 * sizeof(f32)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(glMode, 0, static_cast<GLsizei>(m_CurrentVertex));

        glBindVertexArray(0);

        m_DrawCalls++;
    }

} // namespace Blackberry
