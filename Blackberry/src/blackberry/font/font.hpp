#pragma once

#include "blackberry/image/image.hpp"
#include "blackberry/rendering/texture.hpp"

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace msdfgen {
    class FontHandle;
}

namespace msdf_atlas {
    class GlyphGeometry;
}

struct BlGlyphInfo {
    u8 Value = 0; // unicode character value
    BlRec Rect;
    f32 AdvanceX = 0.0f;
    i32 Top = 0;
    i32 Left = 0;
    i32 BaselineOffset = 0;
};

namespace Blackberry {

    class Font {
    public:
        Font();
        Font(const std::filesystem::path& path);
        ~Font();

        void CreateFont();
        BlTexture GetTexture();

        BlGlyphInfo GetGlyphInfo(u8 codepoint);

    private:
        void InitFreeType();

    private:
        msdfgen::FontHandle* m_Handle;
        std::filesystem::path m_Path = "Assets/arial/arial.ttf";

        // std::vector<msdf_atlas::GlyphGeometry> m_Glyphs; // internal glyphs
        // std::unordered_map<u8, BlGlyphInfo> m_UserGlyphs; // user land glyphs

        BlTexture m_Texture;
    };

} // namespace Blackberry