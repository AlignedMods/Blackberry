#pragma once

#include "blackberry/image/image.hpp"
#include "blackberry/rendering/texture.hpp"

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace Blackberry {

    struct GlyphInfo {
        u8 Value = 0; // unicode character value
        BlRec Rect;
        f32 AdvanceX = 0.0f;
        i32 Top = 0;
        i32 Left = 0;
        i32 BaselineOffset = 0;
    };
    

    class Font {
    public:
        static Font Create(const std::filesystem::path& path);
        static void InitFreeType();

        GlyphInfo GetGlyphInfo(u8 codepoint, u32 size);
        void GenerateAtlas();

    public:
        BlTexture TextureAtlas;
        Image ImageAtlas;
        u32 GlyphCount = 0;
        std::unordered_map<u8, GlyphInfo> Glyphs;
        i32 RowHeight = 0;
        i32 Ascender = 0;

    private:
        std::vector<u8> m_FontFileData;
    };

} // namespace Blackberry