#include "blackberry/scene/scene_renderer.hpp"
#include "blackberry/scene/scene.hpp"
#include "blackberry/project/project.hpp"

namespace Blackberry {

#pragma region ShaderCode

    static const char* s_VertexShaderMeshGeometrySource = BL_STR(
        \x23version 460 core\n
        \x23 extension GL_ARB_bindless_texture : enable\n

        layout (location = 0) in vec3 a_Pos;
        layout (location = 1) in vec3 a_Normal;
        layout (location = 2) in vec2 a_TexCoord;
        layout (location = 3) in int a_MaterialIndex;
        layout (location = 4) in int a_ObjectIndex;

        uniform mat4 u_ViewProjection;

        layout(std430, binding = 0) buffer TransformBuffer {
            mat4 Transforms[];
        };

        layout (location = 0) out vec3 o_Normal;
        layout (location = 1) out vec2 o_TexCoord;
        layout (location = 2) out vec3 o_FragPos;
        layout (location = 3) out flat int o_MaterialIndex;

        void main() {
            gl_Position = u_ViewProjection * Transforms[a_ObjectIndex] * vec4(a_Pos, 1.0f);

            o_Normal = mat3(transpose(inverse(Transforms[a_ObjectIndex]))) * a_Normal;
            o_TexCoord = a_TexCoord;
            o_FragPos = a_Pos;
            o_MaterialIndex = a_MaterialIndex;
        }
    );

    static const char* s_FragmentShaderMeshGeometrySource = BL_STR(
        \x23version 460 core\n
        \x23 extension GL_ARB_bindless_texture : enable\n // enable ARB bindless textures

        layout (location = 0) in vec3 a_Normal;
        layout (location = 1) in vec2 a_TexCoord;
        layout (location = 2) in vec3 a_FragPos;
        layout (location = 3) in flat int a_MaterialIndex;
        
        struct Material {
            uvec2 Albedo;
            uvec2 Metallic;
            uvec2 Roughness;
            uvec2 AO;
        };
        
        layout (std430, binding = 1) buffer MaterialBuffer {
            Material Materials[];
        };
        
        layout (location = 0) out vec4 o_GPosition;
        layout (location = 1) out vec4 o_GNormal;
        layout (location = 2) out vec4 o_GAlbedo;
        layout (location = 3) out vec4 o_GMat;
         
        void main() {
            // Store the position in the first buffer
            o_GPosition.rgb = a_FragPos;
            // Store the normal in the second buffer
            o_GNormal.rgb = normalize(a_Normal);
            // Store the diffuse color in the third buffer
            o_GAlbedo.rgb = texture(sampler2D(Materials[a_MaterialIndex].Albedo), a_TexCoord).rgb;
            // Store material information in the fourth buffer
            o_GMat.r = texture(sampler2D(Materials[a_MaterialIndex].Metallic), a_TexCoord).r;
            o_GMat.g = texture(sampler2D(Materials[a_MaterialIndex].Roughness), a_TexCoord).r;
            o_GMat.b = texture(sampler2D(Materials[a_MaterialIndex].AO), a_TexCoord).r;

            // For visualizations (normally the alpha would just get set to 0.0)
            o_GPosition.a = 1.0;
            o_GNormal.a = 1.0;
            o_GAlbedo.a = 1.0;
            o_GMat.a = 1.0;
        }
    );

    static const char* s_VertexShaderMeshLightingSource = BL_STR(
        \x23version 460 core\n
        \x23 extension GL_ARB_bindless_texture : enable\n

        layout (location = 0) in vec2 a_Pos;
        layout (location = 1) in vec2 a_TexCoord;

        layout (location = 0) out vec2 o_TexCoord;

        void main() {
            gl_Position = vec4(a_Pos, 0.0, 1.0);
            o_TexCoord = a_TexCoord;
        }
    );

    static const char* s_FragmentShaderMeshLightingSource = BL_STR(
        \x23version 460 core\n
        \x23 extension GL_ARB_bindless_texture : enable\n

        layout (location = 0) in vec2 a_TexCoord;

        layout (std430, binding = 2) buffer GBuffer {
            uvec2 GPosition;
            uvec2 GNormal;
            uvec2 GAlbedo;
            uvec2 GMat;
        };

        struct Light {
            vec3 Position;
            vec3 Color;
        };
        const int MAX_LIGHTS = 32;
        uniform Light u_Lights[MAX_LIGHTS];
        uniform vec3 u_ViewPos;

        out vec4 o_FragColor;

        const float PI = 3.14159265359;

        float DistributionGGX(vec3 N, vec3 H, float roughness);
        float GeometrySchlickGGX(float NdotV, float roughness);
        float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
        vec3 FresnelSchlick(float cosTheta, vec3 F0);

        void main() {
            vec3 worldPos =   texture(sampler2D(GPosition), a_TexCoord).rgb;
            vec3 normal =     texture(sampler2D(GNormal), a_TexCoord).rgb;
            vec3 albedo = pow(texture(sampler2D(GAlbedo), a_TexCoord).rgb, vec3(2.2));
            float metallic =  texture(sampler2D(GMat), a_TexCoord).r;
            float roughness = texture(sampler2D(GMat), a_TexCoord).g;
            float ao =        texture(sampler2D(GMat), a_TexCoord).b;
            
            vec3 N = normal;
            vec3 V = normalize(u_ViewPos - worldPos);
            
            vec3 F0 = vec3(0.04);
            F0 = mix(F0, albedo, metallic);
            
            // reflectance equation
            vec3 Lo = vec3(0.0);
            for (int i = 0; i < MAX_LIGHTS; i++) {
                // calculate per light radiance
                vec3 L = normalize(u_Lights[i].Position - worldPos);
                vec3 H = normalize(V + L);
                float distance = length(u_Lights[i].Position - worldPos);
                float attenuation = 1.0 / (distance * distance);
                vec3 radiance = u_Lights[i].Color * attenuation;
            
                // cook-torrance brdf
                float NDF = DistributionGGX(N, H, roughness);
                float G = GeometrySmith(N, V, L, roughness);
                vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
            
                vec3 kS = F;
                vec3 kD = vec3(1.0) - kS;
                kD *= 1.0 - metallic;
            
                vec3 numerator = NDF * G * F;
                float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
                vec3 specular = numerator / denominator;
            
                // add to radiance
                float NdotL = max(dot(N, L), 0.0);
                Lo += (kD * albedo / PI + specular) * radiance * NdotL;
            }
            
            vec3 ambient = vec3(0.03) * albedo * ao;
            vec3 color = ambient + Lo;
            
            // HDR tonemapping
            color = color / (color + vec3(1.0));
            // Gamma correct
            color = pow(color, vec3(1.0 / 2.2));
            
            o_FragColor = vec4(color, 1.0);
        }

        float DistributionGGX(vec3 N, vec3 H, float roughness) {
            float a = roughness * roughness;
            float a2 = a * a;
            float NdotH = max(dot(N, H), 0.0);
            float NdotH2 = NdotH * NdotH;
        
            float nom   = a2;
            float denom = (NdotH2 * (a2 - 1.0) + 1.0);
            denom = PI * denom * denom;
        
            return nom / denom;
        }

        float GeometrySchlickGGX(float NdotV, float roughness) {
            float r = (roughness + 1.0);
            float k = (r * r) / 8.0;
        
            float nom   = NdotV;
            float denom = NdotV * (1.0 - k) + k;
        
            return nom / denom;
        }

        float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
            float NdotV = max(dot(N, V), 0.0);
            float NdotL = max(dot(N, L), 0.0);
            float ggx2 = GeometrySchlickGGX(NdotV, roughness);
            float ggx1 = GeometrySchlickGGX(NdotL, roughness);
        
            return ggx1 * ggx2;
        }

        vec3 FresnelSchlick(float cosTheta, vec3 F0) {
            return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
        }
    );

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

    constexpr u32 MAX_OBJECTS = 8192;
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
        m_State.MeshGeometryShader = Shader::Create(s_VertexShaderMeshGeometrySource, s_FragmentShaderMeshGeometrySource);
        m_State.MeshLightingShader = Shader::Create(s_VertexShaderMeshLightingSource, s_FragmentShaderMeshLightingSource);
        m_State.FontShader = Shader::Create(s_VertexShaderFontSource, s_FragmentShaderFontSource);

        m_State.TransformBuffer = ShaderStorageBuffer::Create(0);
        m_State.TransformBuffer.ReserveMemory(sizeof(glm::mat4) * MAX_OBJECTS);

        m_State.MaterialBuffer = ShaderStorageBuffer::Create(1);
        m_State.MaterialBuffer.ReserveMemory(sizeof(GPUMaterial) * MAX_MATERIALS);

        m_State.ShaderGBuffer = ShaderStorageBuffer::Create(2);
        m_State.ShaderGBuffer.ReserveMemory(sizeof(u64) * 4); // four uvec2's

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

    // SceneRenderer::~SceneRenderer() {}

    void SceneRenderer::Render(Scene* scene, RenderTexture* target) {
        m_Target = target;

        auto dirLightView = scene->m_ECS->GetEntitiesWithComponents<TransformComponent, DirectionalLightComponent>();

        dirLightView.each([&](TransformComponent& transform, DirectionalLightComponent& light) {
            AddDirectionalLight(transform, light);    
        });

        auto lightView = scene->m_ECS->GetEntitiesWithComponents<TransformComponent, LightComponent>();

        lightView.each([&](TransformComponent& transform, LightComponent& light) {
            AddLight(transform, light);
        });

        auto meshView = scene->m_ECS->GetEntitiesWithComponents<TransformComponent, MeshComponent>();

        meshView.each([&](TransformComponent& transform, MeshComponent& mesh) {
            if (Project::GetAssetManager().ContainsAsset(mesh.MeshHandle)) {
                Model& model = std::get<Model>(Project::GetAssetManager().GetAsset(mesh.MeshHandle).Data);

                AddModel(transform.GetMatrix(), model, BlColor(255, 255, 255, 255));
            }
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
            // glm::vec4 pos = transform * glm::vec4(mesh.Positions[i].x, mesh.Positions[i].y, mesh.Positions[i].z, 1.0f);
            // glm::vec3 normal = glm::mat3(glm::transpose(glm::inverse(transform))) * glm::vec3(mesh.Normals[i].x, mesh.Normals[i].y, mesh.Normals[i].z);
            // glm::vec3 normal = glm::vec3(mesh.Normals[i].x, mesh.Normals[i].y, mesh.Normals[i].z);
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
        gpuMat.Albedo = mat.Albedo.BindlessHandle;
        gpuMat.Metallic = mat.Metallic.BindlessHandle;
        gpuMat.Roughness = mat.Roughness.BindlessHandle;
        gpuMat.AO = mat.AO.BindlessHandle;
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
        m_State.DirectionalLight.Direction = transform.Rotation;
        
        m_State.DirectionalLight.Ambient = light.Ambient;
        m_State.DirectionalLight.Diffuse = light.Diffuse;
        m_State.DirectionalLight.Specular = light.Specular;
    }

    void SceneRenderer::AddLight(const TransformComponent& transform, const LightComponent& light) {
        SceneLight l;
        l.Position = transform.Position;
        l.Color = light.Color;

        m_State.Lights[m_State.LightIndex] = l;
        m_State.LightIndex++;
    }

    void SceneRenderer::Flush() {
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
                {4, ShaderDataType::Int, "ObjectIndex"}
            });

            renderer.BindShader(m_State.MeshGeometryShader);
            m_State.MeshGeometryShader.SetMatrix("u_ViewProjection", m_Camera.GetCameraMatrixFloat());

            // send tranform data
            glm::mat4* tBuffer = reinterpret_cast<glm::mat4*>(m_State.TransformBuffer.MapMemory());

            for (u32 i = 0; i < m_State.Transforms.size(); i++) {
                tBuffer[i] = m_State.Transforms[i];
            }

            // we MUST unmap buffer
            m_State.TransformBuffer.UnMapMemory();

            GPUMaterial* mBuffer = reinterpret_cast<GPUMaterial*>(m_State.MaterialBuffer.MapMemory());

            // apply materials
            for (u32 i = 0; i < m_State.MaterialIndex; i++) {
                mBuffer[i] = m_State.Materials[i];
            }

            m_State.MaterialBuffer.UnMapMemory();

            renderer.BindRenderTexture(m_State.GBuffer);
            renderer.Clear(BlColor(0, 0, 0, 255));

            renderer.DrawIndexed(m_State.MeshIndexCount);

            renderer.UnBindRenderTexture();

            // Lighting pass
            if (m_Target) {
                renderer.BindRenderTexture(*m_Target);
                renderer.Clear(BlColor(69, 69, 69, 255));
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

            u64* gBuffer = reinterpret_cast<u64*>(m_State.ShaderGBuffer.MapMemory());

            for (u32 i = 0; i < 4; i++) {
                gBuffer[i] = m_State.GBuffer.Attachments[i].BindlessHandle;
            }

            m_State.ShaderGBuffer.UnMapMemory();

            // m_State.MeshLightingShader.SetUInt64("u_GPosition", m_State.GBuffer.Attachments[0].BindlessHandle);
            // m_State.MeshLightingShader.SetUInt64("u_GNormal",   m_State.GBuffer.Attachments[1].BindlessHandle);
            // m_State.MeshLightingShader.SetUInt64("u_GAlbedo",   m_State.GBuffer.Attachments[2].BindlessHandle);
            // m_State.MeshLightingShader.SetUInt64("u_GMat",      m_State.GBuffer.Attachments[3].BindlessHandle);

            // set directional light
            // m_State.MeshLightingShader.SetVec3("u_DirectionalLight.Direction", m_State.DirectionalLight.Direction);
            // m_State.MeshLightingShader.SetVec3("u_DirectionalLight.Ambient", m_State.DirectionalLight.Ambient);
            // m_State.MeshLightingShader.SetVec3("u_DirectionalLight.Diffuse", m_State.DirectionalLight.Diffuse);
            // m_State.MeshLightingShader.SetVec3("u_DirectionalLight.Specular", m_State.DirectionalLight.Specular);

            // set lights
            for (u32 i = 0; i < 32; i++) {
                m_State.MeshLightingShader.SetVec3(fmt::format("u_Lights[{}].Position", i), m_State.Lights[i].Position);
                m_State.MeshLightingShader.SetVec3(fmt::format("u_Lights[{}].Color", i), m_State.Lights[i].Color);
            }

            renderer.DrawIndexed(6);

            if (m_Target) {
                renderer.UnBindRenderTexture();
            }

            m_State.MeshVertices.clear();
            m_State.MeshIndices.clear();

            m_State.MeshVertexCount = 0;
            m_State.MeshIndexCount = 0;
        }

        m_State.MaterialIndex = 0;
        m_State.ObjectIndex = 0;
        m_State.LightIndex = 0;
        m_State.Transforms.clear();
        m_State.Materials.clear();
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