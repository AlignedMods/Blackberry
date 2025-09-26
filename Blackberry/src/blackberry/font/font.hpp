#pragma once

#include "blackberry/image/image.hpp"

#include <filesystem>
#include <unordered_map>

namespace Blackberry {

    struct BlGlyphInfo {
        u32 Value; // unicode character value
        BlRec Rect;
        i32 AdvanceX;
    };

    struct __BlFont {
        BlTexture Atlas = nullptr;
        u32 GlyphCount = 0;
        BlGlyphInfo* Glyphs = nullptr;
    };

    class Font {
    public:
        Font();
        Font(const std::filesystem::path& path, u32 size = 24);
        ~Font();

        void LoadFontFromFile(const std::filesystem::path& path, u32 size = 24);
        BlGlyphInfo GetGlyphInfo(u32 value, u32 size = 24);

        void SetSize(u32 size);

        BlTexture GetTexture(u32 size = 24);

    private:
        std::filesystem::path m_FontPath;
        std::unordered_map<u32, __BlFont> m_Fonts; // map of fonts (with the key being the font size)
    };

} // namespace Blackberry