#include "blackberry/font/font.hpp"
#include "blackberry/core/types.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/rendering/rendering.hpp"
#include "blackberry/core/util.hpp"

#include "msdfgen.h"
#include "msdf-atlas-gen/msdf-atlas-gen.h"
#include "ext/import-font.h"

#include <fstream>
#include <iostream>

namespace Blackberry {

    static msdfgen::FreetypeHandle* s_FreetypeHandle = nullptr;

    Font Font::Create(const std::filesystem::path& path) {
        Font font;

        InitFreeType();

        std::string contents = ReadEntireFile(path);
        font.m_FontFileData.assign(contents.begin(), contents.end());
        font.GenerateAtlas();

        return font;
    }

    void Font::InitFreeType() {
        if (s_FreetypeHandle) { return; }
        s_FreetypeHandle = msdfgen::initializeFreetype();
        
        BL_ASSERT(s_FreetypeHandle, "Freetype not initialized properly!");
    }

    GlyphInfo Font::GetGlyphInfo(u8 codepoint, u32 size) {
        return Glyphs[codepoint];
    }

    void Font::GenerateAtlas() {
        using namespace msdf_atlas;

        msdfgen::FontHandle* font = msdfgen::loadFontData(s_FreetypeHandle, m_FontFileData.data(), m_FontFileData.size());
        std::vector<GlyphGeometry> glyphs;

        FontGeometry fontGeometry(&glyphs);

        fontGeometry.loadCharset(font, 1.0, Charset::ASCII);
        constexpr f64 MAX_CORNER_ANGLE = 3.0;
        for (auto& glyph : glyphs) {
            glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, MAX_CORNER_ANGLE, 0);
        }

        TightAtlasPacker packer;
        packer.setDimensionsConstraint(DimensionsConstraint::SQUARE);
        packer.setMinimumScale(24.0);
        packer.setPixelRange(2.0);
        packer.setMiterLimit(1.0);

        packer.pack(glyphs.data(), glyphs.size()); // pack glyphs (so we can know size)

        int width = 0, height = 0;
        packer.getDimensions(width, height);

        ImmediateAtlasGenerator<f32, 3, msdfGenerator, BitmapAtlasStorage<byte, 3>> generator(width, height);
        GeneratorAttributes attribs;
        generator.setAttributes(attribs);
        generator.setThreadCount(4);

        generator.generate(glyphs.data(), glyphs.size()); // create atlas
        msdfgen::BitmapConstRef<u8, 3> bitmap(generator.atlasStorage());

        // funny cast
        TextureAtlas.Create(const_cast<byte*>(bitmap.pixels), bitmap.width, bitmap.height, ImageFormat::RGB8);

        for (auto codepoint : Charset::ASCII) {
            auto glyph = fontGeometry.getGlyph(codepoint);
            GlyphInfo info;

            int x, y, w, h;
            glyph->getBoxRect(x, y, w, h);
            
            info.Rect = BlRec(x, y, w, h);
            info.AdvanceX = static_cast<f32>(glyph->getAdvance() * 64.0);

            Glyphs[codepoint] = info;
        }

        msdfgen::destroyFont(font);
    }

} // namespace Blackberry