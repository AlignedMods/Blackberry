#include "rendering.hpp"

namespace Blackberry {

    void DrawRectangle(BlVec2 pos, BlVec2 dimensions, const BlColor& color) {
        auto& renderer = Application::Get().GetRenderer();

        renderer.Begin(RenderingMode::Triangles);

        // renderer.SetColor(color);
        // // first triangle
        // renderer.Vertex2f(pos.x, pos.y + dimensions.y);
        // renderer.Vertex2f(pos.x + dimensions.x, pos.y);
        // renderer.Vertex2f(pos.x + dimensions.x, pos.y + dimensions.y);
        // 
        // // second triangle
        // 
        // renderer.Vertex2f(pos.x, pos.y + dimensions.y);
        // renderer.Vertex2f(pos.x, pos.y);
        // renderer.Vertex2f(pos.x + dimensions.x, pos.y);

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

} // namespace Blackberry