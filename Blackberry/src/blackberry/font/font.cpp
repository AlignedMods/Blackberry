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
        packer.setMinimumScale(32.0);
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
        TextureAtlas.Create(const_cast<byte*>(bitmap.pixels), bitmap.width, bitmap.height, ImageFormat::RGB8, BlTextureFiltering::Linear);

        for (auto codepoint : Charset::ASCII) {
            auto glyph = fontGeometry.getGlyph(codepoint);
            GlyphInfo info;

            f64 x, y, w, h;
            glyph->getQuadAtlasBounds(x, y, w, h);
            f64 l, b, r, t;
            glyph->getQuadPlaneBounds(l, b, r, t);
            glyph->getAdvance();
            
            info.AtlasRect = BlRec(static_cast<f32>(x), static_cast<f32>(y), static_cast<f32>(w), static_cast<f32>(h));
            info.PlaneRect = BlRec(static_cast<f32>(l), static_cast<f32>(t), static_cast<f32>(r), static_cast<f32>(b));
            info.AdvanceX = static_cast<f32>(glyph->getAdvance());

            Glyphs[codepoint] = info;
        }

        GeometryScale = fontGeometry.getGeometryScale();
        Ascender = fontGeometry.getMetrics().ascenderY;
        Descender = fontGeometry.getMetrics().descenderY;
        LineHeight = fontGeometry.getMetrics().lineHeight;

        msdfgen::destroyFont(font);
    }

} // namespace Blackberry