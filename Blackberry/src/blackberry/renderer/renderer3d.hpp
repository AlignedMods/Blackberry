#include "blackberry/application/application.hpp"
#include "blackberry/core/types.hpp"
#include "blackberry/font/font.hpp"
#include "blackberry/scene/camera.hpp"
#include "blackberry/model/mesh.hpp"

#include <string>

#undef DrawText // windows i will genuenly kill you

namespace Blackberry {

    namespace Colors {

        inline BlColor White = BlColor(255, 255, 255, 255);
        inline BlColor Black = BlColor(0, 0, 0, 255);
        inline BlColor Red = BlColor(225, 25, 25, 255);
        inline BlColor Blue = BlColor(25, 25, 225, 255);

    } // namespace Colors

    struct TextParams {
        f32 Kerning = 0.0f;
        f32 LineSpacing = 0.0f;
    };

    struct Renderer3DStats {
        u32 DrawCalls = 0;
        u32 Vertices = 0;
        u32 Indicies = 0;
        u32 ActiveTextures = 0;
        u32 ReservedTextures = 0;
    };

    // 3D renderer used to render scenes
    class Renderer3D {
    public:
        static void Init();
        static void Shutdown();

        static void Clear(BlColor = Colors::White);

        static void NewFrame();

        static void DrawMesh(const glm::mat4& transform, Mesh& mesh, BlColor color = Colors::White);

        static void DrawText(BlVec3<f32> pos, f32 fontSize, const std::string& text, Font& font, TextParams params = TextParams{}, BlColor color = Colors::White);
        static void DrawText(const glm::mat4& transform, const std::string& text, Font& font, TextParams params = TextParams{}, BlColor color = Colors::White);

        static void BindRenderTexture(RenderTexture texture);
        static void UnBindRenderTexture();

        static void SetProjection(SceneCamera camera);
        static void ResetProjection();

        static void Render();

        static BlVec2<f32> MeasureText(const std::string& text, Font& font, TextParams parameters);
        static Renderer3DStats GetRendererStats();
    };

    // font rendering and utilities
    // void DrawText(const std::string& str, BlVec2 pos, Font& font, u32 size, BlColor color = White);
    // BlVec2 MeasureText(const std::string& str, Font& font, u32 size);

} // namespace Blackberry