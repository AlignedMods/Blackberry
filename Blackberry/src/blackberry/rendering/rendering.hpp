#include "blackberry/application/application.hpp"
#include "blackberry/types.hpp"

namespace Blackberry {

    // shapes
    void DrawRectangle(BlVec2 pos, BlVec2 dimensions, const BlColor& color);
    void DrawTriangle(BlVec2 bl, BlVec2 t, BlVec2 br, const BlColor& color);

    // loading and drawing textures
    BlTexture LoadTextureFromImage(const Image& image);
    void DrawTexture(BlVec2 pos, BlTexture texture);
    void DrawTextureEx(BlVec2 pos, BlVec2 dimensions, BlTexture texture);

} // namespace Blackberry