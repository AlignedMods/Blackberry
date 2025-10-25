#include "rendering.hpp"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Blackberry {

    static void CalculateRotation(BlVec2& vertexPos, BlVec2 pos, f32 rotation) {
        f32 sinR = glm::sin(glm::radians(rotation));
        f32 cosR = glm::cos(glm::radians(rotation));
        
        vertexPos = BlVec2(
            pos.x + (vertexPos.x - pos.x) * cosR - (vertexPos.y - pos.y) * sinR,
            pos.y + (vertexPos.x - pos.x) * sinR + (vertexPos.y - pos.y) * cosR
        );
    }

    void Clear() {
        auto& renderer = BL_APP.GetRenderer();

        renderer.Clear();
    }

    void DrawRectangle(BlVec2 pos, BlVec2 dimensions, BlColor color) {
        DrawRectangle(pos, dimensions, 0.0f, color);
    }

    void DrawRectangle(BlVec2 pos, BlVec2 dimensions, f32 rotation, BlColor color) {
        DrawTexturedQuad(pos, dimensions, nullptr, BlRec(0.0f, 0.0f, 0.0f, 0.0f), rotation, color);
    }

    void DrawTriangle(BlVec2 pos, BlVec2 dimensions, f32 rotation, BlColor color) {
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

    void DrawTriangle(BlVec2 bl, BlVec2 t, BlVec2 br, BlColor color) {
        auto& renderer = BL_APP.GetRenderer();
        renderer.AttachDefaultShader(DefaultShader::Shape);

        renderer.Begin(RenderingMode::Triangles);

        renderer.SubVertex(BlVertex(bl, color, BlVec2(0.0f, 0.0f)));
        renderer.SubVertex(BlVertex(t, color, BlVec2(0.0f, 0.0f)));
        renderer.SubVertex(BlVertex(br, color, BlVec2(0.0f, 0.0f)));

        renderer.End();
    }

    void DrawTexture(BlVec2 pos, BlTexture texture, f32 rotation, BlColor color) {
        DrawTextureEx(pos, BlVec2(static_cast<f32>(texture.Width), static_cast<f32>(texture.Height)), texture, rotation, color);
    }

    void DrawTextureEx(BlVec2 pos, BlVec2 dimensions, BlTexture texture, f32 rotation, BlColor color)  {
        DrawTextureArea(pos, dimensions, BlRec(0.0f, 0.0f, static_cast<f32>(texture.Width), static_cast<f32>(texture.Height)), texture, rotation, color);
    }

    void DrawTextureArea(BlVec2 pos, BlVec2 dimensions, BlRec area, BlTexture texture, f32 rotation, BlColor color) {
        DrawTexturedQuad(pos, dimensions, &texture, area, rotation, color);
    }

    void DrawTexturedQuad(BlVec2 pos, BlVec2 dimensions, BlTexture* texture, BlRec area, f32 rotation, BlColor color) {
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
            CalculateRotation(bl.pos, pos, rotation);
            CalculateRotation(tr.pos, pos, rotation);
            CalculateRotation(br.pos, pos, rotation);
            CalculateRotation(tl.pos, pos, rotation);
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
    }

    void DrawRenderTexture(BlVec2 pos, BlVec2 dimensions, BlRenderTexture texture) {
        DrawTextureArea(pos, dimensions, BlRec(0.0f, 0.0f, texture.Texture.Width, texture.Texture.Height * -1.0f), texture.Texture);
    }

    void AttachRenderTexture(BlRenderTexture texture) {
        auto& renderer = BL_APP.GetRenderer();

        renderer.AttachRenderTexture(texture);
    }

    void DetachRenderTexture() {
        auto& renderer = BL_APP.GetRenderer();

        renderer.DettachRenderTexture();
    }

    void DrawText(const std::string& str, BlVec2 pos, Font& font, u32 size, BlColor color) {
        auto& renderer = BL_APP.GetRenderer();
        BlTexture tex = font.GetTexture();
        
        renderer.AttachDefaultShader(DefaultShader::Font);

        f32 currentX = pos.x;
        f32 currentY = pos.y;
        
        BlColor currentColor = color;
        
        for (u32 c = 0; c < str.length(); c++) {
            if (str.at(c) == '\n') {
                // currentY += font.GetRowHeight();
                currentX = pos.x;
            } else {
                BlGlyphInfo glyph = font.GetGlyphInfo(str.at(c));
        
                // DrawTextureArea(BlVec2(currentX + glyph.Left, currentY - glyph.Top + font.GetAscender(size)), BlVec2(glyph.Rect.w, glyph.Rect.h), glyph.Rect, tex, White);
                DrawTextureArea(BlVec2(currentX + glyph.Left, currentY - glyph.Top), BlVec2(glyph.Rect.w, glyph.Rect.h), glyph.Rect, tex);
                currentX += glyph.AdvanceX + 1;
            }
        }
    }

    BlVec2 MeasureText(const std::string& str, Font& font, u32 size) {
        // f32 x = 0.0f;
        // f32 y = 0.0f;
        // 
        // for (u32 c = 0; c < str.length(); c++) {
        //     if (str.at(c) == '\n') {
        //         y += font.GetRowHeight();
        //         x = 0.0f;
        //     } else {
        //         BlGlyphInfo glyph = font.GetGlyphInfo(str.at(c), size);
        // 
        //         x += glyph.AdvanceX;
        //         y += font.GetRowHeight();
        //     }
        // }
        // 
        // return BlVec2(x, y);
        return BlVec2();
    }

    std::string FileDialogs::OpenFile(const char* filter) {
        return BL_APP.GetWindow().OpenFile(filter);
    }

    std::string Directories::GetAppDataDirectory() {
        auto& window = BL_APP.GetWindow();
        return window.GetAppDataDirectory();
    }

} // namespace Blackberry