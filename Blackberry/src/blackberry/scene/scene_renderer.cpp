#include "blackberry/scene/scene_renderer.hpp"
#include "blackberry/scene/scene.hpp"
#include "blackberry/project/project.hpp"

namespace Blackberry {

#pragma region ShaderCode

    static const char* s_VertexShaderMeshSource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec3 a_Pos;
        layout (location = 1) in vec3 a_Normal;
        layout (location = 2) in vec4 a_Color;

        uniform mat4 u_Projection;

        layout (location = 0) out vec3 o_Normal;
        layout (location = 1) out vec4 o_Color;
        layout (location = 4) out vec3 o_FragPos;

        void main() {
            gl_Position = u_Projection * vec4(a_Pos, 1.0f);

            o_Normal = a_Normal;
            o_Color = a_Color;
            o_FragPos = a_Pos;
        }
    );

    static const char* s_FragmentShaderMeshSource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec3 a_Normal;
        layout (location = 1) in vec4 a_Color;
        layout (location = 4) in vec3 a_FragPos;

        struct DirectionalLight {
            vec3 Direction;

            vec3 Ambient;
            vec3 Diffuse;
            vec3 Specular;
        };

        uniform vec3 u_ViewPos;
        uniform DirectionalLight u_Light;
        uniform sampler2D u_Textures[16];

        out vec4 o_FragColor;

        void main() {
            // ambient
            vec3 ambient = u_Light.Ambient;

            // diffuse
            vec3 norm = normalize(a_Normal);
            vec3 lightDir = normalize(-u_Light.Direction);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = u_Light.Diffuse * diff;

            // specular
            vec3 viewDir = normalize(u_ViewPos - a_FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = u_Light.Specular * spec;  

            vec4 result = vec4(ambient + diffuse + specular, 1.0);
            o_FragColor = result * a_Color;
            // o_FragColor = vec4(a_Normal * 0.5 + 0.5, 1.0);
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
        m_State.MeshShader = Shader::Create(s_VertexShaderMeshSource, s_FragmentShaderMeshSource);
        m_State.FontShader = Shader::Create(s_VertexShaderFontSource, s_FragmentShaderFontSource);
    }

    // SceneRenderer::~SceneRenderer() {}

    void SceneRenderer::Render(Scene* scene) {
        auto dirLightView = scene->m_ECS->GetEntitiesWithComponents<DirectionalLightComponent>();

        dirLightView.each([&](DirectionalLightComponent& light) {
            AddDirectionalLight(light);    
        });

        auto meshView = scene->m_ECS->GetEntitiesWithComponents<TransformComponent, MeshComponent>();

        meshView.each([&](TransformComponent& transform, MeshComponent& mesh) {
            Model model = std::get<Model>(Project::GetAssetManager().GetAsset(mesh.MeshHandle).Data);

            AddModel(transform.GetMatrix(), model, BlColor(155, 255, 100, 255));
        });

        Flush();
    }

    void SceneRenderer::SetCamera(const SceneCamera& camera) {
        m_Camera = camera;
    }

    void SceneRenderer::AddMesh(const glm::mat4& transform, const Mesh& mesh, BlColor color) {
        BlVec4<f32> normColor = NormalizeColor(color);

        // vertices
        for (u32 i = 0; i < mesh.Positions.size(); i++) {
            glm::vec4 pos = transform * glm::vec4(mesh.Positions[i].x, mesh.Positions[i].y, mesh.Positions[i].z, 1.0f);
            glm::vec3 normal = glm::mat3(glm::transpose(glm::inverse(transform))) * glm::vec3(mesh.Normals[i].x, mesh.Normals[i].y, mesh.Normals[i].z);
            SceneMeshVertex vert = SceneMeshVertex(BlVec3<f32>(pos.x, pos.y, pos.z), BlVec3<f32>(normal.x, normal.y, normal.z), normColor);
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

    void SceneRenderer::AddDirectionalLight(const DirectionalLightComponent& light) {
        m_State.DirectionalLight = light;
    }

    void SceneRenderer::Flush() {
        auto& renderer = BL_APP.GetRenderer();

        if (m_State.MeshIndices.size() > 0) {
            DrawBuffer buffer;
            buffer.Vertices = m_State.MeshVertices.data();
            buffer.VertexSize = sizeof(SceneMeshVertex);
            buffer.VertexCount = m_State.MeshVertexCount;

            buffer.Indices = m_State.MeshIndices.data();
            buffer.IndexSize = sizeof(u32);
            buffer.IndexCount = m_State.MeshIndexCount;

            renderer.SubmitDrawBuffer(buffer);
            renderer.SetBufferLayout({
                {0, ShaderDataType::Float3, "Position"},
                {1, ShaderDataType::Float3, "Normal"},
                {2, ShaderDataType::Float4, "Color"},
            });

            renderer.BindShader(m_State.MeshShader);
            m_State.MeshShader.SetMatrix("u_Projection", m_Camera.GetCameraMatrixFloat());
            m_State.MeshShader.SetVec3("u_ViewPos", m_Camera.Transform.Position);

            auto& light = m_State.DirectionalLight;
            m_State.MeshShader.SetVec3("u_Light.Direction", BlVec3(light.Direction));
            m_State.MeshShader.SetVec3("u_Light.Ambient", BlVec3(light.Ambient.r / 255.0f, light.Ambient.g / 255.0f, light.Ambient.b / 255.0f));
            m_State.MeshShader.SetVec3("u_Light.Diffuse", BlVec3(light.Diffuse.r / 255.0f, light.Diffuse.g / 255.0f, light.Diffuse.b / 255.0f));
            m_State.MeshShader.SetVec3("u_Light.Specular", BlVec3(light.Specular.r / 255.0f, light.Specular.g / 255.0f, light.Specular.b / 255.0f));

            renderer.DrawIndexed(m_State.MeshIndexCount);

            m_State.MeshVertices.clear();
            m_State.MeshIndices.clear();

            m_State.MeshVertexCount = 0;
            m_State.MeshIndexCount = 0;
        }
    }

} // namespace Blackberry