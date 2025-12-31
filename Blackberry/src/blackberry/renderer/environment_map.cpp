#include "blackberry/renderer/environment_map.hpp"
#include "blackberry/renderer/shader.hpp"
#include "blackberry/renderer/texture.hpp"
#include "blackberry/application/application.hpp"
#include "blackberry/renderer/debug_renderer.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/gl.h"
#include "stb_image.h"

namespace Blackberry {

    static void RenderCube() {
        auto& api = BL_APP.GetRendererAPI();
        
        api.DrawVertexArray(DebugRenderer::GetCubeVAO());
    }

    static void RenderQuad() {
        auto& api = BL_APP.GetRendererAPI();
        
        api.DrawVertexArray(DebugRenderer::GetQuadVAO());
    }

    static Ref<Framebuffer> SetupFramebuffer() {
        FramebufferSpecification spec;
        spec.Width = 1024;
        spec.Height = 1024;
        spec.Attachments = {
            {0, FramebufferAttachmentType::Depth24}
        };
        spec.ActiveAttachments = {0};

        Ref<Framebuffer> framebuffer = Framebuffer::Create(spec);
        
        return framebuffer;
    }

    static Ref<Texture> LoadHDR(const FS::Path& path) {
        Ref<Texture> tex = Texture2D::Create(path, TextureFormat::RGB16F);

        return tex;
    }

    // The great rewrite of "EnvironmentMap::Create"
    Ref<EnvironmentMap> EnvironmentMap::Create(const FS::Path& hdr) {
        auto& api = BL_APP.GetRendererAPI();

        // Setup shaders
        Ref<Shader> equirectangularToCubemapShader = Shader::Create(FS::Path("Assets/Shaders/Default/GenerateSkybox.vert"), FS::Path("Assets/Shaders/Default/GenerateSkybox.frag"));
        Ref<Shader> irradianceShader = Shader::Create(FS::Path("Assets/Shaders/Default/GenerateSkybox.vert"), FS::Path("Assets/Shaders/Default/GenerateIrradiance.frag"));
        Ref<Shader> prefilterShader = Shader::Create(FS::Path("Assets/Shaders/Default/GenerateSkybox.vert"), FS::Path("Assets/Shaders/Default/GeneratePrefilter.frag"));
        Ref<Shader> brdfShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/GenerateBrdf.frag"));
        
        api.EnableCapability(RendererCapability::SeamlessCubemap);

        // Setup framebuffer
        Ref<Framebuffer> captureFramebuffer = SetupFramebuffer();
        // Load HDR image
        Ref<Texture> rawHDR = LoadHDR(hdr);
        Ref<Texture> envCubemap = TextureCubemap::Create(1024, 1024, TextureFormat::RGB16F);
        Ref<Texture> envIrradiance = TextureCubemap::Create(32, 32, TextureFormat::RGB16F);
        Ref<Texture> envPrefilter = TextureCubemap::Create(1024, 1024, TextureFormat::RGB16F);
        Ref<Texture> brdfLut = Texture2D::Create(nullptr, 512, 512, TextureFormat::RG16F);
        
        // Capture the skybox
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };
        
        api.BindShader(equirectangularToCubemapShader);
        equirectangularToCubemapShader->SetInt("u_EquirectangularMap", 0);
        equirectangularToCubemapShader->SetMatrix("u_Projection", glm::value_ptr(captureProjection));
        api.BindTexture2D(rawHDR, 0);

        api.BindFramebuffer(captureFramebuffer);
        api.ClearFramebuffer();

        constexpr u32 MAX_MIP_LEVELS = 8;

        for (u32 mip = 0; mip < MAX_MIP_LEVELS; mip++) {
            u32 mipWidth  = static_cast<u32>(1024 * std::pow(0.5, mip));
            u32 mipHeight = static_cast<u32>(1024 * std::pow(0.5, mip)); 

            // Okay so this look may be confusing (it was to me at first so this is more of a future reference)
            // All we are doing here is *setting* the color attachment in the framebuffer to the texture we need it to be, that is all
            // Nothing special, just setting an attachment
            for (u32 i = 0; i < 6; ++i) {
                equirectangularToCubemapShader->SetMatrix("u_View", glm::value_ptr(captureViews[i]));
                captureFramebuffer->AttachColorAttachmentCubemap(0, envCubemap, i, mip);
                api.ClearFramebuffer();
                api.SetViewportSize(BlVec2(mipWidth, mipHeight));

                equirectangularToCubemapShader->SetFloat("u_CurrentMip", static_cast<f32>(mip));
            
                RenderCube();
            }
        }
        
        api.UnBindFramebuffer();

        // Create an irradiance cubemap
        captureFramebuffer->Resize(32, 32);
        api.BindFramebuffer(captureFramebuffer);

        api.BindShader(irradianceShader);
        irradianceShader->SetInt("u_EnvironmentMap", 0);
        irradianceShader->SetMatrix("u_Projection", glm::value_ptr(captureProjection));
        api.BindTextureCubemap(envCubemap);

        for (u32 i = 0; i < 6; i++) {
            irradianceShader->SetMatrix("u_View", glm::value_ptr(captureViews[i]));

            captureFramebuffer->AttachColorAttachmentCubemap(0, envIrradiance, i, 0);
            api.ClearFramebuffer();

            RenderCube();
        }

        api.UnBindFramebuffer();

        // Create an prefilter cubemap
        captureFramebuffer->Resize(1024, 1024);
        api.BindFramebuffer(captureFramebuffer);

        api.BindShader(prefilterShader);
        prefilterShader->SetInt("u_EnvironmentMap", 0);
        prefilterShader->SetMatrix("u_Projection", glm::value_ptr(captureProjection));
        api.BindTextureCubemap(envCubemap);

        for (u32 mip = 0; mip < MAX_MIP_LEVELS; mip++) {
            u32 mipWidth  = static_cast<u32>(1024 * std::pow(0.5, mip));
            u32 mipHeight = static_cast<u32>(1024 * std::pow(0.5, mip)); 

            f32 roughness = static_cast<f32>(mip) / static_cast<f32>(MAX_MIP_LEVELS - 1);
            prefilterShader->SetFloat("u_Roughness", roughness);

            api.SetViewportSize(BlVec2(mipWidth, mipHeight));

            for (u32 i = 0; i < 6; i++) {
                prefilterShader->SetMatrix("u_View", glm::value_ptr(captureViews[i]));

                captureFramebuffer->AttachColorAttachmentCubemap(0, envPrefilter, i, mip);
                api.ClearFramebuffer();

                RenderCube();
            } 
        }

        api.UnBindFramebuffer();

        captureFramebuffer->Resize(512, 512);
        api.BindFramebuffer(captureFramebuffer);
        captureFramebuffer->AttachColorAttachment(0, brdfLut, 0);
        api.ClearFramebuffer();

        api.BindShader(brdfShader);
        RenderQuad();

        api.UnBindFramebuffer();
        
        Ref<EnvironmentMap> env = CreateRef<EnvironmentMap>();
        env->Skybox = envCubemap;
        env->Irradiance = envIrradiance;
        env->Prefilter = envPrefilter;
        env->BrdfLUT = brdfLut;
        
        return env;
    }

} // namespace Blackberry