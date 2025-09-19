#include "rendering.hpp"

namespace Blackberry {

    void DrawRectangle(BlVec2 pos, BlVec2 dimensions, const BlColor& color) {
        auto& renderer = Application::Get().GetRenderer();

        renderer.Begin(RenderingMode::Triangles);

        renderer.SetColor(color);
        // first triangle
        renderer.Vertex2f(pos.x, pos.y + dimensions.y);
        renderer.Vertex2f(pos.x + dimensions.x, pos.y);
        renderer.Vertex2f(pos.x + dimensions.x, pos.y + dimensions.y);

        // second triangle

        renderer.Vertex2f(pos.x, pos.y + dimensions.y);
        renderer.Vertex2f(pos.x, pos.y);
        renderer.Vertex2f(pos.x + dimensions.x, pos.y);

        renderer.End();
    }

    void DrawTriangle(BlVec2 bl, BlVec2 t, BlVec2 br, const BlColor& color) {
        auto& renderer = Application::Get().GetRenderer();

        renderer.Begin(RenderingMode::Triangles);

        renderer.SetColor(color);
        renderer.Vertex2f(bl.x, bl.y);
        renderer.Vertex2f(t.x, t.y);
        renderer.Vertex2f(br.x, br.y);

        renderer.End();
    }

} // namespace Blackberry