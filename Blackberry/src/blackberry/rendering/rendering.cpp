#include "rendering.hpp"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct BlShapeVertex {
    BlVec3 Pos;
    BlVec4 Color;
};

struct BlTextureVertex {
    BlVec3 Pos;
    BlVec4 Color;
    BlVec2 TexCoord;
    f32 TexIndex = 0.0f; // OpenGL is weird with passing integers to shaders
};

namespace Blackberry {

    struct Renderer2DState {
        // Shape buffer data
        u32 ShapeIndexCount = 0;
        u32 ShapeVertexCount = 0;
        std::vector<BlShapeVertex> ShapeVertices;
        std::vector<u32> ShapeIndices;
        BlDrawBuffer ShapeDrawBuffer; // buffer for shapes (rectangles, triangles, etc)

        // Texture buffer data
        u32 TextureIndexCount = 0;
        u32 TextureVertexCount = 0;
        std::vector<BlTextureVertex> TextureVertices;
        std::vector<u32> TextureIndices;
        BlDrawBuffer TextureDrawBuffer; // buffer for textures
        // used for batching
        u32 CurrentTexIndex = 0;
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
        BlVec3 bl, tr, br, tl;
        BlVec4 normalizedColor;

        SetupQuad(pos, dimensions, rotation, color, &bl, &tr, &br, &tl);
        NormalizeColor(color, &normalizedColor);

        BlShapeVertex vertexBL = BlShapeVertex(bl, normalizedColor);
        BlShapeVertex vertexTR = BlShapeVertex(tr, normalizedColor);
        BlShapeVertex vertexBR = BlShapeVertex(br, normalizedColor);
        BlShapeVertex vertexTL = BlShapeVertex(tl, normalizedColor);

        // push all vertices (bl, tr, br, tl)
        State.ShapeVertices.push_back(vertexBL);
        State.ShapeVertices.push_back(vertexTR);
        State.ShapeVertices.push_back(vertexBR);
        State.ShapeVertices.push_back(vertexTL);

        // first triangle (bl, tr, br)
        State.ShapeIndices.push_back(State.ShapeVertexCount + 0);
        State.ShapeIndices.push_back(State.ShapeVertexCount + 1);
        State.ShapeIndices.push_back(State.ShapeVertexCount + 2);

        // second triangle (bl, tl, tr)
        State.ShapeIndices.push_back(State.ShapeVertexCount + 0);
        State.ShapeIndices.push_back(State.ShapeVertexCount + 3);
        State.ShapeIndices.push_back(State.ShapeVertexCount + 1);

        State.ShapeIndexCount += 6;
        State.ShapeVertexCount += 4;
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
    }

    void Renderer2D::DrawTexture(BlVec3 pos, BlTexture texture, f32 rotation, BlColor color) {
        DrawTextureEx(pos, BlVec2(static_cast<f32>(texture.Width), static_cast<f32>(texture.Height)), texture, rotation, color);
    }

    void Renderer2D::DrawTextureEx(BlVec3 pos, BlVec2 dimensions, BlTexture texture, f32 rotation, BlColor color)  {
        DrawTextureArea(pos, dimensions, BlRec(0.0f, 0.0f, static_cast<f32>(texture.Width), static_cast<f32>(texture.Height)), texture, rotation, color);
    }

    void Renderer2D::DrawTextureArea(BlVec3 pos, BlVec2 dimensions, BlRec area, BlTexture texture, f32 rotation, BlColor color) {
        if (State.CurrentTexIndex >= 16) {
            Render();
        }

        f32 texIndex = 0.0f;
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

        BlTextureVertex vertexBL = BlTextureVertex(bl, normalizedColor, BlVec2(0.0f, 1.0f), texIndex);
        BlTextureVertex vertexTR = BlTextureVertex(tr, normalizedColor, BlVec2(1.0f, 0.0f), texIndex);
        BlTextureVertex vertexBR = BlTextureVertex(br, normalizedColor, BlVec2(1.0f, 1.0f), texIndex);
        BlTextureVertex vertexTL = BlTextureVertex(tl, normalizedColor, BlVec2(0.0f, 0.0f), texIndex);

        // push all vertices (bl, tr, br, tl)
        State.TextureVertices.push_back(vertexBL);
        State.TextureVertices.push_back(vertexTR);
        State.TextureVertices.push_back(vertexBR);
        State.TextureVertices.push_back(vertexTL);

        // first triangle (bl, tr, br)
        State.TextureIndices.push_back(State.TextureVertexCount + 0);
        State.TextureIndices.push_back(State.TextureVertexCount + 1);
        State.TextureIndices.push_back(State.TextureVertexCount + 2);

        // second triangle (bl, tl, tr)
        State.TextureIndices.push_back(State.TextureVertexCount + 0);
        State.TextureIndices.push_back(State.TextureVertexCount + 3);
        State.TextureIndices.push_back(State.TextureVertexCount + 1);

        State.TextureIndexCount += 6;
        State.TextureVertexCount += 4;
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
        State.Info.Vertices = State.ShapeVertexCount + State.TextureVertexCount;
        State.Info.Indicies = State.ShapeIndexCount + State.TextureIndexCount;

        auto& renderer = BL_APP.GetRenderer();
        // draw shape buffer
        if (State.ShapeIndexCount > 0) {
            BlDrawBufferLayout vertPosLayout;
            vertPosLayout.Index = 0;
            vertPosLayout.Count = 3;
            vertPosLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertPosLayout.Stride = sizeof(BlShapeVertex);
            vertPosLayout.Offset = offsetof(BlShapeVertex, Pos);

            BlDrawBufferLayout vertColorLayout;
            vertColorLayout.Index = 1;
            vertColorLayout.Count = 4;
            vertColorLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertColorLayout.Stride = sizeof(BlShapeVertex);
            vertColorLayout.Offset = offsetof(BlShapeVertex, Color);

            State.ShapeDrawBuffer.Vertices = State.ShapeVertices.data();
            State.ShapeDrawBuffer.VertexCount = State.ShapeVertexCount;
            State.ShapeDrawBuffer.VertexSize = sizeof(BlShapeVertex);

            State.ShapeDrawBuffer.Indices = State.ShapeIndices.data();
            State.ShapeDrawBuffer.IndexCount = State.ShapeIndexCount;
            State.ShapeDrawBuffer.IndexSize = sizeof(u32);
            
            renderer.SubmitDrawBuffer(State.ShapeDrawBuffer);
            
            renderer.SetBufferLayout(vertPosLayout);
            renderer.SetBufferLayout(vertColorLayout);
            
            renderer.BindDefaultShader(DefaultShader::Shape);

            renderer.DrawIndexed(State.ShapeIndexCount);

            State.Info.DrawCalls++;

            // clear buffer after rendering
            State.ShapeIndices.clear();
            State.ShapeVertices.clear();
            State.ShapeIndexCount = 0;
            State.ShapeVertexCount = 0;
        }

        if (State.TextureIndexCount > 0) {
            BlDrawBufferLayout vertPosLayout;
            vertPosLayout.Index = 0;
            vertPosLayout.Count = 3;
            vertPosLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertPosLayout.Stride = sizeof(BlTextureVertex);
            vertPosLayout.Offset = offsetof(BlTextureVertex, Pos);

            BlDrawBufferLayout vertColorLayout;
            vertColorLayout.Index = 1;
            vertColorLayout.Count = 4;
            vertColorLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertColorLayout.Stride = sizeof(BlTextureVertex);
            vertColorLayout.Offset = offsetof(BlTextureVertex, Color);

            BlDrawBufferLayout vertTexCoordLayout;
            vertTexCoordLayout.Index = 2;
            vertTexCoordLayout.Count = 2;
            vertTexCoordLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertTexCoordLayout.Stride = sizeof(BlTextureVertex);
            vertTexCoordLayout.Offset = offsetof(BlTextureVertex, TexCoord);

            BlDrawBufferLayout vertTexIndexLayout;
            vertTexIndexLayout.Index = 3;
            vertTexIndexLayout.Count = 1;
            vertTexIndexLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertTexIndexLayout.Stride = sizeof(BlTextureVertex);
            vertTexIndexLayout.Offset = offsetof(BlTextureVertex, TexIndex);

            State.TextureDrawBuffer.Vertices = State.TextureVertices.data();
            State.TextureDrawBuffer.VertexCount = State.TextureVertexCount;
            State.TextureDrawBuffer.VertexSize = sizeof(BlTextureVertex);

            State.TextureDrawBuffer.Indices = State.TextureIndices.data();
            State.TextureDrawBuffer.IndexCount = State.TextureIndexCount;
            State.TextureDrawBuffer.IndexSize = sizeof(u32);

            renderer.SubmitDrawBuffer(State.TextureDrawBuffer);
            
            renderer.SetBufferLayout(vertPosLayout);
            renderer.SetBufferLayout(vertColorLayout);
            renderer.SetBufferLayout(vertTexCoordLayout);
            renderer.SetBufferLayout(vertTexIndexLayout);
            
            renderer.BindDefaultShader(DefaultShader::Texture);
            for (u32 i = 0; i < State.CurrentTexIndex; i++) {
                renderer.AttachTexture(State.CurrentAttachedTextures[i], i);
            }

            int samplers[16]; // opengl texture IDs
            for (u32 i = 0; i < State.CurrentAttachedTextures.size(); i++) {
                samplers[i] = i;
            }

            BlShader shader = renderer.GetDefaultShader(DefaultShader::Texture);
            shader.SetIntArray("u_Textures", 16, samplers);

            renderer.DrawIndexed(State.TextureIndexCount);

            renderer.DetachTexture();

            State.Info.DrawCalls++;
            State.Info.ActiveTextures = State.CurrentTexIndex;

            // clear buffer after rendering
            State.TextureIndices.clear();
            State.TextureVertices.clear();
            State.TextureIndexCount = 0;
            State.TextureVertexCount = 0;
            State.CurrentTexIndex = 0;
        }
    }

    BlRenderer2DInfo Renderer2D::GetRenderingInfo() {
        return State.Info;
    }

} // namespace Blackberry