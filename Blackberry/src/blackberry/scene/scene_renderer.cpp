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

    static BlVec4<f32> NormalizeColor(BlColor color) {
        return BlVec4<f32>(
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
        // m_State.FontShader = Shader::Create(s_VertexShaderFontSource, s_FragmentShaderFontSource);

        // m_State.EnviromentMap = EnviromentMap::Create("Assets/Textures/Skybox.hdr");
        m_State.DefaultEnviromentMap = CreateRef<EnviromentMap>();
        m_State.DefaultEnviromentMap->Skybox = CreateRef<Texture2D>();
        m_State.DefaultEnviromentMap->Irradiance = CreateRef<Texture2D>();
        m_State.CurrentEnviromentMap = m_State.DefaultEnviromentMap;

        m_State.TransformBuffer = ShaderStorageBuffer::Create(0);
        m_State.MaterialBuffer = ShaderStorageBuffer::Create(1);
        m_State.ShaderGBuffer = ShaderStorageBuffer::Create(2);
        m_State.LightBuffer = ShaderStorageBuffer::Create(3);

        {
            RenderTextureSpecification spec;
            spec.Size = BlVec2<u32>(1920, 1080);
            spec.Attachments = {
                {0, RenderTextureAttachmentType::ColorRGBA16F}, // position color buffer
                {1, RenderTextureAttachmentType::ColorRGBA16F}, // normal buffer
                {2, RenderTextureAttachmentType::ColorRGBA8}, // color buffer
                {3, RenderTextureAttachmentType::ColorRGBA16F}, // material buffer
                {4, RenderTextureAttachmentType::Depth} // depth
            };
            spec.ActiveAttachments = {0, 1, 2, 3}; // which attachments we want to use for rendering

            m_State.GBuffer = RenderTexture::Create(spec);
        }
    }

    // SceneRenderer::~SceneRenderer() {}

    void SceneRenderer::Render(Scene* scene, RenderTexture* target) {
        m_Target = target;

        auto dirLightView = scene->m_ECS->GetEntitiesWithComponents<TransformComponent, DirectionalLightComponent>();

        dirLightView.each([&](TransformComponent& transform, DirectionalLightComponent& light) {
            AddDirectionalLight(transform, light);    
        });

        auto lightView = scene->m_ECS->GetEntitiesWithComponents<TransformComponent, PointLightComponent>();

        lightView.each([&](TransformComponent& transform, PointLightComponent& light) {
            AddPointLight(transform, light);
        });

        auto meshView = scene->m_ECS->GetEntitiesWithComponents<TransformComponent, MeshComponent>();

        meshView.each([&](TransformComponent& transform, MeshComponent& mesh) {
            if (Project::GetAssetManager().ContainsAsset(mesh.MeshHandle)) {
                Model& model = std::get<Model>(Project::GetAssetManager().GetAsset(mesh.MeshHandle).Data);

                AddModel(transform.GetMatrix(), model, BlColor(255, 255, 255, 255));
            }
        });

        auto envView = scene->m_ECS->GetEntitiesWithComponents<EnviromentComponent>();

        envView.each([&](EnviromentComponent& env) {
            AddEnviroment(env);
        });

        Flush();
    }

    void SceneRenderer::SetCamera(const SceneCamera& camera) {
        m_Camera = camera;
    }

    void SceneRenderer::AddMesh(const glm::mat4& transform, const Mesh& mesh, BlColor color) {
        if (!Project::GetAssetManager().ContainsAsset(mesh.MaterialHandle)) return;

        Material& mat = std::get<Material>(Project::GetAssetManager().GetAsset(mesh.MaterialHandle).Data);

        BlVec4<f32> normColor = NormalizeColor(color);

        // vertices
        for (u32 i = 0; i < mesh.Positions.size(); i++) {
            SceneMeshVertex vert = SceneMeshVertex(mesh.Positions[i], mesh.Normals[i], mesh.TexCoords[i], m_State.MaterialIndex, m_State.ObjectIndex);
            m_State.MeshVertices.push_back(vert);
        }

        // indices
        for (u32 i = 0; i < mesh.Indices.size(); i++) {
            const u32 vertexCount = m_State.MeshVertexCount;
            m_State.MeshIndices.push_back(mesh.Indices[i] + vertexCount);
        }

        // Add transform which will be sent to shader
        m_State.Transforms.push_back(transform);

        GPUMaterial gpuMat;
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

        m_State.Materials.push_back(gpuMat);

        m_State.MeshVertexCount += mesh.Positions.size();
        m_State.MeshIndexCount += mesh.Indices.size();

        m_State.ObjectIndex++;
        m_State.MaterialIndex++;
    }

    void SceneRenderer::AddModel(const glm::mat4& transform, const Model& model, BlColor color) {
        for (u32 i = 0; i < model.MeshCount; ++i) {
            AddMesh(transform, model.Meshes[i], color);
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

    void SceneRenderer::AddEnviroment(const EnviromentComponent& env) {
        if (!Project::GetAssetManager().ContainsAsset(env.EnviromentMap)) return;

        Asset& a = Project::GetAssetManager().GetAsset(env.EnviromentMap);

        m_State.CurrentEnviromentMap = std::get<Ref<EnviromentMap>>(Project::GetAssetManager().GetAsset(env.EnviromentMap).Data);
    }

    void SceneRenderer::Flush() {
        ScopedTimer timer("SceneRenderer::Flush");

        auto& renderer = BL_APP.GetRenderer();

        if (m_State.MeshIndices.size() > 0) {
            {
                ScopedTimer geometryPassTimer("SceneRenderer::Flush/Geometry Pass");

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
                    {4, ShaderDataType::Int, "ObjectIndex"}
                });

                renderer.BindShader(m_State.MeshGeometryShader);
                m_State.MeshGeometryShader.SetMatrix("u_ViewProjection", m_Camera.GetCameraMatrixFloat());

                {
                    ScopedTimer transformSettingTimer("SceneRenderer::Flush/Passing transforms");
                    m_State.TransformBuffer.ReserveMemory(sizeof(glm::mat4) * m_State.Transforms.size(), m_State.Transforms.data());
                }

                {
                    ScopedTimer materialSettingTimer("SceneRenderer::Flush/Passing materials");
                    m_State.MaterialBuffer.ReserveMemory(sizeof(GPUMaterial) * m_State.Materials.size(), m_State.Materials.data());
                }


                renderer.BindRenderTexture(m_State.GBuffer);
                renderer.Clear(BlColor(0, 0, 0, 255));

                renderer.DrawIndexed(m_State.MeshIndexCount);

                renderer.UnBindRenderTexture();
            }

            {
                ScopedTimer lightingPassTimer("SceneRenderer::Flush/Lighting Pass");

                if (m_Target) {
                    renderer.BindRenderTexture(m_Target);
                    renderer.Clear(BlColor(0, 0, 0, 255));
                }

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

                m_State.SkyboxShader.SetMatrix("u_Projection", glm::value_ptr(projection));
                m_State.SkyboxShader.SetMatrix("u_View", glm::value_ptr(view));

                if (m_State.CurrentEnviromentMap)
                    renderer.BindCubemap(m_State.CurrentEnviromentMap->Skybox, 0);

                renderer.DrawIndexed(36);

                glDepthMask(GL_TRUE);

                if (m_Target) {
                    renderer.UnBindRenderTexture();
                }

                // Lighting pass
                if (m_Target) {
                    renderer.BindRenderTexture(m_Target);
                    // renderer.Clear(BlColor(69, 69, 69, 255));
                }

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
                m_State.MeshLightingShader.SetVec3("u_ViewPos", m_Camera.Transform.Position);

                u64 handles[4]{};
                for (u32 i = 0; i < 4; i++) {
                    handles[i] = m_State.GBuffer->Attachments[i]->BindlessHandle;
                }
                m_State.ShaderGBuffer.ReserveMemory(sizeof(u64) * 4, handles);

                // set lights
                m_State.MeshLightingShader.SetVec4("u_DirectionalLight.Direction", m_State.DirectionalLight.Direction);
                m_State.MeshLightingShader.SetVec4("u_DirectionalLight.Color", m_State.DirectionalLight.Color);
                m_State.MeshLightingShader.SetVec4("u_DirectionalLight.Params", m_State.DirectionalLight.Params);

                m_State.LightBuffer.ReserveMemory(sizeof(GPUPointLight) * m_State.PointLights.size(), m_State.PointLights.data());
                m_State.MeshLightingShader.SetInt("u_PointLightCount", m_State.PointLights.size());

                if (m_State.CurrentEnviromentMap)
                    renderer.BindCubemap(m_State.CurrentEnviromentMap->Irradiance, 0);

                renderer.DrawIndexed(6);

                renderer.UnBindCubemap();

                if (m_Target) {
                    renderer.UnBindRenderTexture();
                }
            }

            m_State.MeshVertices.clear();
            m_State.MeshIndices.clear();

            m_State.MeshVertexCount = 0;
            m_State.MeshIndexCount = 0;
        }

        m_State.MaterialIndex = 0;
        m_State.ObjectIndex = 0;
        m_State.Transforms.clear();
        m_State.Materials.clear();
        m_State.PointLights.clear();

        m_State.CurrentEnviromentMap = m_State.DefaultEnviromentMap;
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

    SceneRendererState& Blackberry::SceneRenderer::GetState() {
        return m_State;
    }

} // namespace Blackberry