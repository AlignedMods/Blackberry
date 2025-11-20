#include "blackberry/application/application.hpp"
#include "blackberry/core/types.hpp"
#include "blackberry/font/font.hpp"
#include "blackberry/scene/camera.hpp"

#include <string>

#undef DrawText // windows i will genuenly kill you

struct BlRenderer2DInfo {
    u32 DrawCalls = 0;
    u32 Vertices = 0;
    u32 Indicies = 0;
    u32 ActiveTextures = 0;
    u32 ReservedTextures = 0;
};

namespace Blackberry {

    namespace Colors
    {

        inline BlColor White = BlColor(255, 255, 255, 255);
        inline BlColor Black = BlColor(0, 0, 0, 255);
        inline BlColor Red = BlColor(225, 25, 25, 255);
        inline BlColor Blue = BlColor(25, 25, 225, 255);

    } // namespace Colors

    struct TextParams {
        f32 Kerning = 0.0f;
        f32 LineSpacing = 0.0f;
    };

    class Renderer2D {
    public:
        static void Init();
        static void Shutdown();

        static void Clear(BlColor = Colors::White);

        static void NewFrame();

        static void DrawRectangle(BlVec3<f32> pos, BlVec2<f32> dimensions, BlColor color);
        static void DrawRectangle(BlVec3<f32> pos, BlVec2<f32> dimensions, f32 rotation, BlColor color);
        static void DrawRectangle(const glm::mat4& transform, BlColor color);

        static void DrawTriangle(BlVec3<f32> pos, BlVec2<f32> dimensions, f32 rotation = 0.0f, BlColor color = Colors::White);
        static void DrawTriangle(const glm::mat4& transform, BlColor color);

        static void DrawCircle(BlVec3<f32> pos, f32 radius, BlColor color = Colors::White);
        static void DrawElipse(const glm::mat4& transform, BlColor color = Colors::White);

        static void DrawTexture(BlVec3<f32> pos, Texture2D texture, f32 rotation = 0.0f, BlColor color = Colors::White);
        static void DrawTextureEx(BlVec3<f32> pos, BlVec2<f32> dimensions, Texture2D texture, f32 rotation = 0.0f, BlColor color = Colors::White);
        static void DrawTextureArea(BlVec3<f32> pos, BlVec2<f32> dimensions, BlRec area, Texture2D texture, f32 rotation = 0.0f, BlColor color = Colors::White);

        static void DrawText(BlVec3<f32> pos, f32 fontSize, const std::string& text, Font& font, TextParams params = TextParams{}, BlColor color = Colors::White);
        static void DrawText(const glm::mat4& transform, const std::string& text, Font& font, TextParams params = TextParams{}, BlColor color = Colors::White);

        static void DrawTexturedQuad(const glm::mat4& transform, BlRec area, Texture2D texture, BlColor color);
        static void DrawTexturedTriangle(const glm::mat4& transform, BlRec area, Texture2D texture, BlColor color);

        static void DrawRenderTexture(BlVec3<f32> pos, BlVec2<f32> dimensions, RenderTexture texture);

        static void AttachRenderTexture(RenderTexture texture);
        static void DetachRenderTexture();

        static void SetProjection(SceneCamera camera);
        static void ResetProjection();

        static void Render();

        static BlVec2<f32> MeasureText(const std::string& text, Font& font, TextParams parameters);
        static BlRenderer2DInfo GetRenderingInfo();
    };

    // font rendering and utilities
    // void DrawText(const std::string& str, BlVec2 pos, Font& font, u32 size, BlColor color = White);
    // BlVec2 MeasureText(const std::string& str, Font& font, u32 size);

} // namespace Blackberry