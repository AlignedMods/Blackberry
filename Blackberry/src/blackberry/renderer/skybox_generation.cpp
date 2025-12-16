#include "blackberry/renderer/skybox_generation.hpp"
#include "blackberry/renderer/shader.hpp"
#include "blackberry/renderer/texture.hpp"
#include "blackberry/application/application.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/gl.h"

namespace Blackberry {

    static std::array<f32, 108> s_CubeVertices = {{
       -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
       -0.5f,  0.5f, -0.5f,
       -0.5f, -0.5f, -0.5f,

       -0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
       -0.5f,  0.5f,  0.5f,
       -0.5f, -0.5f,  0.5f,

       -0.5f,  0.5f,  0.5f,
       -0.5f,  0.5f, -0.5f,
       -0.5f, -0.5f, -0.5f,
       -0.5f, -0.5f, -0.5f,
       -0.5f, -0.5f,  0.5f,
       -0.5f,  0.5f,  0.5f,

        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,

       -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
       -0.5f, -0.5f,  0.5f,
       -0.5f, -0.5f, -0.5f,

       -0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
       -0.5f,  0.5f,  0.5f,
       -0.5f,  0.5f, -0.5f,
    }};

    static std::array<u32, 36> s_CubeIndices = {{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 }};

    // horrible mess of a function, really hope to clean it up soon
    Ref<Texture2D> GenerateSkybox(const FS::Path& hdr) {
        auto& renderer = BL_APP.GetRenderer();

        Ref<Texture2D> rawHDR = Texture2D::Create(hdr, TextureFormat::RGB16F);

        Shader skyboxGenerateShader = Shader::Create(FS::Path("Assets/Shaders/Default/GenerateSkybox.vert"), FS::Path("Assets/Shaders/Default/GenerateSkybox.frag"));

        RenderTextureSpecification spec;
        spec.Size = BlVec2<u32>(512);
        spec.Attachments = {
            {0, RenderTextureAttachmentType::ColorRGBA16F},
            {1, RenderTextureAttachmentType::Depth24}
        };
        spec.ActiveAttachments = { 0 };

        Ref<RenderTexture> skyboxRenderTexture = RenderTexture::Create(spec);

        // Setup cubemap
        u32 envCubemap = 0;
        glGenTextures(1, &envCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        for (u32 i = 0; i < 6; i++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Setup projection and view matrices
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        // Render to cube map
        {
            renderer.BindRenderTexture(skyboxRenderTexture);
            renderer.Clear(BlColor(0, 0, 0, 255));

            // Skybox creation
            DrawBuffer skyboxBuffer;
            skyboxBuffer.Vertices = s_CubeVertices.data();
            skyboxBuffer.VertexSize = sizeof(f32) * 3;
            skyboxBuffer.VertexCount = 36;

            skyboxBuffer.Indices = s_CubeIndices.data();
            skyboxBuffer.IndexSize = sizeof(u32);
            skyboxBuffer.IndexCount = 36;

            renderer.SubmitDrawBuffer(skyboxBuffer);
            renderer.SetBufferLayout({
                { 0, ShaderDataType::Float3, "Position" }  
            });

            renderer.BindShader(skyboxGenerateShader);

            renderer.BindTexture(rawHDR, 0);
            skyboxGenerateShader.SetMatrix("u_Projection", glm::value_ptr(captureProjection));

            for (u32 i = 0; i < 6; i++) {
                skyboxGenerateShader.SetMatrix("u_View", glm::value_ptr(captureViews[i]));
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                renderer.DrawIndexed(36);
            }
            
            renderer.UnBindRenderTexture();

            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }

        Ref<Texture2D> skybox = CreateRef<Texture2D>();
        skybox->ID = envCubemap;
        skybox->BindlessHandle = glGetTextureHandleARB(envCubemap);

        return skybox;
    }

} // namespace Blackberry