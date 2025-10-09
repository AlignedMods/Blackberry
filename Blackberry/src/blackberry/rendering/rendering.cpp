#include "rendering.hpp"

namespace Blackberry {

    void Clear() {
        auto& renderer = BL_APP.GetRenderer();

        renderer.Clear();
    }

    void DrawRectangle(BlVec2 pos, BlVec2 dimensions, const BlColor& color) {
        auto& renderer = BL_APP.GetRenderer();

        BlVertex bl = BlVertex(BlVec2(pos.x, pos.y + dimensions.y), color, BlVec2(0.0f, 1.0f));
        BlVertex tr = BlVertex(BlVec2(pos.x + dimensions.x, pos.y), color, BlVec2(1.0f, 0.0f));
        BlVertex br = BlVertex(BlVec2(pos.x + dimensions.x, pos.y + dimensions.y), color, BlVec2(1.0f, 1.0f));
        BlVertex tl = BlVertex(BlVec2(pos.x, pos.y), color, BlVec2(0.0f, 0.0f));

        renderer.Begin(RenderingMode::Triangles);

        renderer.SubVertex(bl);
        renderer.SubVertex(tr);
        renderer.SubVertex(br);

        renderer.SubVertex(bl);
        renderer.SubVertex(tl);
        renderer.SubVertex(tr);

        renderer.End();
    }

    void DrawTriangle(BlVec2 bl, BlVec2 t, BlVec2 br, const BlColor& color) {
        auto& renderer = BL_APP.GetRenderer();

        renderer.Begin(RenderingMode::Triangles);

        renderer.SubVertex(BlVertex(bl, color, BlVec2(0.0f, 0.0f)));
        renderer.SubVertex(BlVertex(t, color, BlVec2(0.0f, 0.0f)));
        renderer.SubVertex(BlVertex(br, color, BlVec2(0.0f, 0.0f)));

        renderer.End();
    }

    void DrawTexture(BlVec2 pos, BlTexture texture, BlColor color) {
        DrawTextureEx(pos, BlVec2(static_cast<f32>(texture.Width), static_cast<f32>(texture.Height)), texture, color);
    }

    void DrawTextureEx(BlVec2 pos, BlVec2 dimensions, BlTexture texture, BlColor color) {
        DrawTextureArea(pos, dimensions, BlRec(0.0f, 0.0f, static_cast<f32>(texture.Width), static_cast<f32>(texture.Height)), texture, color);
    }

    void DrawTextureArea(BlVec2 pos, BlVec2 dimensions, BlRec area, BlTexture texture, BlColor color) {
        auto& renderer = BL_APP.GetRenderer();

        BlVec2 texSize = BlVec2(static_cast<f32>(texture.Width), static_cast<f32>(texture.Height));

        BlVertex bl = BlVertex(BlVec2(pos.x, pos.y + dimensions.y), color, BlVec2(area.x / texSize.x, (area.h + area.y) / texSize.y));
        BlVertex tr = BlVertex(BlVec2(pos.x + dimensions.x, pos.y), color, BlVec2((area.w + area.x) / texSize.x, area.y / texSize.y));
        BlVertex br = BlVertex(BlVec2(pos.x + dimensions.x, pos.y + dimensions.y), color, BlVec2((area.w + area.x) / texSize.x, (area.h + area.y) / texSize.y));
        BlVertex tl = BlVertex(BlVec2(pos.x, pos.y), color, BlVec2(area.x / texSize.x, area.y / texSize.y));

        renderer.AttachTexture(texture);
        renderer.Begin(RenderingMode::Triangles);

        renderer.SubVertex(bl);
        renderer.SubVertex(tr);
        renderer.SubVertex(br);

        renderer.SubVertex(bl);
        renderer.SubVertex(tl);
        renderer.SubVertex(tr);

        renderer.End();
        renderer.DettachTexture();
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
        BlTexture tex = font.GetTexture(size);
        f32 currentX = pos.x;
        f32 currentY = pos.y;

        BlColor currentColor = color;

        for (u32 c = 0; c < str.length(); c++) {
            if (str.at(c) == '\n') {
                currentY += font.GetRowHeight();
                currentX = pos.x;
            } else {
                BlGlyphInfo glyph = font.GetGlyphInfo(str.at(c), size);

                DrawTextureArea(BlVec2(currentX + glyph.Left, currentY - glyph.Top + font.GetAscender(size)), BlVec2(glyph.Rect.w, glyph.Rect.h), glyph.Rect, tex, White);
                currentX += glyph.AdvanceX + 1;
            }
        }
    }

    BlVec2 MeasureText(const std::string& str, Font& font, u32 size) {
        f32 x = 0.0f;
        f32 y = 0.0f;

        for (u32 c = 0; c < str.length(); c++) {
            if (str.at(c) == '\n') {
                y += font.GetRowHeight();
                x = 0.0f;
            } else {
                BlGlyphInfo glyph = font.GetGlyphInfo(str.at(c), size);

                x += glyph.AdvanceX;
                y += font.GetRowHeight();
            }
        }

        return BlVec2(x, y);
    }

} // namespace Blackberry