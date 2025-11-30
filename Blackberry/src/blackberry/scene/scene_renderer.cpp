#include "blackberry/scene/scene_renderer.hpp"
#include "blackberry/scene/scene.hpp"
#include "blackberry/project/project.hpp"
#include "glad/glad.h"

namespace Blackberry {

#pragma region ShaderCode

    static const char* s_VertexShaderMeshGeometrySource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec3 a_Pos;
        layout (location = 1) in vec3 a_Normal;
        layout (location = 2) in vec2 a_TexCoord;
        layout (location = 3) in int a_MaterialIndex;

        uniform mat4 u_Projection;

        layout (location = 0) out vec3 o_Normal;
        layout (location = 1) out vec2 o_TexCoord;
        layout (location = 2) out vec3 o_FragPos;
        layout (location = 3) out flat int o_MaterialIndex;

        void main() {
            gl_Position = u_Projection * vec4(a_Pos, 1.0f);

            o_Normal = a_Normal;
            o_TexCoord = a_TexCoord;
            o_FragPos = a_Pos;
            o_MaterialIndex = a_MaterialIndex;
        }
    );

    static const char* s_FragmentShaderMeshGeometrySource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec3 a_Normal;
        layout (location = 1) in vec2 a_TexCoord;
        layout (location = 2) in vec3 a_FragPos;
        layout (location = 3) in flat int a_MaterialIndex;
        
        struct Material {
            sampler2D Diffuse;
            sampler2D Specular;
            float Shininess;
        };
        
        uniform Material u_Materials[16];
        
        layout (location = 0) out vec4 o_GPosition;
        layout (location = 1) out vec4 o_GNormal;
        layout (location = 2) out vec4 o_GAlbedoSpec;
         
        void main() {
            // Store the position in the first buffer
            o_GPosition.rgb = a_FragPos;
            // Store the normal in the second buffer
            o_GNormal.rgb = normalize(a_Normal);
            // Store the diffuse color in the rgb part of the third buffer
            o_GAlbedoSpec.rgb = texture(u_Materials[a_MaterialIndex].Diffuse, a_TexCoord).rgb;
            // Store the specular intensity in the alpha channel of the third buffer
            o_GAlbedoSpec.a = texture(u_Materials[a_MaterialIndex].Specular, a_TexCoord).r;

            // For visualizations of normals and positions (normally the alpha would just get set to 0.0)
            o_GPosition.a = 1.0;
            o_GNormal.a = 1.0;
        }
    );

    static const char* s_VertexShaderMeshLightingSource = BL_STR(
        \x23version 460 core\n

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

        layout (location = 0) in vec2 a_TexCoord;

        uniform sampler2D u_GPosition;
        uniform sampler2D u_GNormal;
        uniform sampler2D u_GAlbedoSpec;

        struct Light {
            vec3 Position;
            vec3 Color;
        };
        const int MAX_LIGHTS = 32;
        uniform Light u_Lights[MAX_LIGHTS];
        uniform vec3 u_ViewPos;

        out vec4 o_FragColor;

        void main() {
            // Retrieve data from gBuffer
            vec3 fragPos = texture(u_GPosition, a_TexCoord).rgb;
            vec3 normal = texture(u_GNormal, a_TexCoord).rgb;
            vec3 albedo = texture(u_GAlbedoSpec, a_TexCoord).rgb;
            float specular = texture(u_GAlbedoSpec, a_TexCoord).a;

            vec3 lighting = albedo * 0.1;
            vec3 viewDir = normalize(u_ViewPos - fragPos);

            for (int i = 0; i < MAX_LIGHTS; ++i) {
                vec3 lightDir = normalize(u_Lights[i].Position - fragPos);
                vec3 diffuse = max(dot(normal, lightDir), 0.0) * albedo * u_Lights[i].Color;
                lighting += diffuse;
            }

            o_FragColor = vec4(albedo, 1.0);
            // o_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
            // o_FragColor = vec4(normal, 1.0);
        }
    );

    static const char* s_VertexShaderFontSource = BL_STR(
        \x23version 460 core\n

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

        RenderTextureSpecification spec;
        spec.Size = BlVec2<u32>(1920, 1080);
        spec.Attachments = {
            {0, RenderTextureAttachmentType::ColorRGBA16F}, // position color buffer
            {1, RenderTextureAttachmentType::ColorRGBA16F}, // normal buffer
            {2, RenderTextureAttachmentType::ColorRGBA8}, // color + specular buffer
            {3, RenderTextureAttachmentType::Depth} // depth
        };
        spec.ActiveAttachments = {0, 1, 2}; // which attachments we want to use for rendering

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
        u32 materialIndex = GetMaterialIndex(mat);

        // vertices
        for (u32 i = 0; i < mesh.Positions.size(); i++) {
            glm::vec4 pos = transform * glm::vec4(mesh.Positions[i].x, mesh.Positions[i].y, mesh.Positions[i].z, 1.0f);
            glm::vec3 normal = glm::mat3(glm::transpose(glm::inverse(transform))) * glm::vec3(mesh.Normals[i].x, mesh.Normals[i].y, mesh.Normals[i].z);
            SceneMeshVertex vert = SceneMeshVertex(BlVec3<f32>(pos.x, pos.y, pos.z), BlVec3<f32>(normal.x, normal.y, normal.z), mesh.TexCoords[i], materialIndex);
            m_State.MeshVertices.push_back(vert);
        }

        // indices
        for (u32 i = 0; i < mesh.Indices.size(); i++) {
            const u32 vertexCount = m_State.MeshVertexCount;
            m_State.MeshIndices.push_back(mesh.Indices[i] + vertexCount);
        }

        m_State.MeshVertexCount += mesh.Positions.size();
        m_State.MeshIndexCount += mesh.Indices.size();
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
                {3, ShaderDataType::Int, "MaterialIndex"}
            });

            renderer.BindShader(m_State.MeshGeometryShader);
            m_State.MeshGeometryShader.SetMatrix("u_Projection", m_Camera.GetCameraMatrixFloat());

            // apply materials
            for (u32 i = 0; i < m_State.MaterialIndex; i++) {
                Material& mat = m_State.Materials[i];

                m_State.MeshGeometryShader.SetFloat(fmt::format("u_Materials[{}].Shininess", i), mat.Shininess);

                if (Project::GetAssetManager().ContainsAsset(mat.Diffuse)) {
                    Texture2D diffuse = std::get<Texture2D>(Project::GetAssetManager().GetAsset(mat.Diffuse).Data);

                    renderer.BindTexture(diffuse, i + 0);
                    m_State.MeshGeometryShader.SetInt(fmt::format("u_Materials[{}].Diffuse", i), i + 0);
                }

                if (Project::GetAssetManager().ContainsAsset(mat.Specular)) {
                    Texture2D specular = std::get<Texture2D>(Project::GetAssetManager().GetAsset(mat.Specular).Data);

                    renderer.BindTexture(specular, i + 1);
                    m_State.MeshGeometryShader.SetInt(fmt::format("u_Materials[{}].Specular", i), i + 1);
                }
            }

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

            m_State.MeshLightingShader.SetInt("u_GPosition", 0);
            m_State.MeshLightingShader.SetInt("u_GNormal", 1);
            m_State.MeshLightingShader.SetInt("u_GAlbedoSpec", 2);

            for (u32 i = 0; i < m_State.Lights.size(); i++) {
                m_State.MeshLightingShader.SetVec3(fmt::format("u_Lights[{}].Position", i), BlVec3(m_State.Lights[i].Position));
                m_State.MeshLightingShader.SetVec3(fmt::format("u_Lights[{}].Color", i), BlVec3(m_State.Lights[i].Color));
            }

            renderer.BindTexture(m_State.GBuffer.Attachments[0], 0);
            renderer.BindTexture(m_State.GBuffer.Attachments[1], 1);
            renderer.BindTexture(m_State.GBuffer.Attachments[2], 2);

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
        for (u32 i = 0; i < m_State.MaterialIndex; i++) {
            if (mat.ID == m_State.Materials[i].ID) {
                matIndex = i;
                texAlreadyExists = true;

                break;
            }
        }

        // If we haven't found a suitable material we create a new slot
        if (!texAlreadyExists) {
            matIndex = m_State.MaterialIndex;

            m_State.Materials[m_State.MaterialIndex] = mat;
            m_State.MaterialIndex++;
            texAlreadyExists = true;
        }

        return matIndex;
    }

    SceneRendererState& Blackberry::SceneRenderer::GetState() {
        return m_State;
    }

} // namespace Blackberry