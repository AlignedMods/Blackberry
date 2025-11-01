#include "rendering.hpp"
#include "blackberry/core/util.hpp"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct BlQuadVertex {
    BlVec3 Pos;
    BlVec4 Color;
    BlVec2 TexCoord;
    f32 TexIndex = 0.0f; // OpenGL is weird with passing integers to shaders (also -1 means no texture in shader)
};

namespace Blackberry {

    static u8 s_WhiteTextureData[] = {0xff, 0xff, 0xff, 0xff}; // dead simple white texture (1x1 pixel)

    static const char* s_VertexShaderQuadSource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec3 a_Pos; // BlQuadVertex.Pos (also we can have comments like this because preproccesor ignores them!)
        layout (location = 1) in vec4 a_Color; // BlQuadVertex.Color
        layout (location = 2) in vec2 a_TexCoord; // BlQuadVertex.TexCoord
        layout (location = 3) in float a_TexIndex; // BlQuadVertex.TexIndex

        uniform mat4 u_Projection;

        layout (location = 0) out vec4 o_Color;
        layout (location = 1) out vec2 o_TexCoord;
        layout (location = 2) out float o_TexIndex;

        void main() {
            gl_Position = u_Projection * vec4(a_Pos, 1.0f);

            o_Color = a_Color;
            o_TexCoord = a_TexCoord;
            o_TexIndex = a_TexIndex;
        }
    );

    static const char* s_FragmentShaderQuadSource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec4 a_Color;
        layout (location = 1) in vec2 a_TexCoord;
        layout (location = 2) in float a_TexIndex;

        uniform sampler2D u_Textures[16];

        out vec4 o_FragColor;

        void main() {
            int index = int(a_TexIndex);
            
            vec4 texel = texture(u_Textures[index], a_TexCoord);
            o_FragColor = texel * a_Color;
        }
    );

    struct Renderer2DState {
        // shader
        BlShader QuadShader;

        BlTexture WhiteTexture;

        // Quad buffer data
        u32 QuadIndexCount = 0;
        u32 QuadVertexCount = 0;
        std::vector<BlQuadVertex> QuadVertices;
        std::vector<u32> QuadIndices;
        BlDrawBuffer QuadDrawBuffer;
        // for textured quads
        u32 CurrentTexIndex = 1; // NOTE: 0 is reserved for a blank white texture!!!
        std::array<BlTexture, 16> CurrentAttachedTextures;

        BlRenderer2DInfo Info;
    };

    static Renderer2DState State;

    static void CalculateRotation(BlVec3& vertexPos, BlVec3 pos, f32 rotation) {
        f32 sinR = glm::sin(glm::radians(rotation));
        f32 cosR = glm::cos(glm::radians(rotation));
        
        vertexPos = BlVec3(
            pos.x + (vertexPos.x - pos.x) * cosR - (vertexPos.y - pos.y) * sinR,
            pos.y + (vertexPos.x - pos.x) * sinR + (vertexPos.y - pos.y) * cosR,
            pos.z
        );
    }

    static void SetupQuad(BlVec3 pos, BlVec2 dimensions, f32 rotation, BlColor color, BlVec3* bl, BlVec3* tr, BlVec3* br, BlVec3* tl) {
        BlVec3 _bl = BlVec3(pos.x - dimensions.x / 2.0f, pos.y + dimensions.y / 2.0f, pos.z);
        BlVec3 _tr = BlVec3(pos.x + dimensions.x / 2.0f, pos.y - dimensions.y / 2.0f, pos.z);
        BlVec3 _br = BlVec3(pos.x + dimensions.x / 2.0f, pos.y + dimensions.y / 2.0f, pos.z);
        BlVec3 _tl = BlVec3(pos.x - dimensions.x / 2.0f, pos.y - dimensions.y / 2.0f, pos.z);

        if (rotation != 0.0f) {
            CalculateRotation(_bl, pos, rotation);
            CalculateRotation(_tr, pos, rotation);
            CalculateRotation(_br, pos, rotation);
            CalculateRotation(_tl, pos, rotation);
        }

        *bl = _bl;
        *tr = _tr;
        *br = _br;
        *tl = _tl;
    }

    static void NormalizeColor(BlColor color, BlVec4* outColor) {
        *outColor = BlVec4(
            static_cast<f32>(color.r) / 255.0f,
            static_cast<f32>(color.g) / 255.0f,
            static_cast<f32>(color.b) / 255.0f,
            static_cast<f32>(color.a) / 255.0f
        );
    }

    void Renderer2D::Init() {
        State.QuadShader.Create(s_VertexShaderQuadSource, s_FragmentShaderQuadSource);
        State.WhiteTexture.Create(s_WhiteTextureData, 1, 1, ImageFormat::RGBA8);

        State.CurrentAttachedTextures[0] = State.WhiteTexture; // 0 is reserved for white
    }

    void Renderer2D::Shutdown() {
        State.QuadShader.Delete();
    }

    void Renderer2D::Clear(BlColor color) {
        auto& renderer = BL_APP.GetRenderer();

        renderer.Clear(color);
    }

    void Renderer2D::NewFrame() {
        State.Info.DrawCalls = 0;
        State.Info.Vertices = 0;
        State.Info.Indicies = 0;
        State.Info.ActiveTextures = 0;
    }

    void Renderer2D::DrawRectangle(BlVec3 pos, BlVec2 dimensions, BlColor color) {
        DrawRectangle(pos, dimensions, 0.0f, color);
    }

    void Renderer2D::DrawRectangle(BlVec3 pos, BlVec2 dimensions, f32 rotation, BlColor color) {
        DrawTexturedQuad(pos, dimensions, BlRec(0, 0, 1, 1), State.WhiteTexture, rotation, color);
    }

    void Renderer2D::DrawTriangle(BlVec3 pos, BlVec2 dimensions, f32 rotation, BlColor color) {
        BlVec3 bl = BlVec3(pos.x - dimensions.x / 2.0f, pos.y + dimensions.y / 2.0f, pos.z);
        BlVec3 t = BlVec3(pos.x, pos.y - dimensions.y / 2.0f, pos.z);
        BlVec3 br = BlVec3(pos.x + dimensions.x / 2.0f, pos.y + dimensions.y / 2.0f, pos.z);

        if (rotation != 0.0f) {
            CalculateRotation(bl, pos, rotation);
            CalculateRotation(t, pos, rotation);
            CalculateRotation(br, pos, rotation);
        }

        DrawTriangle(bl, t, br, color);
    }

    void Renderer2D::DrawTriangle(BlVec3 bl, BlVec3 t, BlVec3 br, BlColor color) {
#if 0

        BlVec4 normalizedColor = BlVec4(
            static_cast<f32>(color.r) / 255.0f,
            static_cast<f32>(color.g) / 255.0f,
            static_cast<f32>(color.b) / 255.0f,
            static_cast<f32>(color.a) / 255.0f
        );

        BlShapeVertex vertexBL = BlShapeVertex(bl, normalizedColor);
        BlShapeVertex vertexT = BlShapeVertex(t, normalizedColor);
        BlShapeVertex vertexBR = BlShapeVertex(br, normalizedColor);

        State.ShapeVertices.push_back(vertexBL);
        State.ShapeVertices.push_back(vertexT);
        State.ShapeVertices.push_back(vertexBR);

        State.ShapeIndices.push_back(State.ShapeVertexCount + 0);
        State.ShapeIndices.push_back(State.ShapeVertexCount + 1);
        State.ShapeIndices.push_back(State.ShapeVertexCount + 2);

        State.ShapeIndexCount += 3;
        State.ShapeVertexCount += 3;

#endif
    }

    void Renderer2D::DrawTexture(BlVec3 pos, BlTexture texture, f32 rotation, BlColor color) {
        DrawTextureEx(pos, BlVec2(static_cast<f32>(texture.Width), static_cast<f32>(texture.Height)), texture, rotation, color);
    }

    void Renderer2D::DrawTextureEx(BlVec3 pos, BlVec2 dimensions, BlTexture texture, f32 rotation, BlColor color)  {
        DrawTextureArea(pos, dimensions, BlRec(0.0f, 0.0f, static_cast<f32>(texture.Width), static_cast<f32>(texture.Height)), texture, rotation, color);
    }

    void Renderer2D::DrawTextureArea(BlVec3 pos, BlVec2 dimensions, BlRec area, BlTexture texture, f32 rotation, BlColor color) {
        DrawTexturedQuad(pos, dimensions, area, texture, rotation, color);
    }

    void Renderer2D::DrawTexturedQuad(BlVec3 pos, BlVec2 dimensions, BlRec area, BlTexture texture, f32 rotation, BlColor color) {
        f32 texIndex = 0.0f;

        if (State.CurrentTexIndex >= 16) {
            Render();
        }

        bool texAlreadyExists = false;

        for (u32 i = 0; i < State.CurrentTexIndex; i++) {
            if (texture.ID == State.CurrentAttachedTextures[i].ID) {
                texIndex = static_cast<f32>(i);
                texAlreadyExists = true;

                break;
            }
        }

        if (!texAlreadyExists) {
            texIndex = State.CurrentTexIndex;

            State.CurrentAttachedTextures[State.CurrentTexIndex] = texture;
            State.CurrentTexIndex++;
            texAlreadyExists = true;
        }
        
        BlVec3 bl, tr, br, tl;
        BlVec4 normalizedColor;

        SetupQuad(pos, dimensions, rotation, color, &bl, &tr, &br, &tl);
        NormalizeColor(color, &normalizedColor);

        BlQuadVertex vertexBL = BlQuadVertex(bl, normalizedColor, BlVec2(0.0f, 1.0f), texIndex);
        BlQuadVertex vertexTR = BlQuadVertex(tr, normalizedColor, BlVec2(1.0f, 0.0f), texIndex);
        BlQuadVertex vertexBR = BlQuadVertex(br, normalizedColor, BlVec2(1.0f, 1.0f), texIndex);
        BlQuadVertex vertexTL = BlQuadVertex(tl, normalizedColor, BlVec2(0.0f, 0.0f), texIndex);

        // push all vertices (bl, tr, br, tl)
        State.QuadVertices.push_back(vertexBL);
        State.QuadVertices.push_back(vertexTR);
        State.QuadVertices.push_back(vertexBR);
        State.QuadVertices.push_back(vertexTL);

        // first triangle (bl, tr, br)
        State.QuadIndices.push_back(State.QuadVertexCount + 0);
        State.QuadIndices.push_back(State.QuadVertexCount + 1);
        State.QuadIndices.push_back(State.QuadVertexCount + 2);

        // second triangle (bl, tl, tr)
        State.QuadIndices.push_back(State.QuadVertexCount + 0);
        State.QuadIndices.push_back(State.QuadVertexCount + 3);
        State.QuadIndices.push_back(State.QuadVertexCount + 1);

        State.QuadIndexCount += 6;
        State.QuadVertexCount += 4;
    }

    void Renderer2D::DrawRenderTexture(BlVec3 pos, BlVec2 dimensions, BlRenderTexture texture) {
        DrawTextureArea(pos, dimensions, BlRec(0.0f, 0.0f, texture.Texture.Width, texture.Texture.Height * -1.0f), texture.Texture);
    }

    void Renderer2D::AttachRenderTexture(BlRenderTexture texture) {
        auto& renderer = BL_APP.GetRenderer();

        renderer.AttachRenderTexture(texture);
    }

    void Renderer2D::DetachRenderTexture() {
        auto& renderer = BL_APP.GetRenderer();

        renderer.DetachRenderTexture();
    }

    void Renderer2D::SetProjection(glm::mat4 projection) {
        auto& renderer = BL_APP.GetRenderer();

        renderer.SetProjection(projection);
    }

    void Renderer2D::ResetProjection() {
        auto& renderer = BL_APP.GetRenderer();

        renderer.ResetProjection();
    }

    void Renderer2D::Render() {
        State.Info.Vertices = State.QuadVertexCount;
        State.Info.Indicies = State.QuadIndexCount;

        auto& renderer = BL_APP.GetRenderer();

        // quad buffer
        if (State.QuadIndexCount > 0) {
            BlDrawBufferLayout vertPosLayout;
            vertPosLayout.Index = 0;
            vertPosLayout.Count = 3;
            vertPosLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertPosLayout.Stride = sizeof(BlQuadVertex);
            vertPosLayout.Offset = offsetof(BlQuadVertex, Pos);

            BlDrawBufferLayout vertColorLayout;
            vertColorLayout.Index = 1;
            vertColorLayout.Count = 4;
            vertColorLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertColorLayout.Stride = sizeof(BlQuadVertex);
            vertColorLayout.Offset = offsetof(BlQuadVertex, Color);

            BlDrawBufferLayout vertTexCoordLayout;
            vertTexCoordLayout.Index = 2;
            vertTexCoordLayout.Count = 2;
            vertTexCoordLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertTexCoordLayout.Stride = sizeof(BlQuadVertex);
            vertTexCoordLayout.Offset = offsetof(BlQuadVertex, TexCoord);

            BlDrawBufferLayout vertTexIndexLayout;
            vertTexIndexLayout.Index = 3;
            vertTexIndexLayout.Count = 1;
            vertTexIndexLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertTexIndexLayout.Stride = sizeof(BlQuadVertex);
            vertTexIndexLayout.Offset = offsetof(BlQuadVertex, TexIndex);

            State.QuadDrawBuffer.Vertices = State.QuadVertices.data();
            State.QuadDrawBuffer.VertexCount = State.QuadVertexCount;
            State.QuadDrawBuffer.VertexSize = sizeof(BlQuadVertex);

            State.QuadDrawBuffer.Indices = State.QuadIndices.data();
            State.QuadDrawBuffer.IndexCount = State.QuadIndexCount;
            State.QuadDrawBuffer.IndexSize = sizeof(u32);

            renderer.SubmitDrawBuffer(State.QuadDrawBuffer);
            
            renderer.SetBufferLayout(vertPosLayout);
            renderer.SetBufferLayout(vertColorLayout);
            renderer.SetBufferLayout(vertTexCoordLayout);
            renderer.SetBufferLayout(vertTexIndexLayout);
            
            renderer.BindShader(State.QuadShader);
            for (u32 i = 0; i < State.CurrentTexIndex; i++) {
                renderer.AttachTexture(State.CurrentAttachedTextures[i], i);
            }

            int samplers[16]; // opengl texture IDs
            for (u32 i = 0; i < State.CurrentAttachedTextures.size(); i++) {
                samplers[i] = i;
            }

            BlShader shader = State.QuadShader;
            shader.SetIntArray("u_Textures", 16, samplers);

            renderer.DrawIndexed(State.QuadIndexCount);

            renderer.DetachTexture();

            State.Info.DrawCalls++;
            State.Info.ActiveTextures = State.CurrentTexIndex;

            // clear buffer after rendering
            State.QuadIndices.clear();
            State.QuadVertices.clear();
            State.QuadIndexCount = 0;
            State.QuadVertexCount = 0;
            State.CurrentTexIndex = 1; // 0 is reserved and never changes
        }
    }

    BlRenderer2DInfo Renderer2D::GetRenderingInfo() {
        return State.Info;
    }

} // namespace Blackberry