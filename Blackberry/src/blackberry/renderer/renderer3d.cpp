#include "blackberry/renderer/renderer3d.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/scene/camera.hpp"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Blackberry {

    struct MeshVertex { // used for meshes (or just any 3d rendering)
        BlVec3<f32> Pos;
        BlVec3<f32> Normal;
        BlVec4<f32> Color;
        BlVec2<f32> TexCoord;
        f32 TexIndex = 0.0f; // OpenGL is weird with passing integers to shaders (also 0 is a white texture)
    };
    
    struct FontVertex {
        BlVec3<f32> Pos;
        BlVec4<f32> Color;
        BlVec2<f32> TexCoord;
    };

    static u8 s_WhiteTextureData[] = {0xff, 0xff, 0xff, 0xff}; // dead simple white texture (1x1 pixel)

    static const char* s_VertexShaderMeshSource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec3 a_Pos;
        layout (location = 1) in vec3 a_Normal;
        layout (location = 2) in vec4 a_Color;
        layout (location = 3) in vec2 a_TexCoord;
        layout (location = 4) in float a_TexIndex;

        uniform mat4 u_Projection;

        layout (location = 0) out vec3 o_Normal;
        layout (location = 1) out vec4 o_Color;
        layout (location = 2) out vec2 o_TexCoord;
        layout (location = 3) out flat float o_TexIndex;
        layout (location = 4) out vec3 o_FragPos;

        void main() {
            gl_Position = u_Projection * vec4(a_Pos, 1.0f);

            o_Normal = a_Normal;
            o_Color = a_Color;
            o_TexCoord = a_TexCoord;
            o_TexIndex = a_TexIndex;
            o_FragPos = a_Pos;
        }
    );

    static const char* s_FragmentShaderMeshSource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec3 a_Normal;
        layout (location = 1) in vec4 a_Color;
        layout (location = 2) in vec2 a_TexCoord;
        layout (location = 3) in flat float a_TexIndex;
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
            vec4 texColor = a_Color;

            switch(int(a_TexIndex))
	        {
	        	case  0: texColor *= texture(u_Textures[ 0], a_TexCoord); break;
	        	case  1: texColor *= texture(u_Textures[ 1], a_TexCoord); break;
	        	case  2: texColor *= texture(u_Textures[ 2], a_TexCoord); break;
	        	case  3: texColor *= texture(u_Textures[ 3], a_TexCoord); break;
	        	case  4: texColor *= texture(u_Textures[ 4], a_TexCoord); break;
	        	case  5: texColor *= texture(u_Textures[ 5], a_TexCoord); break;
	        	case  6: texColor *= texture(u_Textures[ 6], a_TexCoord); break;
	        	case  7: texColor *= texture(u_Textures[ 7], a_TexCoord); break;
	        	case  8: texColor *= texture(u_Textures[ 8], a_TexCoord); break;
	        	case  9: texColor *= texture(u_Textures[ 9], a_TexCoord); break;
	        	case 10: texColor *= texture(u_Textures[10], a_TexCoord); break;
	        	case 11: texColor *= texture(u_Textures[11], a_TexCoord); break;
	        	case 12: texColor *= texture(u_Textures[12], a_TexCoord); break;
	        	case 13: texColor *= texture(u_Textures[13], a_TexCoord); break;
	        	case 14: texColor *= texture(u_Textures[14], a_TexCoord); break;
	        	case 15: texColor *= texture(u_Textures[15], a_TexCoord); break;
	        }

            if (texColor.a == 0.0) { discard; }

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

    struct _Renderer3DState {
        // shaders
        BlShader MeshShader;
        BlShader FontShader;

        Texture2D WhiteTexture;

        // Mesh buffer data
        u32 MeshIndexCount = 0;
        u32 MeshVertexCount = 0;
        std::vector<MeshVertex> MeshVertices;
        std::vector<u32> MeshIndices;
        DrawBuffer MeshDrawBuffer;

        // Font buffer data
        u32 FontIndexCount = 0;
        u32 FontVertexCount = 0;
        std::vector<FontVertex> FontVertices;
        std::vector<u32> FontIndices;
        DrawBuffer FontDrawBuffer;
        Texture2D CurrentFontAtlas;

        // for textures
        u32 CurrentTexIndex = 1; // NOTE: 0 is reserved for a blank white texture!!!
        std::array<Texture2D, 16> CurrentAttachedTextures;

        // we still depend on these for things like font rendering
        const std::array<glm::vec4, 4> QuadVertexPositions = {{
            { -0.5f,  0.5f, 0.0f, 1.0f },
            {  0.5f, -0.5f, 0.0f, 1.0f },
            {  0.5f,  0.5f, 0.0f, 1.0f },
            { -0.5f, -0.5f, 0.0f, 1.0f }
        }};
        const std::array<u32, 6> QuadIndices = {{ 0, 1, 2, 0, 3, 1 }};

        SceneCamera Camera;
        SceneCamera DefaultCamera; // by default the camera is initialized to a basic 1x scale pixel-by-pixel orthographic projection (outdated now!!)

        DirectionalLight DirLight;

        Renderer3DStats Stats;
    };

    static _Renderer3DState Renderer3DState;

    static f32 GetTexIndex(Texture2D texture) {
        if (texture.ID == 0) return 0.0f; // return WhiteTexture if there is no texture

        f32 texIndex = 0.0f;

        if (Renderer3DState.CurrentTexIndex >= 16) {
            Renderer3D::Render();
        }

        bool texAlreadyExists = false;

        for (u32 i = 0; i < Renderer3DState.CurrentTexIndex; i++) {
            if (texture.ID == Renderer3DState.CurrentAttachedTextures[i].ID) {
                texIndex = static_cast<f32>(i);
                texAlreadyExists = true;

                break;
            }
        }

        if (!texAlreadyExists) {
            texIndex = static_cast<f32>(Renderer3DState.CurrentTexIndex);

            Renderer3DState.CurrentAttachedTextures[Renderer3DState.CurrentTexIndex] = texture;
            Renderer3DState.CurrentTexIndex++;
            texAlreadyExists = true;
        }

        return texIndex;
    }

    static BlVec4<f32> NormalizeColor(BlColor color) {
        return BlVec4<f32>(
            static_cast<f32>(color.r) / 255.0f,
            static_cast<f32>(color.g) / 255.0f,
            static_cast<f32>(color.b) / 255.0f,
            static_cast<f32>(color.a) / 255.0f
        );
    }

    void Renderer3D::Init() {
        Renderer3DState.MeshShader.Create(s_VertexShaderMeshSource, s_FragmentShaderMeshSource);
        Renderer3DState.FontShader.Create(s_VertexShaderFontSource, s_FragmentShaderFontSource);
        Renderer3DState.WhiteTexture = Texture2D::Create(s_WhiteTextureData, 1, 1, ImageFormat::RGBA8);

        Renderer3DState.CurrentAttachedTextures[0] = Renderer3DState.WhiteTexture; // 0 is reserved for white

        Renderer3DState.DefaultCamera.Transform = { BlVec3(0.0f), BlVec3(0.0f), BlVec3(1920, 1280, 0)};
        Renderer3DState.Camera = Renderer3DState.DefaultCamera;
    }

    void Renderer3D::Shutdown() {
        Renderer3DState.MeshShader.Delete();
        Renderer3DState.FontShader.Delete();
    }

    void Renderer3D::Clear(BlColor color) {
        auto& renderer = BL_APP.GetRenderer();

        renderer.Clear(color);
    }

    void Renderer3D::NewFrame() {
        Renderer3DState.Stats.DrawCalls = 0;
        Renderer3DState.Stats.Vertices = 0;
        Renderer3DState.Stats.Indicies = 0;
        Renderer3DState.Stats.ActiveTextures = 0;
    }

    void Renderer3D::DrawText(BlVec3<f32> position, f32 fontSize, const std::string& text, Font& font, TextParams params, BlColor color) {
        glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(fontSize / font.LineHeight, fontSize / font.LineHeight, 1.0f));

        glm::mat4 transform = pos * scale;

        DrawText(transform, text, font, params, color);
    }

    void Renderer3D::DrawText(const glm::mat4& transform, const std::string& text, Font& font, TextParams params, BlColor color) {
        Texture2D tex = font.TextureAtlas;
        f32 fsScale = 1.0f / (font.Ascender - font.Descender);
        f32 currentX = 0.0f;
        f32 currentY = 0.0f;

        BlVec2<f32> textSize = MeasureText(text, font, params);

        // glm::mat4 finalTextTransform = glm::scale(transform, glm::vec3(1.0f / textSize.x, 1.0f / textSize.y, 1.0f));
        glm::mat4 finalTextTransform = glm::scale(transform, glm::vec3(2.0f, 2.0f, 1.0f));

        BlColor currentColor = color;

        for (u32 c = 0; c < text.length(); c++) {
            if (text.at(c) == '\n') {
                currentY -= fsScale * font.LineHeight + params.LineSpacing;
                currentX = 0;
            } else {
                GlyphInfo glyph = font.GetGlyphInfo(text.at(c), 0);
                BlRec atlasBounds = glyph.AtlasRect;
                BlRec planeBounds = glyph.PlaneRect;

                if (tex.ID != Renderer3DState.CurrentFontAtlas.ID) {
                    Render();
                    Renderer3DState.CurrentFontAtlas = tex;
                }

                BlVec2<f32> texCoordMin(atlasBounds.x, atlasBounds.y);
                BlVec2<f32> texCoordMax(atlasBounds.w, atlasBounds.h);

                BlVec2<f32> quadMin = BlVec2<f32>(glyph.PlaneRect.x, glyph.PlaneRect.y);
                BlVec2<f32> quadMax = BlVec2<f32>(glyph.PlaneRect.w, glyph.PlaneRect.h);

                quadMin *= BlVec2<f32>(fsScale); quadMax *= BlVec2<f32>(fsScale);
                quadMin += BlVec2<f32>(currentX, currentY); quadMax += BlVec2<f32>(currentX, currentY);

                // make the text be perfectly centered (since the whole string must fit in a rectangle this is easy)
                // quadMin -= textSize * BlVec2<f32>(0.5f); quadMax -= textSize * BlVec2<f32>(0.5f);
                // quadMin -= BlVec2<f32>(0.5f); quadMax -= BlVec2<f32>(0.5f);
                quadMin.x -= 0.25f; quadMax.x -= 0.25f;
                quadMin.y -= 0.25f; quadMax.y -= 0.25f;

                f32 texelWidth = 1.0f / tex.Size.x;
                f32 texelHeight = 1.0f / tex.Size.y;
                texCoordMin *= BlVec2<f32>(texelWidth, texelHeight);
                texCoordMax *= BlVec2<f32>(texelWidth, texelHeight);

                BlVec4 normalizedColor = NormalizeColor(color);

                // vertices
                glm::vec4 pos = finalTextTransform * glm::vec4(quadMin.x, quadMin.y, 0.0f, 1.0f);
                FontVertex vert = FontVertex(BlVec3<f32>(pos.x, pos.y, pos.z), normalizedColor, BlVec2<f32>(texCoordMin.x, texCoordMax.y));
                Renderer3DState.FontVertices.push_back(vert);
                
                pos = finalTextTransform * glm::vec4(quadMax.x, quadMax.y, 0.0f, 1.0f);
                vert = FontVertex(BlVec3<f32>(pos.x, pos.y, pos.z), normalizedColor, BlVec2<f32>(texCoordMax.x, texCoordMin.y));
                Renderer3DState.FontVertices.push_back(vert);

                pos = finalTextTransform * glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
                vert = FontVertex(BlVec3<f32>(pos.x, pos.y, pos.z), normalizedColor, BlVec2<f32>(texCoordMax.x, texCoordMax.y));
                Renderer3DState.FontVertices.push_back(vert);

                pos = finalTextTransform * glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
                vert = FontVertex(BlVec3<f32>(pos.x, pos.y, pos.z), normalizedColor, BlVec2<f32>(texCoordMin.x, texCoordMin.y));
                Renderer3DState.FontVertices.push_back(vert);

                // indices
                for (u32 i = 0; i < Renderer3DState.QuadIndices.size(); i++) {
                    const u32 vertexCount = Renderer3DState.FontVertexCount;
                    Renderer3DState.FontIndices.push_back(Renderer3DState.QuadIndices[i] + vertexCount);
                }

                Renderer3DState.CurrentFontAtlas = tex;

                Renderer3DState.FontIndexCount += 6;
                Renderer3DState.FontVertexCount += 4;

                currentX += fsScale * (glyph.AdvanceX + params.Kerning);
            }
        }
    }

    void Renderer3D::DrawMesh(const glm::mat4& transform, Mesh& mesh, BlColor color) {
        BlVec4 normalizedColor = NormalizeColor(color);
        f32 texIndex = GetTexIndex(mesh.Texture);

        // vertices
        for (u32 i = 0; i < mesh.Positions.size(); i++) {
            glm::vec4 pos = transform * glm::vec4(mesh.Positions[i].x, mesh.Positions[i].y, mesh.Positions[i].z, 1.0f);
            glm::vec3 normal = glm::mat3(glm::transpose(glm::inverse(transform))) * glm::vec3(mesh.Normals[i].x, mesh.Normals[i].y, mesh.Normals[i].z);
            MeshVertex vert = MeshVertex(BlVec3<f32>(pos.x, pos.y, pos.z), BlVec3<f32>(normal.x, normal.y, normal.z), normalizedColor, mesh.TexCoords[i], texIndex);
            Renderer3DState.MeshVertices.push_back(vert);
        }

        // indices
        for (u32 i = 0; i < mesh.Indices.size(); i++) {
            const u32 vertexCount = Renderer3DState.MeshVertexCount;
            Renderer3DState.MeshIndices.push_back(mesh.Indices[i] + vertexCount);
        }

        Renderer3DState.MeshIndexCount += mesh.Indices.size();
        Renderer3DState.MeshVertexCount += mesh.Positions.size();
    }

    void Renderer3D::DrawModel(const glm::mat4& transform, Model& model, BlColor color) {
        for (u32 i = 0; i < model.MeshCount; ++i) {
            DrawMesh(transform, model.Meshes[i], color);
        }
    }

    void Renderer3D::BindRenderTexture(RenderTexture texture) {
        auto& renderer = BL_APP.GetRenderer();

        renderer.BindRenderTexture(texture);
    }

    void Renderer3D::UnBindRenderTexture() {
        auto& renderer = BL_APP.GetRenderer();

        renderer.UnBindRenderTexture();
    }

    void Renderer3D::SetProjection(SceneCamera camera) {
        Renderer3DState.Camera = camera;
    }

    void Renderer3D::ResetProjection() {
        Renderer3DState.Camera = Renderer3DState.DefaultCamera;
    }

    void Renderer3D::SetDirectionalLight(const DirectionalLight& light) {
        Renderer3DState.DirLight = light;
    }

    void Renderer3D::Render() {
        Renderer3DState.Stats.Vertices = Renderer3DState.MeshVertexCount;
        Renderer3DState.Stats.Indicies = Renderer3DState.MeshIndexCount;

        auto& renderer = BL_APP.GetRenderer();

        // mesh buffer
        if (Renderer3DState.MeshIndexCount > 0) {
            Renderer3DState.MeshDrawBuffer.Vertices = Renderer3DState.MeshVertices.data();
            Renderer3DState.MeshDrawBuffer.VertexCount = Renderer3DState.MeshVertexCount;
            Renderer3DState.MeshDrawBuffer.VertexSize = sizeof(MeshVertex);
                            
            Renderer3DState.MeshDrawBuffer.Indices = Renderer3DState.MeshIndices.data();
            Renderer3DState.MeshDrawBuffer.IndexCount = Renderer3DState.MeshIndexCount;
            Renderer3DState.MeshDrawBuffer.IndexSize = sizeof(u32);

            renderer.SubmitDrawBuffer(Renderer3DState.MeshDrawBuffer);
            
            renderer.SetBufferLayout({
                { 0, ShaderDataType::Float3, "Position" },
                { 1, ShaderDataType::Float3, "Normal"   },
                { 2, ShaderDataType::Float4, "Color"    },
                { 3, ShaderDataType::Float2, "TexCoord" },
                { 4, ShaderDataType::Float,  "TexIndex" }
            });
            
            renderer.BindShader(Renderer3DState.MeshShader);
            for (u32 i = 0; i < Renderer3DState.CurrentTexIndex; i++) {
                renderer.BindTexture(Renderer3DState.CurrentAttachedTextures[i], i);
            }

            int samplers[16]; // opengl texture IDs
            for (u32 i = 0; i < Renderer3DState.CurrentAttachedTextures.size(); i++) {
                samplers[i] = i;
            }

            BlShader shader = Renderer3DState.MeshShader;
            shader.SetMatrix("u_Projection", Renderer3DState.Camera.GetCameraMatrixFloat());
            shader.SetIntArray("u_Textures", 16, samplers);
            shader.SetVec3("u_ViewPos", Renderer3DState.Camera.Transform.Position);
            // Set directional light
            auto& light = Renderer3DState.DirLight;

            shader.SetVec3("u_Light.Direction", BlVec3(light.Direction));
            shader.SetVec3("u_Light.Ambient", BlVec3(light.Ambient.r / 255.0f, light.Ambient.g / 255.0f, light.Ambient.b / 255.0f));
            shader.SetVec3("u_Light.Diffuse", BlVec3(light.Diffuse.r / 255.0f, light.Diffuse.g / 255.0f, light.Diffuse.b / 255.0f));
            shader.SetVec3("u_Light.Specular", BlVec3(light.Specular.r / 255.0f, light.Specular.g / 255.0f, light.Specular.b / 255.0f));

            renderer.DrawIndexed(Renderer3DState.MeshIndexCount);

            renderer.UnBindTexture();

            Renderer3DState.Stats.DrawCalls++;
            Renderer3DState.Stats.ActiveTextures = Renderer3DState.CurrentTexIndex;
            Renderer3DState.Stats.ReservedTextures = 1;

            // clear buffer after rendering
            Renderer3DState.MeshIndices.clear();
            Renderer3DState.MeshVertices.clear();

            // reserve memory again
            Renderer3DState.MeshIndices.reserve(2048);
            Renderer3DState.MeshVertices.reserve(2048);
            Renderer3DState.MeshIndexCount = 0;
            Renderer3DState.MeshVertexCount = 0;
            Renderer3DState.CurrentTexIndex = 1; // 0 is reserved and never changes
        }

        // font buffer
        if (Renderer3DState.FontIndexCount > 0) {
            Renderer3DState.FontDrawBuffer.Vertices = Renderer3DState.FontVertices.data();
            Renderer3DState.FontDrawBuffer.VertexCount = Renderer3DState.FontVertexCount;
            Renderer3DState.FontDrawBuffer.VertexSize = sizeof(FontVertex);
                            
            Renderer3DState.FontDrawBuffer.Indices = Renderer3DState.FontIndices.data();
            Renderer3DState.FontDrawBuffer.IndexCount = Renderer3DState.FontIndexCount;
            Renderer3DState.FontDrawBuffer.IndexSize = sizeof(u32);

            renderer.SubmitDrawBuffer(Renderer3DState.FontDrawBuffer);
            
            renderer.SetBufferLayout({
                { 0, ShaderDataType::Float3, "Position" },
                { 1, ShaderDataType::Float4, "Color"    },
                { 2, ShaderDataType::Float2, "TexCoord" }
            });
            
            renderer.BindShader(Renderer3DState.FontShader);
            renderer.BindTexture(Renderer3DState.CurrentFontAtlas);

            BlShader shader = Renderer3DState.FontShader;
            shader.SetMatrix("u_Projection", Renderer3DState.Camera.GetCameraMatrixFloat());

            renderer.DrawIndexed(Renderer3DState.FontIndexCount);

            renderer.UnBindTexture();

            Renderer3DState.Stats.DrawCalls++;
            // Renderer3DState.Info.ActiveTextures = Renderer3DState.CurrentFontIndex;
            // Renderer3DState.Info.ReservedTextures = 1;

            // clear buffer after rendering
            Renderer3DState.FontIndices.clear();
            Renderer3DState.FontVertices.clear();

            // reserve memory again
            Renderer3DState.FontIndices.reserve(2048);
            Renderer3DState.FontVertices.reserve(2048);
            Renderer3DState.FontIndexCount = 0;
            Renderer3DState.FontVertexCount = 0;
            Renderer3DState.CurrentTexIndex = 0;
        }
    }

    BlVec2<f32> Renderer3D::MeasureText(const std::string& text, Font& font, TextParams parameters) {
        f32 fsScale = 1.0f / (font.Ascender - font.Descender);
        f32 currentX = 0.0f;
        f32 currentY = 0.0f;

        for (u32 c = 0; c < text.length(); c++) {
            auto glyph = font.GetGlyphInfo(text.at(c), 0);
            BlRec& pb = glyph.PlaneRect;

            currentX += fsScale * (glyph.AdvanceX + parameters.Kerning);
            currentY = std::max(currentY, fsScale * (glyph.PlaneRect.y - glyph.PlaneRect.h));
        }

        return BlVec2<f32>(currentX, currentY);
    }

    Renderer3DStats Renderer3D::GetRendererStats() {
        return Renderer3DState.Stats;
    }

} // namespace Blackberry