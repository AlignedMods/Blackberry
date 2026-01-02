#include "blackberry/scene/scene_renderer.hpp"
#include "blackberry/scene/scene.hpp"
#include "blackberry/project/project.hpp"
#include "blackberry/core/timer.hpp"
#include "blackberry/renderer/debug_renderer.hpp"

#include "glad/gl.h"

namespace Blackberry {

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

    SceneRenderer::SceneRenderer(Scene* scene) {
        m_Context = scene;

        Ref<VertexBuffer> vbo = VertexBuffer::Create(BufferUsage::Dynamic);
        Ref<IndexBuffer> ibo = IndexBuffer::Create(BufferUsage::Dynamic);
        m_State.GeometryBuffer = VertexArray::Create();
        m_State.GeometryBuffer->SetVertexBuffer(vbo);
        m_State.GeometryBuffer->SetIndexBuffer(ibo);
        m_State.GeometryBuffer->SetVertexLayout({
           {0, ShaderDataType::Float3, "Position"},
           {1, ShaderDataType::Float3, "Normal"},
           {2, ShaderDataType::Float2, "TexCoord"}
        });

        m_State.MeshGeometryShader = Shader::Create(FS::Path("Assets/Shaders/Default/GeometryPass.vert"), FS::Path("Assets/Shaders/Default/GeometryPass.frag"));
        m_State.MeshLightingShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/LightingPass.frag"));
        m_State.SkyboxShader = Shader::Create(FS::Path("Assets/Shaders/Default/Skybox.vert"), FS::Path("Assets/Shaders/Default/Skybox.frag"));

        m_State.BloomExtractBrightAreasShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/Bloom/ExtractBrightAreas.frag"));
        m_State.BloomDownscaleShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/Bloom/Downscale.frag"));
        m_State.BloomUpscaleShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/Bloom/Upscale.frag"));
        m_State.BloomCombineShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/Bloom/Combine.frag"));

        m_State.ToneMapShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/Core/ToneMap.frag"));
        // m_State.FontShader = Shader::Create(s_VertexShaderFontSource, s_FragmentShaderFontSource);

        m_State.DefaultEnvironmentMap = CreateRef<EnvironmentMap>();
        m_State.DefaultEnvironmentMap->Prefilter = CreateRef<Texture>();
        m_State.DefaultEnvironmentMap->Irradiance = CreateRef<Texture>();
        m_State.DefaultEnvironmentMap->BrdfLUT = CreateRef<Texture>();
        m_State.CurrentEnvironmentMap = m_State.DefaultEnvironmentMap;

        m_State.InstanceDataBuffer = ShaderStorageBuffer::Create(0);
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
                {0, FramebufferAttachmentType::ColorRGBA16F},
                {1, FramebufferAttachmentType::Depth}
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
            
            auto envView = scene->m_ECS->GetEntitiesWithComponents<EnvironmentComponent>();
            
            envView.each([&](EnvironmentComponent& env) {
                AddEnvironment(env);
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

    void SceneRenderer::AddMesh(const TransformComponent& transform, const Mesh& mesh, const Material& mat, BlColor color, u32 entityID, u64 meshHandle) {
        auto& meshInstance = m_State.Meshes[meshHandle];

        BlVec4 normColor = NormalizeColor(color);

        // vertices
        for (u32 i = 0; i < mesh.Positions.size(); i++) {
            SceneMeshVertex vert = SceneMeshVertex(mesh.Positions[i], mesh.Normals[i], mesh.TexCoords[i]);
            meshInstance.MeshVertices.push_back(vert);
        }

        // indices
        for (u32 i = 0; i < mesh.Indices.size(); i++) {
            meshInstance.MeshIndices.push_back(mesh.Indices[i]);
        }

        TransformComponent transformMat = m_Context->GetEntityTransform(static_cast<EntityID>(entityID));
        BlMat4 final = transformMat.GetMatrix() * mesh.Transform;

        GPUInstanceData data;
        data.Transform = final;
        data.MaterialIndex = meshInstance.MaterialData.size() - 1;
        data.EntityID = entityID;

        meshInstance.InstanceData.push_back(data);

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

        meshInstance.MaterialData.push_back(gpuMat);

        meshInstance.InstanceCount++;
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

                        AddMesh(transform, trueModel.Meshes[i], material, color, entityID, model.MeshHandle);

                        useDefaultMaterial = false;
                    }
                }

                if (useDefaultMaterial && trueModel.Meshes[i].HasMeshMaterial) {
                    auto& material = trueModel.Meshes[i].MeshMaterial;
                    AddMesh(transform, trueModel.Meshes[i], material, color, entityID, model.MeshHandle); 
                } else if (useDefaultMaterial && !trueModel.Meshes[i].HasMeshMaterial) {
                    auto& material = DEFAULT_MATERIAL;
                    AddMesh(transform, trueModel.Meshes[i], material, color, entityID, model.MeshHandle); 
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

    void SceneRenderer::AddEnvironment(const EnvironmentComponent& env) {
        if (!Project::GetAssetManager().ContainsAsset(env.EnvironmentMap)) return;

        Asset& a = Project::GetAssetManager().GetAsset(env.EnvironmentMap);

        m_State.CurrentEnvironmentMap = std::get<Ref<EnvironmentMap>>(a.Data);
        m_State.EnvironmentMapLOD = env.LevelOfDetail;
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

        auto& api = BL_APP.GetRendererAPI();

        api.BindShader(m_State.MeshGeometryShader);
        m_State.MeshGeometryShader->SetMatrix("u_ViewProjection", m_Camera.GetCameraMatrixFloat());

        api.BindFramebuffer(m_State.GBuffer);
        api.ClearFramebuffer();
        m_State.GBuffer->ClearAttachmentFloat(4, -1.0f);

        api.EnableCapability(RendererCapability::DepthTest);
        api.EnableCapability(RendererCapability::FaceCull);
        api.SetDepthFunc(DepthFunc::Lequal);

        for (auto& [handle, instance] : m_State.Meshes) {
            m_State.GeometryBuffer->GetVertexBuffer()->UpdateData(instance.MeshVertices.data(), sizeof(SceneMeshVertex), instance.MeshVertices.size());
            m_State.GeometryBuffer->GetIndexBuffer()->UpdateData(instance.MeshIndices.data(), sizeof(u32), instance.MeshIndices.size());

            {
                BL_PROFILE_SCOPE("SceneRenderer::Flush/Passing instance data");
                m_State.InstanceDataBuffer.ReserveMemory(sizeof(GPUInstanceData) * instance.InstanceData.size(), instance.InstanceData.data());
            }

            {
                BL_PROFILE_SCOPE("SceneRenderer::Flush/Passing materials");
                m_State.MaterialBuffer.ReserveMemory(sizeof(GPUMaterial) * instance.MaterialData.size(), instance.MaterialData.data());
            }

            api.DrawVertexArrayInstanced(m_State.GeometryBuffer, instance.InstanceCount);

            instance.InstanceCount = 0;
            instance.InstanceData.clear();
            instance.MaterialData.clear();
            instance.MeshIndices.clear();
            instance.MeshVertices.clear();
        }

        api.UnBindFramebuffer();
    }

    void SceneRenderer::LightingPass() {
        BL_PROFILE_SCOPE("SceneRenderer::LightingPass");

        auto& api = BL_APP.GetRendererAPI();

        api.BindFramebuffer(m_State.PBROutput);
        api.ClearFramebuffer();

        api.BindShader(m_State.MeshLightingShader);
        
        m_State.MeshLightingShader->SetInt("u_GPosition", 0);
        m_State.MeshLightingShader->SetInt("u_GNormal", 1);
        m_State.MeshLightingShader->SetInt("u_GAlbedo", 2);
        m_State.MeshLightingShader->SetInt("u_GMat", 3);
        
        api.BindTexture2D(m_State.GBuffer->Attachments[0], 0);
        api.BindTexture2D(m_State.GBuffer->Attachments[1], 1);
        api.BindTexture2D(m_State.GBuffer->Attachments[2], 2);
        api.BindTexture2D(m_State.GBuffer->Attachments[3], 3);
        
        m_State.MeshLightingShader->SetVec3("u_ViewPos", m_Camera.Transform.Position);
        
        // set lights
        m_State.MeshLightingShader->SetVec4("u_DirectionalLight.Direction", m_State.DirectionalLight.Direction);
        m_State.MeshLightingShader->SetVec4("u_DirectionalLight.Color", m_State.DirectionalLight.Color);
        m_State.MeshLightingShader->SetVec4("u_DirectionalLight.Params", m_State.DirectionalLight.Params);
        
        m_State.PointLightBuffer.ReserveMemory(sizeof(GPUPointLight) * m_State.PointLights.size(), m_State.PointLights.data());
        m_State.SpotLightBuffer.ReserveMemory(sizeof(GPUSpotLight) * m_State.SpotLights.size(), m_State.SpotLights.data());
        m_State.MeshLightingShader->SetInt("u_PointLightCount", m_State.PointLights.size());
        m_State.MeshLightingShader->SetInt("u_SpotLightCount", m_State.SpotLights.size());
                                  
        m_State.MeshLightingShader->SetInt("u_IrradianceMap", 4);
        m_State.MeshLightingShader->SetInt("u_PrefilterMap", 5);
        m_State.MeshLightingShader->SetInt("u_BrdfLUT", 6);
        
        if (m_State.CurrentEnvironmentMap) {
            api.BindTextureCubemap(m_State.CurrentEnvironmentMap->Irradiance, 4);
            api.BindTextureCubemap(m_State.CurrentEnvironmentMap->Prefilter, 5);
            api.BindTexture2D(m_State.CurrentEnvironmentMap->BrdfLUT, 6);
        
            m_State.MeshLightingShader->SetVec3("u_FogColor", m_State.EnvironmentFogColor);
            m_State.MeshLightingShader->SetFloat("u_FogDistance", m_State.EnvironmentFogDistance);
        } else {
            api.BindTextureCubemap(m_State.DefaultEnvironmentMap->Irradiance, 4);
            api.BindTextureCubemap(m_State.DefaultEnvironmentMap->Prefilter, 5);
            api.BindTexture2D(m_State.DefaultEnvironmentMap->BrdfLUT, 6);
        
            m_State.MeshLightingShader->SetVec3("u_FogColor", m_State.EnvironmentFogColor);
            m_State.MeshLightingShader->SetFloat("u_FogDistance", m_State.EnvironmentFogDistance);
        }

        api.DrawVertexArray(DebugRenderer::GetQuadVAO());
        
        api.UnBindTextureCubemap();
        
        // Copy depth from geometry pass
        m_State.GBuffer->BlitDepthBuffer(m_State.PBROutput);
        
        api.SetDepthMask(false);
        api.DisableCapability(RendererCapability::FaceCull);

        api.BindShader(m_State.SkyboxShader);
        
        glm::mat4 projection = m_Camera.GetCameraProjection();
        glm::mat4 view = m_Camera.GetCameraView();
        
        m_State.SkyboxShader->SetMatrix("u_Projection", glm::value_ptr(projection));
        m_State.SkyboxShader->SetMatrix("u_View", glm::value_ptr(view));
        
        if (m_State.CurrentEnvironmentMap) {
            api.BindTextureCubemap(m_State.CurrentEnvironmentMap->Prefilter, 0);
            m_State.SkyboxShader->SetFloat("u_LOD", m_State.EnvironmentMapLOD);
        } else {
            api.BindTextureCubemap(m_State.DefaultEnvironmentMap->Skybox, 0);
            m_State.SkyboxShader->SetFloat("u_LOD", 0.0f);
        }
        
        api.DrawVertexArray(DebugRenderer::GetCubeVAO());

        api.SetDepthMask(true);

        api.UnBindFramebuffer();
    }

    void SceneRenderer::BloomPass() {
        auto& api = BL_APP.GetRendererAPI();

        {
            BL_PROFILE_SCOPE("SceneRenderer::BloomPass/BrightAreas");

            api.BindFramebuffer(m_State.BloomBrightAreas);
            api.ClearFramebuffer();
        
            api.BindShader(m_State.BloomExtractBrightAreasShader);
        
            m_State.BloomExtractBrightAreasShader->SetFloat("u_Threshold", m_State.BloomThreshold);
            api.BindTexture2D(m_State.PBROutput->Attachments.at(0), 0);
            
            api.DrawVertexArray(DebugRenderer::GetQuadVAO());
        
            api.UnBindTexture2D();
        
            api.UnBindFramebuffer();
        }
        
        {
            BL_PROFILE_SCOPE("SceneRenderer::BloomPass/Downscale");
        
            api.BindShader(m_State.BloomDownscaleShader);
            m_State.BloomDownscaleShader->SetVec2("u_TexResolution", BlVec2(1920, 1080));
            m_State.BloomDownscaleShader->SetInt("u_CurrentMip", 0);
        
            api.BindTexture2D(m_State.BloomBrightAreas->Attachments[0], 0);
        
            for (u32 i = 0; i < m_State.BloomDownscalePasses.size(); i++) {
                api.BindFramebuffer(m_State.BloomDownscalePasses[i]);
                api.ClearFramebuffer();
        
                api.DrawVertexArray(DebugRenderer::GetQuadVAO());
        
                api.BindTexture2D(m_State.BloomDownscalePasses[i]->Attachments[0], 0);
                m_State.BloomDownscaleShader->SetVec2("u_TexResolution", BlVec2(m_State.BloomDownscalePasses[i]->Attachments[0]->Width, m_State.BloomDownscalePasses[i]->Attachments[0]->Height));
                if (i == 0) m_State.BloomDownscaleShader->SetInt("u_CurrentMip", 1);
            }
        }
        
        {
            BL_PROFILE_SCOPE("SceneRenderer::BloomPass/Upscale");
        
            api.BindShader(m_State.BloomUpscaleShader);

            m_State.BloomUpscaleShader->SetInt("u_Texture0", 0);
            m_State.BloomUpscaleShader->SetInt("u_Texture1", 1);

            m_State.BloomUpscaleShader->SetFloat("u_FilterRadius", 0.005f);

            for (u32 i = m_State.BloomUpscalePasses.size(); i > 0; i--) {
                u32 mip = i - 1; // Goofy hack i don't want to explain

                api.BindFramebuffer(m_State.BloomUpscalePasses[mip]);
                api.ClearFramebuffer();

                if (mip == m_State.BloomUpscalePasses.size() - 1) { // This is the first pass
                    api.BindTexture2D(m_State.BloomDownscalePasses[i]->Attachments[0], 0);
                    api.BindTexture2D(m_State.BloomDownscalePasses[i - 1]->Attachments[0], 1);
                } else if (mip == 0) { // This is the last pass
                    api.BindTexture2D(m_State.BloomUpscalePasses[mip + 1]->Attachments[0], 0);
                    api.BindTexture2D(m_State.BloomBrightAreas->Attachments[0], 1);
                } else { // This is any other pass
                    api.BindTexture2D(m_State.BloomUpscalePasses[mip + 1]->Attachments[0], 0);
                    api.BindTexture2D(m_State.BloomDownscalePasses[mip]->Attachments[0], 1);
                }
        
                api.DrawVertexArray(DebugRenderer::GetQuadVAO());
            }
        
            api.UnBindFramebuffer();
        }
        
        {
            BL_PROFILE_SCOPE("SceneRenderer::BloomPass/CombinePBR");
        
            api.BindShader(m_State.BloomCombineShader);
            m_State.BloomCombineShader->SetInt("u_Original", 0);
            m_State.BloomCombineShader->SetInt("u_Blurred", 1);
        
            m_State.BloomCombineShader->SetFloat("u_CombineAmount", 0.5f);
            m_State.BloomCombineShader->SetInt("u_Mode", 0);
        
            api.BindTexture2D(m_State.PBROutput->Attachments[0], 0);
            api.BindTexture2D(m_State.BloomUpscalePasses[0]->Attachments[0], 1);
        
            api.BindFramebuffer(m_State.BloomCombinePass);
            api.ClearFramebuffer();
        
            api.DrawVertexArray(DebugRenderer::GetQuadVAO());
        
            api.UnBindFramebuffer();
        }
        
        {
            BL_PROFILE_SCOPE("SceneRenderer::BloomPass/Tonemapping");
        
            api.BindShader(m_State.ToneMapShader);
            
            api.BindTexture2D(m_State.BloomCombinePass->Attachments[0], 0);
            
            api.BindFramebuffer(m_RenderTarget);
            api.ClearFramebuffer();
            
            api.DrawVertexArray(DebugRenderer::GetQuadVAO());
            
            api.UnBindFramebuffer();
        }
    }

    u32 SceneRenderer::GetMaterialIndex(const Material& mat) {
        if (mat.ID == 0) return 0; // default material

        // u32 matIndex = 0;
        // 
        // if (m_State.MaterialIndex >= 16) {
        //     Flush();
        // }
        // 
        // bool texAlreadyExists = false;

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

        return 0;
    }

    void SceneRenderer::ResetState() {
        m_State.PointLights.clear();
        m_State.SpotLights.clear();

        m_State.CurrentEnvironmentMap = m_State.DefaultEnvironmentMap;
    }

    SceneRendererState& SceneRenderer::GetState() {
        return m_State;
    }

} // namespace Blackberry