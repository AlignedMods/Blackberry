#include "blackberry/application/application.hpp"
#include "blackberry/core/types.hpp"
#include "blackberry/font/font.hpp"

#include <string>

namespace Blackberry {

    inline BlColor White = BlColor(255, 255, 255, 255);
    inline BlColor Black = BlColor(0, 0, 0, 255);
    inline BlColor Red = BlColor(225, 25, 25, 255);
    inline BlColor Blue = BlColor(25, 25, 225, 255);

    void Clear();

    // shapes
    void DrawRectangle(BlVec2 pos, BlVec2 dimensions, const BlColor& color);
    void DrawTriangle(BlVec2 bl, BlVec2 t, BlVec2 br, const BlColor& color);

    // loading and drawing textures
    void DrawTexture(BlVec2 pos, BlTexture texture, BlColor color = White);
    void DrawTextureEx(BlVec2 pos, BlVec2 dimensions, BlTexture texture, BlColor color = White);
    void DrawTextureArea(BlVec2 pos, BlVec2 dimensions, BlRec area, BlTexture texture, BlColor color = White); // draw only a part of the texture

    // render textures
    void DrawRenderTexture(BlVec2 pos, BlVec2 dimensions, BlRenderTexture texture);
    void AttachRenderTexture(BlRenderTexture texture);
    void DetachRenderTexture();

    // font rendering and utilities
    void DrawText(const std::string& str, BlVec2 pos, Font& font, u32 size, BlColor color = White);
    BlVec2 MeasureText(const std::string& str, Font& font, u32 size);

    namespace FileDialogs {
        
        std::string OpenFile(const char* filter);

    } // namespace FileDialogs

    namespace Directories {

        std::string GetAppDataDirectory();

    } // namespace Directories

} // namespace Blackberry