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

    void DrawTexture(BlVec2 pos, BlTexture texture) {
        auto& renderer = Application::Get().GetRenderer();

        // BlVertex bl = BlVertex(BlVec2(pos.x, pos.y + texture))

        renderer.AttachTexture(texture);
        renderer.Begin(RenderingMode::Triangles);

        // renderer.SubVertex(BlVertex(BlVec2());

        renderer.End();
        renderer.DettachTexture();
    }

    void DrawTextureEx(BlVec2 pos, BlVec2 dimensions, BlTexture texture) {
        auto& renderer = Application::Get().GetRenderer();

        BlVertex bl = BlVertex(BlVec2(pos.x, pos.y + dimensions.y), BlColor(255, 255, 255, 255), BlVec2(0.0f, 1.0f));
        BlVertex tr = BlVertex(BlVec2(pos.x + dimensions.x, pos.y), BlColor(255, 255, 255, 255), BlVec2(1.0f, 0.0f));
        BlVertex br = BlVertex(BlVec2(pos.x + dimensions.x, pos.y + dimensions.y), BlColor(255, 255, 255, 255), BlVec2(1.0f, 1.0f));
        BlVertex tl = BlVertex(BlVec2(pos.x, pos.y), BlColor(255, 255, 255, 255), BlVec2(0.0f, 0.0f));

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

} // namespace Blackberry