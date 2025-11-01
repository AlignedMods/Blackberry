#include "blackberry/application/application.hpp"
#include "blackberry/core/types.hpp"
#include "blackberry/font/font.hpp"

#include <string>

struct BlRenderer2DInfo {
    u32 DrawCalls = 0;
    u32 Vertices = 0;
    u32 Indicies = 0;
    u32 ActiveTextures = 0;
};

namespace Blackberry {

    namespace Colors
    {

        inline BlColor White = BlColor(255, 255, 255, 255);
        inline BlColor Black = BlColor(0, 0, 0, 255);
        inline BlColor Red = BlColor(225, 25, 25, 255);
        inline BlColor Blue = BlColor(25, 25, 225, 255);

    } // namespace Colors

    class Renderer2D {
    public:
        static void Clear(BlColor = Colors::White);

        static void NewFrame();

        static void DrawRectangle(BlVec3 pos, BlVec2 dimensions, BlColor color);
        static void DrawRectangle(BlVec3 pos, BlVec2 dimensions, f32 rotation, BlColor color);

        static void DrawTriangle(BlVec3 pos, BlVec2 dimensions, f32 rotation = 0.0f, BlColor color = Colors::White);
        static void DrawTriangle(BlVec3 bl, BlVec3 t, BlVec3 br, BlColor color);

        static void DrawTexture(BlVec3 pos, BlTexture texture, f32 rotation = 0.0f, BlColor color = Colors::White);
        static void DrawTextureEx(BlVec3 pos, BlVec2 dimensions, BlTexture texture, f32 rotation = 0.0f, BlColor color = Colors::White);
        static void DrawTextureArea(BlVec3 pos, BlVec2 dimensions, BlRec area, BlTexture texture, f32 rotation = 0.0f, BlColor color = Colors::White);

        static void DrawRenderTexture(BlVec3 pos, BlVec2 dimensions, BlRenderTexture texture);

        static void AttachRenderTexture(BlRenderTexture texture);
        static void DetachRenderTexture();

        static void SetProjection(glm::mat4 projection);
        static void ResetProjection();

        static void Render();

        static BlRenderer2DInfo GetRenderingInfo();
    };

    // font rendering and utilities
    // void DrawText(const std::string& str, BlVec2 pos, Font& font, u32 size, BlColor color = White);
    // BlVec2 MeasureText(const std::string& str, Font& font, u32 size);

} // namespace Blackberry