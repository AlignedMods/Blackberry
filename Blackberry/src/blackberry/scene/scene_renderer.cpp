#include "blackberry/scene/scene_renderer.hpp"
#include "blackberry/scene/scene.hpp"
#include "blackberry/project/project.hpp"
#include "blackberry/core/timer.hpp"

#include "glad/gl.h"

namespace Blackberry {

#pragma region ShaderCode

    static const char* s_VertexShaderFontSource = BL_STR(
        \x23version 460 core\n
        \x23 extension GL_ARB_bindless_texture : enable\n

        layout (location = 0) in vec3 a_Pos;
        layout (location = 1) in vec4 a_Color;
        layout (location = 2) in vec2 a_TexCoord;

        uniform mat4 u_Projection;

        layout (location = 0) out vec4 o_Color;
        layout (location = 1) out vec2 o_TexCoord;

        void main() {
            gl_Position = u_Projection * vec4(a_Pos, 1.0f);

            o_Color = a_Color;
            o_TexCoord = a_TexCoord;
        }
    );

    static const char* s_FragmentShaderFontSource = BL_STR(
        \x23version 460 core\n
        \x23 extension GL_ARB_bindless_texture : enable\n

        layout (location = 0) in vec4 a_Color;
        layout (location = 1) in vec2 a_TexCoord;

        uniform sampler2D u_FontAtlas;

        out vec4 o_FragColor;

        float screenPxRange() {
            const float pxRange = 2.0;
            vec2 unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlas, 0));
            vec2 screenTexSize = vec2(1.0) / fwidth(a_TexCoord);
            return max(0.5*dot(unitRange, screenTexSize), 1.0);
        }

        float median(float r, float g, float b) {
            return max(min(r, g), min(max(r, g), b));
        }

        void main() {
            vec4 texelColor = texture(u_FontAtlas, a_TexCoord) * a_Color;

            // msdf thing
            vec3 msd = texture(u_FontAtlas, a_TexCoord).rgb;
            float sd = median(msd.r, msd.g, msd.b);
            float screenPxDistance = screenPxRange() * (sd - 0.5);
            float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
            if (opacity == 0.0) { discard; }

            vec4 bgColor = vec4(0.0);
            o_FragColor = mix(bgColor, a_Color, opacity);
        }
    );

#pragma endregion

    constexpr u32 MAX_OBJECTS = 2048;
    constexpr u32 MAX_MATERIALS = 2048;

    static const Material DEFAULT_MATERIAL = Material::Create();

    static BlVec4 NormalizeColor(BlColor color) {
        return BlVec4(
            static_cast<f32>(color.r) / 255.0f,
            static_cast<f32>(color.g) / 255.0f,
            static_cast<f32>(color.b) / 255.0f,
            static_cast<f32>(color.a) / 255.0f
        );
    }

    SceneRenderer::SceneRenderer() {
        m_State.MeshGeometryShader = Shader::Create(FS::Path("Assets/Shaders/Default/GeometryPass.vert"), FS::Path("Assets/Shaders/Default/GeometryPass.frag"));
        m_State.MeshLightingShader = Shader::Create(FS::Path("Assets/Shaders/Default/LightingPass.vert"), FS::Path("Assets/Shaders/Default/LightingPass.frag"));
        m_State.SkyboxShader = Shader::Create(FS::Path("Assets/Shaders/Default/Skybox.vert"), FS::Path("Assets/Shaders/Default/Skybox.frag"));

        m_State.BloomExtractBrightAreasShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/Bloom/ExtractBrightAreas.frag"));
        m_State.BloomDownscaleShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/Bloom/Downscale.frag"));
        m_State.BloomUpscaleShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/Bloom/Upscale.frag"));
        m_State.BloomCombineShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/Bloom/Combine.frag"));

        m_State.ToneMapShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/Core/ToneMap.frag"));
        // m_State.FontShader = Shader::Create(s_VertexShaderFontSource, s_FragmentShaderFontSource);

        // m_State.EnviromentMap = EnviromentMap::Create("Assets/Textures/Skybox.hdr");
        m_State.DefaultEnviromentMap = CreateRef<EnviromentMap>();
        m_State.DefaultEnviromentMap->Prefilter = CreateRef<Texture2D>();
        m_State.DefaultEnviromentMap->Irradiance = CreateRef<Texture2D>();
        m_State.DefaultEnviromentMap->BrdfLUT = CreateRef<Texture2D>();
        m_State.CurrentEnviromentMap = m_State.DefaultEnviromentMap;

        m_State.TransformBuffer = ShaderStorageBuffer::Create(0);
        m_State.MaterialBuffer = ShaderStorageBuffer::Create(1);
        m_State.ShaderGBuffer = ShaderStorageBuffer::Create(2);
        m_State.PointLightBuffer = ShaderStorageBuffer::Create(3);
        m_State.SpotLightBuffer = ShaderStorageBuffer::Create(4);

        {
            FramebufferSpecification spec;
            spec.Width = 1920;
            spec.Height = 1080;
            spec.Attachments = {
                {0, FramebufferAttachmentType::ColorRGBA16F}, // position color buffer
                {1, FramebufferAttachmentType::ColorRGBA16F}, // normal buffer
                {2, FramebufferAttachmentType::ColorRGBA8}, // color buffer
                {3, FramebufferAttachmentType::ColorRGBA16F}, // material buffer
                {4, FramebufferAttachmentType::ColorR32F}, // EntityID buffer
                {5, FramebufferAttachmentType::Depth} // depth
            };
            spec.ActiveAttachments = {0, 1, 2, 3, 4}; // which attachments we want to use for rendering

            m_State.GBuffer = Framebuffer::Create(spec);
        }
        
        {
            FramebufferSpecification spec;
            spec.Width = 1920;
            spec.Height = 1080;
            spec.Attachments = {
                {0, FramebufferAttachmentType::ColorRGBA16F}
            };
            spec.ActiveAttachments = {0};

            m_State.PBROutput = Framebuffer::Create(spec);
        }

        {
            FramebufferSpecification spec;
            spec.Width = 1920;
            spec.Height = 1080;
            spec.Attachments = {
                {0, FramebufferAttachmentType::ColorRGBA16F}
            };
            spec.ActiveAttachments = {0};

            m_State.BloomBrightAreas = Framebuffer::Create(spec);
        }

        // Create all the render targets (NOTE: We could use mips for this exact purpose)
        {
            FramebufferSpecification spec;
            spec.Width = 1920;
            spec.Height = 1080;
            spec.Attachments = {
                {0, FramebufferAttachmentType::ColorRGBA16F}
            };
            spec.ActiveAttachments = {0};

            for (u32 i = 0; i < m_State.BloomDownscalePasses.size(); i++) {
                m_State.BloomDownscalePasses[i] = Framebuffer::Create(spec);

                spec.Width *= 0.5f;
                spec.Height *= 0.5f;
            }
        }

        // Create all the upscale render targets
        {
            FramebufferSpecification spec;
            spec.Width = 1920;
            spec.Height = 1080;
            spec.Attachments = {
                {0, FramebufferAttachmentType::ColorRGBA16F}
            };
            spec.ActiveAttachments = {0};

            m_State.BloomCombinePass = Framebuffer::Create(spec);

            for (u32 i = 0; i < m_State.BloomUpscalePasses.size(); i++) {
                m_State.BloomUpscalePasses[i] = Framebuffer::Create(spec);

                spec.Width *= 0.5f;
                spec.Height *= 0.5f;
            }
        }
    }

    // SceneRenderer::~SceneRenderer() {}

    void SceneRenderer::Render(Scene* scene) {
        {
            BL_PROFILE_SCOPE("SceneRenderer::Render");

            auto dirLightView = scene->m_ECS->GetEntitiesWithComponents<TransformComponent, DirectionalLightComponent>();
            
            dirLightView.each([&](TransformComponent& transform, DirectionalLightComponent& light) {
                AddDirectionalLight(transform, light);    
            });
            
            auto lightView = scene->m_ECS->GetEntitiesWithComponents<TransformComponent, PointLightComponent>();
            
            lightView.each([&](TransformComponent& transform, PointLightComponent& light) {
                AddPointLight(transform, light);
            });
            
            auto spotLightView = scene->m_ECS->GetEntitiesWithComponents<TransformComponent, SpotLightComponent>();
            
            spotLightView.each([&](TransformComponent& transform, SpotLightComponent& light) {
                AddSpotLight(transform, light);
            });
            
            auto meshView = scene->m_ECS->GetEntitiesWithComponents<TransformComponent, MeshComponent>();
            
            meshView.each([&](entt::entity id, TransformComponent& transform, MeshComponent& mesh) {
                AddModel(transform, mesh, BlColor(255, 255, 255, 255), static_cast<u32>(id));
            });
            
            auto envView = scene->m_ECS->GetEntitiesWithComponents<EnviromentComponent>();
            
            envView.each([&](EnviromentComponent& env) {
                AddEnviroment(env);
            });
        }

        Flush();
    }

    void SceneRenderer::SetCamera(const SceneCamera& camera) {
        m_Camera = camera;
    }

    SceneCamera Blackberry::SceneRenderer::GetCamera() {
        return m_Camera;
    }

    void SceneRenderer::SetRenderTarget(Ref<Framebuffer> target) {
        m_RenderTarget = target;
    }

    void SceneRenderer::RenderEntity(Entity entity) {
        if (entity.HasComponent<TransformComponent>() && entity.HasComponent<MeshComponent>()) {
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& mesh = entity.GetComponent<MeshComponent>();

            AddModel(transform, mesh, BlColor(255, 255, 255, 255), static_cast<u32>(entity.ID));
        }
    }

    void SceneRenderer::AddMesh(const TransformComponent& transform, const Mesh& mesh, const Material& mat, BlColor color, u32 entityID) {
        BlVec4 normColor = NormalizeColor(color);

        // vertices
        for (u32 i = 0; i < mesh.Positions.size(); i++) {
            SceneMeshVertex vert = SceneMeshVertex(mesh.Positions[i], mesh.Normals[i], mesh.TexCoords[i], m_State.MaterialIndex, m_State.ObjectIndex, entityID);
            m_State.MeshVertices.push_back(vert);
        }

        // indices
        for (u32 i = 0; i < mesh.Indices.size(); i++) {
            const u32 vertexCount = m_State.MeshVertexCount;
            m_State.MeshIndices.push_back(mesh.Indices[i] + vertexCount);
        }

        // Add transform which will be sent to shader
        m_State.Transforms.push_back(transform.GetMatrix());

        GPUMaterial gpuMat;

        if (mat.ID != 0) {
            gpuMat.UseAlbedoTexture = mat.UseAlbedoTexture;
            gpuMat.AlbedoTexture = mat.AlbedoTexture->BindlessHandle;
            gpuMat.AlbedoColor = mat.AlbedoColor;
            
            gpuMat.UseMetallicTexture = mat.UseMetallicTexture;
            gpuMat.MetallicTexture = mat.MetallicTexture->BindlessHandle;
            gpuMat.MetallicFactor = mat.MetallicFactor;
            
            gpuMat.UseRoughnessTexture = mat.UseRoughnessTexture;
            gpuMat.RoughnessTexture = mat.RoughnessTexture->BindlessHandle;
            gpuMat.RoughnessFactor = mat.RoughnessFactor;
            
            gpuMat.UseAOTexture = mat.UseAOTexture;
            gpuMat.AOTexture = mat.AOTexture->BindlessHandle;
            gpuMat.AOFactor = mat.AOFactor;

            gpuMat.Emission = mat.Emission;
        }

        m_State.Materials.push_back(gpuMat);

        m_State.MeshVertexCount += mesh.Positions.size();
        m_State.MeshIndexCount += mesh.Indices.size();

        m_State.ObjectIndex++;
        m_State.MaterialIndex++;
    }

    void SceneRenderer::AddModel(const TransformComponent& transform, const MeshComponent& model, BlColor color, u32 entityID) {
        if (Project::GetAssetManager().ContainsAsset(model.MeshHandle)) {
            const Asset& asset = Project::GetAssetManager().GetAsset(model.MeshHandle);
            auto& trueModel = std::get<Model>(asset.Data);

            for (u32 i = 0; i < trueModel.MeshCount; ++i) {
                bool useDefaultMaterial = true;

                if (model.MaterialHandles.contains(i)) {
                    if (Project::GetAssetManager().ContainsAsset(model.MaterialHandles.at(i))) {
                        auto& asset = Project::GetAssetManager().GetAsset(model.MaterialHandles.at(i));
                        auto& material = std::get<Material>(Project::GetAssetManager().GetAsset(model.MaterialHandles.at(i)).Data);

                        AddMesh(transform, trueModel.Meshes[i], material, color, entityID);

                        useDefaultMaterial = false;
                    }
                }

                if (useDefaultMaterial && trueModel.Meshes[i].HasMeshMaterial) {
                    auto& material = trueModel.Meshes[i].MeshMaterial;
                    AddMesh(transform, trueModel.Meshes[i], material, color, entityID); 
                } else if (useDefaultMaterial && !trueModel.Meshes[i].HasMeshMaterial) {
                    auto& material = DEFAULT_MATERIAL;
                    AddMesh(transform, trueModel.Meshes[i], material, color, entityID); 
                }
            }
        }
    }

    void SceneRenderer::AddDirectionalLight(const TransformComponent& transform, const DirectionalLightComponent& light) {
        GPUDirectionalLight l;
        l.Direction = BlVec4(transform.Rotation.x, transform.Rotation.y, transform.Rotation.z, 0.0f);
        l.Color = BlVec4(light.Color.x, light.Color.y, light.Color.z, 0.0f);
        l.Params.x = light.Intensity;

        m_State.DirectionalLight = l;
    }

    void SceneRenderer::AddPointLight(const TransformComponent& transform, const PointLightComponent& light) {
        GPUPointLight l;
        l.Position = BlVec4(transform.Position.x, transform.Position.y, transform.Position.z, 0.0f);
        l.Color = BlVec4(light.Color.x, light.Color.y, light.Color.z, 0.0f);
        l.Params = BlVec4(light.Radius, light.Intensity, 0.0f, 0.0f);

        m_State.PointLights.push_back(l);
    }

    void SceneRenderer::AddSpotLight(const TransformComponent& transform, const SpotLightComponent& light) {
        GPUSpotLight l;
        l.Position = BlVec4(transform.Position.x, transform.Position.y, transform.Position.z, 0.0f);
        l.Direction = BlVec4(transform.Rotation.x, transform.Rotation.y, transform.Rotation.z, glm::cos(glm::radians(light.Cutoff)));
        l.Color = BlVec4(light.Color.x, light.Color.y, light.Color.z, light.Intensity);

        m_State.SpotLights.push_back(l);
    }

    void SceneRenderer::AddEnviroment(const EnviromentComponent& env) {
        if (!Project::GetAssetManager().ContainsAsset(env.EnviromentMap)) return;

        Asset& a = Project::GetAssetManager().GetAsset(env.EnviromentMap);

        m_State.CurrentEnviromentMap = std::get<Ref<EnviromentMap>>(a.Data);
        m_State.EnviromentMapLOD = env.LevelOfDetail;
        m_State.BloomEnabled = env.EnableBloom;
        m_State.BloomThreshold = env.BloomThreshold;
    }

    void SceneRenderer::Flush() {
        BL_PROFILE_SCOPE("SceneRenderer::Flush");

        GeometryPass();
        LightingPass();

        if (m_State.BloomEnabled) {
            BloomPass(); // NOTE: By doing this we are not allowing rendering to happen to the desired rendering target
        }

        ResetState();
    }

    void SceneRenderer::GeometryPass() {
        BL_PROFILE_SCOPE("SceneRenderer::GeometryPass");

        auto& renderer = BL_APP.GetRenderer();

        if (m_State.MeshIndices.size() > 0) {
            // Geometry pass
            DrawBuffer geometryBuffer;
            geometryBuffer.Vertices = m_State.MeshVertices.data();
            geometryBuffer.VertexSize = sizeof(SceneMeshVertex);
            geometryBuffer.VertexCount = m_State.MeshVertexCount;

            geometryBuffer.Indices = m_State.MeshIndices.data();
            geometryBuffer.IndexSize = sizeof(u32);
            geometryBuffer.IndexCount = m_State.MeshIndexCount;

            renderer.SubmitDrawBuffer(geometryBuffer);
            renderer.SetBufferLayout({
                {0, ShaderDataType::Float3, "Position"},
                {1, ShaderDataType::Float3, "Normal"},
                {2, ShaderDataType::Float2, "TexCoord"},
                {3, ShaderDataType::Int, "MaterialIndex"},
                {4, ShaderDataType::Int, "ObjectIndex"},
                {5, ShaderDataType::Int, "EntityID"}
            });

            renderer.BindShader(m_State.MeshGeometryShader);
            m_State.MeshGeometryShader->SetMatrix("u_ViewProjection", m_Camera.GetCameraMatrixFloat());

            {
                BL_PROFILE_SCOPE("SceneRenderer::Flush/Passing transforms");
                m_State.TransformBuffer.ReserveMemory(sizeof(glm::mat4) * m_State.Transforms.size(), m_State.Transforms.data());
            }

            {
                BL_PROFILE_SCOPE("SceneRenderer::Flush/Passing materials");
                m_State.MaterialBuffer.ReserveMemory(sizeof(GPUMaterial) * m_State.Materials.size(), m_State.Materials.data());
            }


            renderer.BindFramebuffer(m_State.GBuffer);
            renderer.Clear(BlColor(0, 0, 0, 255));

            m_State.GBuffer->ClearAttachmentFloat(4, -1.0f);

            renderer.DrawIndexed(m_State.MeshIndexCount);

            renderer.UnBindFramebuffer();
        }
    }

    void SceneRenderer::LightingPass() {
        BL_PROFILE_SCOPE("SceneRenderer::LightingPass");

        auto& renderer = BL_APP.GetRenderer();

        renderer.BindFramebuffer(m_State.PBROutput);
        renderer.Clear(BlColor(0, 0, 0, 255));
        
        DrawBuffer skyboxBuffer;
        skyboxBuffer.Vertices = m_State.CubeVertices.data();
        skyboxBuffer.VertexSize = sizeof(f32) * 3;
        skyboxBuffer.VertexCount = 36;

        skyboxBuffer.Indices = m_State.CubeIndices.data();
        skyboxBuffer.IndexSize = sizeof(u32);
        skyboxBuffer.IndexCount = 36;

        renderer.SubmitDrawBuffer(skyboxBuffer);
        renderer.SetBufferLayout({
            {0, ShaderDataType::Float3, "Position"},
        });

        glDepthMask(GL_FALSE);

        renderer.BindShader(m_State.SkyboxShader);
        
        glm::mat4 projection = m_Camera.GetCameraProjection();
        glm::mat4 view = m_Camera.GetCameraView();
        
        m_State.SkyboxShader->SetMatrix("u_Projection", glm::value_ptr(projection));
        m_State.SkyboxShader->SetMatrix("u_View", glm::value_ptr(view));
        
        if (m_State.CurrentEnviromentMap) {
            renderer.BindCubemap(m_State.CurrentEnviromentMap->Prefilter, 0);
            m_State.SkyboxShader->SetFloat("u_LOD", m_State.EnviromentMapLOD);
        } else {
            renderer.BindCubemap(m_State.DefaultEnviromentMap->Prefilter, 0);
            m_State.SkyboxShader->SetFloat("u_LOD", 0.0f);
        }
        
        renderer.DrawIndexed(36);

        glDepthMask(GL_TRUE);

        DrawBuffer lightingBuffer;
        lightingBuffer.Vertices = m_State.QuadVertices.data();
        lightingBuffer.VertexSize = sizeof(f32) * 4;
        lightingBuffer.VertexCount = 6;

        lightingBuffer.Indices = m_State.QuadIndices.data();
        lightingBuffer.IndexSize = sizeof(u32);
        lightingBuffer.IndexCount = 6;

        renderer.SubmitDrawBuffer(lightingBuffer);
        renderer.SetBufferLayout({
            {0, ShaderDataType::Float2, "Position"},
            {1, ShaderDataType::Float2, "TexCoord"}
        });

        renderer.BindShader(m_State.MeshLightingShader);
        m_State.MeshLightingShader->SetVec3("u_ViewPos", m_Camera.Transform.Position);

        u64 handles[5]{};
        for (u32 i = 0; i < 5; i++) {
            handles[i] = m_State.GBuffer->Attachments[i]->BindlessHandle;
        }
        m_State.ShaderGBuffer.ReserveMemory(sizeof(u64) * 5, handles);

        // set lights
        m_State.MeshLightingShader->SetVec4("u_DirectionalLight.Direction", m_State.DirectionalLight.Direction);
        m_State.MeshLightingShader->SetVec4("u_DirectionalLight.Color", m_State.DirectionalLight.Color);
        m_State.MeshLightingShader->SetVec4("u_DirectionalLight.Params", m_State.DirectionalLight.Params);

        m_State.PointLightBuffer.ReserveMemory(sizeof(GPUPointLight) * m_State.PointLights.size(), m_State.PointLights.data());
        m_State.SpotLightBuffer.ReserveMemory(sizeof(GPUSpotLight) * m_State.SpotLights.size(), m_State.SpotLights.data());
        m_State.MeshLightingShader->SetInt("u_PointLightCount", m_State.PointLights.size());
        m_State.MeshLightingShader->SetInt("u_SpotLightCount", m_State.SpotLights.size());
                                  
        m_State.MeshLightingShader->SetInt("u_IrradianceMap", 0);
        m_State.MeshLightingShader->SetInt("u_PrefilterMap", 1);
        m_State.MeshLightingShader->SetInt("u_BrdfLUT", 2);

        if (m_State.CurrentEnviromentMap) {
            renderer.BindCubemap(m_State.CurrentEnviromentMap->Irradiance, 0);
            renderer.BindCubemap(m_State.CurrentEnviromentMap->Prefilter, 1);
            renderer.BindTexture(m_State.CurrentEnviromentMap->BrdfLUT, 2);
        
            m_State.MeshLightingShader->SetVec3("u_FogColor", m_State.EnviromentFogColor);
            m_State.MeshLightingShader->SetFloat("u_FogDistance", m_State.EnviromentFogDistance);
        } else {
            renderer.BindCubemap(m_State.DefaultEnviromentMap->Irradiance, 0);
            renderer.BindCubemap(m_State.DefaultEnviromentMap->Prefilter, 1);
            renderer.BindTexture(m_State.DefaultEnviromentMap->BrdfLUT, 2);

            m_State.MeshLightingShader->SetVec3("u_FogColor", m_State.EnviromentFogColor);
            m_State.MeshLightingShader->SetFloat("u_FogDistance", m_State.EnviromentFogDistance);
        }

        renderer.DrawIndexed(6);

        renderer.UnBindCubemap();

        renderer.UnBindFramebuffer();
    }

    void SceneRenderer::BloomPass() {
        auto& renderer = BL_APP.GetRenderer();

        {
            BL_PROFILE_SCOPE("SceneRenderer::BloomPass/BrightAreas");

            renderer.BindFramebuffer(m_State.BloomBrightAreas);
            renderer.Clear(BlColor(0, 0, 0, 255));

            renderer.BindShader(m_State.BloomExtractBrightAreasShader);

            m_State.BloomExtractBrightAreasShader->SetFloat("u_Threshold", m_State.BloomThreshold);
            renderer.BindTexture(m_State.PBROutput->Attachments.at(0), 0);

            // we can simply reuse the previous buffer (from the lighting pass)
            renderer.DrawIndexed(6);

            renderer.UnBindTexture();

            renderer.UnBindFramebuffer();
        }

        {
            BL_PROFILE_SCOPE("SceneRenderer::BloomPass/Downscale");

            renderer.BindShader(m_State.BloomDownscaleShader);
            m_State.BloomDownscaleShader->SetVec2("u_TexResolution", BlVec2(1920, 1080));

            renderer.BindTexture(m_State.BloomBrightAreas->Attachments[0], 0);

            for (u32 i = 0; i < m_State.BloomDownscalePasses.size(); i++) {
                renderer.BindFramebuffer(m_State.BloomDownscalePasses[i]);
                renderer.Clear(BlColor(0, 0, 0, 255));

                renderer.DrawIndexed(6);

                renderer.BindTexture(m_State.BloomDownscalePasses[i]->Attachments[0], 0);
                m_State.BloomDownscaleShader->SetVec2("u_TexResolution", BlVec2(m_State.BloomDownscalePasses[i]->Attachments[0]->Width, m_State.BloomDownscalePasses[i]->Attachments[0]->Height));
            }
        }

        {
            BL_PROFILE_SCOPE("SceneRenderer::BloomPass/Upscale");

            renderer.BindShader(m_State.BloomUpscaleShader);
            m_State.BloomUpscaleShader->SetFloat("u_FilterRadius", 0.005f);

            renderer.BindTexture(m_State.BloomDownscalePasses[5]->Attachments[0], 0);

            for (u32 i = 0; i < m_State.BloomUpscalePasses.size(); i++) {
                renderer.BindFramebuffer(m_State.BloomUpscalePasses[4 - i]);
                renderer.Clear(BlColor(0, 0, 0, 255));

                renderer.DrawIndexed(6);

                renderer.BindTexture(m_State.BloomUpscalePasses[4 - i]->Attachments[0], 0);
            }

            renderer.UnBindFramebuffer();
        }

        {
            BL_PROFILE_SCOPE("SceneRenderer::BloomPass/CombineUpscales");

            renderer.BindShader(m_State.BloomCombineShader);
            m_State.BloomCombineShader->SetInt("u_Original", 0);
            m_State.BloomCombineShader->SetInt("u_Blurred", 1);

            renderer.BindTexture(m_State.BloomUpscalePasses[3]->Attachments[0], 0);
            renderer.BindTexture(m_State.BloomUpscalePasses[4]->Attachments[0], 1);

            renderer.BindFramebuffer(m_State.BloomCombinePass);
            renderer.Clear(BlColor(0, 0, 0, 255));

            for (u32 i = 0; i < m_State.BloomUpscalePasses.size() - 1; i++) {
                renderer.DrawIndexed(6);

                renderer.BindTexture(m_State.BloomUpscalePasses[2 - 1]->Attachments[0], 0);
                renderer.BindTexture(m_State.BloomUpscalePasses[3 - 1]->Attachments[0], 1);
            }

            renderer.UnBindFramebuffer();
        }

        {
            BL_PROFILE_SCOPE("SceneRenderer::BloomPass/CombinePBR");
        
            renderer.BindShader(m_State.BloomCombineShader);
            m_State.BloomCombineShader->SetInt("u_Original", 0);
            m_State.BloomCombineShader->SetInt("u_Blurred", 1);
        
            renderer.BindTexture(m_State.BloomCombinePass->Attachments[0], 1);
            renderer.BindTexture(m_State.PBROutput->Attachments[0], 0);
        
            renderer.BindFramebuffer(m_State.BloomUpscalePasses[0]);
            renderer.Clear(BlColor(0, 0, 0, 255));
        
            renderer.DrawIndexed(6);
        
            renderer.UnBindFramebuffer();
        }

        {
            BL_PROFILE_SCOPE("SceneRenderer::BloomPass/Tonemapping");
        
            renderer.BindShader(m_State.ToneMapShader);
            
            renderer.BindTexture(m_State.BloomUpscalePasses[0]->Attachments[0], 0);
            
            renderer.BindFramebuffer(m_RenderTarget);
            renderer.Clear(BlColor(0, 0, 0, 255));
            
            renderer.DrawIndexed(6);
            
            renderer.UnBindFramebuffer();
        }
    }

    u32 SceneRenderer::GetMaterialIndex(const Material& mat) {
        if (mat.ID == 0) return 0; // default material

        u32 matIndex = 0;

        if (m_State.MaterialIndex >= 16) {
            Flush();
        }

        bool texAlreadyExists = false;

        // Loop through all of the current materials to see if we already have a
        // matching material
        // for (u32 i = 0; i < m_State.MaterialIndex; i++) {
        //     if (mat.ID == m_State.Materials[i].ID) {
        //         matIndex = i;
        //         texAlreadyExists = true;
        // 
        //         break;
        //     }
        // }
        // 
        // // If we haven't found a suitable material we create a new slot
        // if (!texAlreadyExists) {
        //     matIndex = m_State.MaterialIndex;
        // 
        //     m_State.Materials[m_State.MaterialIndex] = mat;
        //     m_State.MaterialIndex++;
        //     texAlreadyExists = true;
        // }

        return matIndex;
    }

    void SceneRenderer::ResetState() {
        m_State.MeshVertices.clear();
        m_State.MeshIndices.clear();

        m_State.MeshVertexCount = 0;
        m_State.MeshIndexCount = 0;

        m_State.MaterialIndex = 0;
        m_State.ObjectIndex = 0;
        m_State.Transforms.clear();
        m_State.Materials.clear();
        m_State.PointLights.clear();
        m_State.SpotLights.clear();

        m_State.CurrentEnviromentMap = m_State.DefaultEnviromentMap;
    }

    SceneRendererState& SceneRenderer::GetState() {
        return m_State;
    }

} // namespace Blackberry