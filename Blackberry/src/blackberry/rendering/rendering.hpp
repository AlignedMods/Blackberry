#include "blackberry/application/application.hpp"
#include "blackberry/types.hpp"
#include "blackberry/font/font.hpp"

#include <string>

namespace Blackberry {

    inline BlColor White = BlColor(255, 255, 255, 255);
    inline BlColor Red = BlColor(225, 25, 25, 255);
    inline BlColor Blue = BlColor(25, 25, 225, 255);

    // shapes
    void DrawRectangle(BlVec2 pos, BlVec2 dimensions, const BlColor& color);
    void DrawTriangle(BlVec2 bl, BlVec2 t, BlVec2 br, const BlColor& color);

    // loading and drawing textures
    BlTexture LoadTextureFromImage(const Image& image);
    void DrawTexture(BlVec2 pos, BlTexture texture, BlColor color = White);
    void DrawTextureEx(BlVec2 pos, BlVec2 dimensions, BlTexture texture, BlColor color = White);
    void DrawTextureArea(BlVec2 pos, BlVec2 dimensions, BlRec area, BlTexture texture, BlColor color = White); // draw only a part of the texture

    // texture getters and such
    BlVec2 GetTextureSize(BlTexture texture);
    u32 GetTextureWidth(BlTexture texture);
    u32 GetTextureHeight(BlTexture texture);

    // font rendering and utilities
    void DrawText(const std::string& str, BlVec2 pos, Font& font, u32 size, BlColor color = White);
    BlVec2 MeasureText(const std::string& str, Font& font, u32 size);

} // namespace Blackberry