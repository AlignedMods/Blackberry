#include "rendering.hpp"

namespace Blackberry {

    void DrawRectangle(BlVec2 pos, BlVec2 dimensions, const BlColor& color) {
        auto& renderer = Application::Get().GetRenderer();

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
        auto& renderer = Application::Get().GetRenderer();

        renderer.Begin(RenderingMode::Triangles);

        renderer.SubVertex(BlVertex(bl, color, BlVec2(0.0f, 0.0f)));
        renderer.SubVertex(BlVertex(t, color, BlVec2(0.0f, 0.0f)));
        renderer.SubVertex(BlVertex(br, color, BlVec2(0.0f, 0.0f)));

        renderer.End();
    }

    BlTexture LoadTextureFromImage(const Blackberry::Image& image) {
        auto& renderer = Application::Get().GetRenderer();

        return renderer.GenTexture(image);
    }

    void DrawTexture(BlVec2 pos, BlTexture texture, BlColor color) {
        auto& renderer = Application::Get().GetRenderer();

        // BlVertex bl = BlVertex(BlVec2(pos.x, pos.y + texture))

        renderer.AttachTexture(texture);
        renderer.Begin(RenderingMode::Triangles);

        DrawTextureEx(pos, GetTextureSize(texture), texture, color);

        renderer.End();
        renderer.DettachTexture();
    }

    void DrawTextureEx(BlVec2 pos, BlVec2 dimensions, BlTexture texture, BlColor color) {
        DrawTextureArea(pos, dimensions, BlRec(0.0f, 0.0f, GetTextureWidth(texture), GetTextureHeight(texture)), texture, color);
    }

    void DrawTextureArea(BlVec2 pos, BlVec2 dimensions, BlRec area, BlTexture texture, BlColor color) {
        auto& renderer = Application::Get().GetRenderer();

        BlVec2 texSize = GetTextureSize(texture);

        BlVertex bl = BlVertex(BlVec2(pos.x, pos.y + dimensions.y), color, BlVec2(area.x / texSize.x, (area.h + area.y) / texSize.y));
        BlVertex tr = BlVertex(BlVec2(pos.x + dimensions.x, pos.y), color, BlVec2((area.w + area.x) / texSize.x, area.y /texSize.y));
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

    BlVec2 GetTextureSize(BlTexture texture) {
        auto& renderer = Application::Get().GetRenderer();

        return renderer.GetTexDims(texture);
    }

    u32 GetTextureWidth(BlTexture texture) {
        return GetTextureSize(texture).x;
    }

    u32 GetTextureHeight(BlTexture texture) {
        return GetTextureSize(texture).y;
    }

    void DrawText(const std::string& str, BlVec2 pos, Font& font, u32 size, BlColor color) {
        BlTexture tex = font.GetTexture(size);
        f32 currentX = pos.x;
        f32 currentY = pos.y;

        BlColor currentColor = color;

        for (u32 c = 0; c < str.length(); c++) {
            BlGlyphInfo glyph = font.GetGlyphInfo(str.at(c), size);

            DrawTextureArea(BlVec2(currentX + glyph.Left, currentY - glyph.Top), BlVec2(glyph.Rect.w, glyph.Rect.h), glyph.Rect, tex, White);
            currentX += glyph.AdvanceX + 1;
        }
    }

} // namespace Blackberry