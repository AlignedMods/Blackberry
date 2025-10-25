#include "rendering.hpp"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct BlShapeVertex {
    BlVec2 Pos;
    BlVec4 Color;
};

namespace Blackberry {

    static u32 CurrentIndex;
    static u32 CurrentVertex;

    static std::vector<BlShapeVertex> ShapeVertices;

    static BlDrawBuffer ShapeDrawBuffer; // buffer for shapes (rectangles, triangles, etc)

    static void CalculateRotation(BlVec2& vertexPos, BlVec2 pos, f32 rotation) {
        f32 sinR = glm::sin(glm::radians(rotation));
        f32 cosR = glm::cos(glm::radians(rotation));
        
        vertexPos = BlVec2(
            pos.x + (vertexPos.x - pos.x) * cosR - (vertexPos.y - pos.y) * sinR,
            pos.y + (vertexPos.x - pos.x) * sinR + (vertexPos.y - pos.y) * cosR
        );
    }

    void Renderer2D::Clear(BlColor color) {
        auto& renderer = BL_APP.GetRenderer();

        renderer.Clear(color);
    }

    void Renderer2D::DrawRectangle(BlVec2 pos, BlVec2 dimensions, BlColor color) {
        DrawRectangle(pos, dimensions, 0.0f, color);
    }

    void Renderer2D::DrawRectangle(BlVec2 pos, BlVec2 dimensions, f32 rotation, BlColor color) {
        DrawTexturedQuad(pos, dimensions, nullptr, BlRec(0.0f, 0.0f, 0.0f, 0.0f), rotation, color);
    }

    void Renderer2D::DrawTriangle(BlVec2 pos, BlVec2 dimensions, f32 rotation, BlColor color) {
        BlVec2 bl = BlVec2(pos.x - dimensions.x / 2.0f, pos.y + dimensions.y / 2.0f);
        BlVec2 t = BlVec2(pos.x, pos.y - dimensions.y / 2.0f);
        BlVec2 br = BlVec2(pos.x + dimensions.x / 2.0f, pos.y + dimensions.y / 2.0f);

        if (rotation != 0.0f) {
            CalculateRotation(bl, pos, rotation);
            CalculateRotation(t, pos, rotation);
            CalculateRotation(br, pos, rotation);
        }

        DrawTriangle(bl, t, br, color);
    }

    void Renderer2D::DrawTriangle(BlVec2 bl, BlVec2 t, BlVec2 br, BlColor color) {
        BlVec4 normalizedColor = BlVec4(
            static_cast<f32>(color.r) / 255.0f,
            static_cast<f32>(color.g) / 255.0f,
            static_cast<f32>(color.b) / 255.0f,
            static_cast<f32>(color.a) / 255.0f
        );

        BlShapeVertex vertexBL = BlShapeVertex(bl, normalizedColor);
        BlShapeVertex vertexT = BlShapeVertex(t, normalizedColor);
        BlShapeVertex vertexBR = BlShapeVertex(br, normalizedColor);

        // BlShapeVertex vertexBL = BlShapeVertex(BlVec2(-0.5f, -0.5f), BlVec4(1.0f, 0.0f, 0.0f, 1.0f));
        // BlShapeVertex vertexT = BlShapeVertex(BlVec2(0.0f, 0.5f), BlVec4(1.0f, 0.0f, 0.0f, 1.0f));
        // BlShapeVertex vertexBR = BlShapeVertex(BlVec2(0.5f, -0.5f), BlVec4(1.0f, 0.0f, 0.0f, 1.0f));

        ShapeVertices.push_back(vertexBL);
        ShapeVertices.push_back(vertexT);
        ShapeVertices.push_back(vertexBR);

        ShapeDrawBuffer.Indices.push_back(CurrentIndex + 0);
        ShapeDrawBuffer.Indices.push_back(CurrentIndex + 1);
        ShapeDrawBuffer.Indices.push_back(CurrentIndex + 2);

        CurrentIndex += 3;
    }

    void Renderer2D::DrawTexture(BlVec2 pos, BlTexture texture, f32 rotation, BlColor color) {
        DrawTextureEx(pos, BlVec2(static_cast<f32>(texture.Width), static_cast<f32>(texture.Height)), texture, rotation, color);
    }

    void Renderer2D::DrawTextureEx(BlVec2 pos, BlVec2 dimensions, BlTexture texture, f32 rotation, BlColor color)  {
        DrawTextureArea(pos, dimensions, BlRec(0.0f, 0.0f, static_cast<f32>(texture.Width), static_cast<f32>(texture.Height)), texture, rotation, color);
    }

    void Renderer2D::DrawTextureArea(BlVec2 pos, BlVec2 dimensions, BlRec area, BlTexture texture, f32 rotation, BlColor color) {
        DrawTexturedQuad(pos, dimensions, &texture, area, rotation, color);
    }

    void Renderer2D::DrawTexturedQuad(BlVec2 pos, BlVec2 dimensions, BlTexture* texture, BlRec area, f32 rotation, BlColor color) {
#if 0

        auto& renderer = BL_APP.GetRenderer();

        BlVec2 texSize;
        
        if (texture) {
            texSize = BlVec2(static_cast<f32>(texture->Width), static_cast<f32>(texture->Height));
        }

        BlVertex bl = BlVertex(BlVec2(pos.x - dimensions.x / 2.0f, pos.y + dimensions.y / 2.0f), color, BlVec2(area.x / texSize.x, (area.h + area.y) / texSize.y));
        BlVertex tr = BlVertex(BlVec2(pos.x + dimensions.x / 2.0f, pos.y - dimensions.y / 2.0f), color, BlVec2((area.w + area.x) / texSize.x, area.y / texSize.y));
        BlVertex br = BlVertex(BlVec2(pos.x + dimensions.x / 2.0f, pos.y + dimensions.y / 2.0f), color, BlVec2((area.w + area.x) / texSize.x, (area.h + area.y) / texSize.y));
        BlVertex tl = BlVertex(BlVec2(pos.x - dimensions.x / 2.0f, pos.y - dimensions.y / 2.0f), color, BlVec2(area.x / texSize.x, area.y / texSize.y));

        if (rotation != 0.0f) {
            CalculateRotation(bl.Pos, pos, rotation);
            CalculateRotation(tr.Pos, pos, rotation);
            CalculateRotation(br.Pos, pos, rotation);
            CalculateRotation(tl.Pos, pos, rotation);
        }

        if (texture) {
            renderer.AttachTexture(*texture);
            renderer.AttachDefaultShader(DefaultShader::Texture);
        } else {
            renderer.AttachDefaultShader(DefaultShader::Shape);
        }

        renderer.Begin(RenderingMode::Triangles);

        renderer.SubVertex(bl);
        renderer.SubVertex(tr);
        renderer.SubVertex(br);

        renderer.SubVertex(bl);
        renderer.SubVertex(tl);
        renderer.SubVertex(tr);

        renderer.End();

        if (texture) {
            renderer.DettachTexture();
        }

#endif
    }

    void Renderer2D::DrawRenderTexture(BlVec2 pos, BlVec2 dimensions, BlRenderTexture texture) {
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

    void Renderer2D::Render() {
        auto& renderer = BL_APP.GetRenderer();
        // draw shape buffer
        if (!ShapeDrawBuffer.Indices.empty()) {
            BlDrawBufferLayout vertPosLayout;
            vertPosLayout.Index = 0;
            vertPosLayout.Count = 2;
            vertPosLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertPosLayout.Stride = sizeof(BlShapeVertex);
            vertPosLayout.Offset = 0;

            BlDrawBufferLayout vertColorLayout;
            vertColorLayout.Index = 1;
            vertColorLayout.Count = 4;
            vertColorLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertColorLayout.Stride = sizeof(BlShapeVertex);
            vertColorLayout.Offset = sizeof(BlShapeVertex::Pos);

            static f32 triangle[] = {
                -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
                 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
                 0.0f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f
            };

            ShapeDrawBuffer.Vertices = ShapeVertices.data();
            ShapeDrawBuffer.VertexCount = static_cast<u32>(ShapeVertices.size());
            ShapeDrawBuffer.VertexSize = sizeof(BlShapeVertex);

            // ShapeDrawBuffer.Vertices = triangle;
            // ShapeDrawBuffer.VertexCount = 3;
            // ShapeDrawBuffer.VertexSize = sizeof(f32) * 6;
            
            renderer.SubmitDrawBuffer(ShapeDrawBuffer);
            
            renderer.SetBufferLayout(vertPosLayout);
            renderer.SetBufferLayout(vertColorLayout);
            
            renderer.AttachDefaultShader(DefaultShader::Shape);

            renderer.DrawIndexed(CurrentIndex);

            // clear buffer after rendering
            ShapeDrawBuffer.Indices.clear();
            ShapeVertices.clear();
            CurrentIndex = 0;
        }
    }

    std::string FileDialogs::OpenFile(const char* filter) {
        return BL_APP.GetWindow().OpenFile(filter);
    }

    std::string Directories::GetAppDataDirectory() {
        auto& window = BL_APP.GetWindow();
        return window.GetAppDataDirectory();
    }

} // namespace Blackberry