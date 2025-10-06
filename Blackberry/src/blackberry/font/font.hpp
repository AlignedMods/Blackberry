#pragma once

#include "blackberry/image/image.hpp"

#include <filesystem>
#include <unordered_map>

struct FT_FaceRec_;

namespace Blackberry {

    struct BlGlyphInfo {
        u8 Value = 0; // unicode character value
        BlRec Rect;
        i32 AdvanceX = 0;
        i32 Top = 0;
        i32 Left = 0;
        i32 BaselineOffset = 0;
    };

    struct __BlFont {
        BlTexture Atlas = nullptr;
        Image* Image = nullptr;
        u32 GlyphCount = 0;
        BlGlyphInfo* Glyphs = nullptr;
        i32 RowHeight = 0;
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
        void GetImage(Image& image, u32 size = 24);
        i32 GetRowHeight(u32 size = 24);

    private:
        void InitFreeType();
        void CreateFont(u32 size);

    private:
        std::filesystem::path m_FontPath;
        std::unordered_map<u32, __BlFont> m_Fonts; // map of fonts (with the key being the font size)
        
        FT_FaceRec_* m_Face;
    };

} // namespace Blackberry