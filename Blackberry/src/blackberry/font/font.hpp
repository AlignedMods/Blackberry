#pragma once

#include "blackberry/image/image.hpp"
#include "blackberry/rendering/texture.hpp"

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace Blackberry {

    struct GlyphInfo {
        u8 Value = 0; // unicode character value
        BlRec AtlasRect; // rectangle on the font atlas
        BlRec PlaneRect; // rectangle relative to baseline
        f32 AdvanceX = 0.0f;
        f32 BaselineOffset = 0.0f;
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
        f32 LineHeight = 0.0f;
        f32 Ascender = 0.0f;
        f32 Descender = 0.0f;
        f32 GeometryScale = 0.0f;

    private:
        std::vector<u8> m_FontFileData;
    };

} // namespace Blackberry