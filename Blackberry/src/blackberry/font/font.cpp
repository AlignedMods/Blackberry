#include "blackberry/font/font.hpp"
#include "blackberry/core/types.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/rendering/rendering.hpp"
#include "blackberry/core/util.hpp"

#include "msdf-atlas-gen/msdf-atlas-gen.h"
#include "msdfgen/msdfgen.h"
#include "stb_image_write.h"

#include <fstream>
#include <iostream>

namespace Blackberry {

    static msdfgen::FreetypeHandle* s_FreetypeHandle = nullptr;

    Font::Font() {
        InitFreeType();
    }

    Font::Font(const std::filesystem::path& path)
        : m_Path(path) 
    {
        InitFreeType();
        CreateFont();
    }

    Font::~Font()
    {
    }

    void Font::InitFreeType() {
        if (s_FreetypeHandle) { return; }
        s_FreetypeHandle = msdfgen::initializeFreetype();
        
        BL_ASSERT(s_FreetypeHandle, "Freetype not initialized properly!");
    }

    void Font::CreateFont() {
        m_Handle = msdfgen::loadFont(s_FreetypeHandle, m_Path.string().c_str());

        BL_ASSERT(m_Handle, "Font was not created properly!");

        msdf_atlas::FontGeometry fontGeometry(&m_Glyphs);
        fontGeometry.loadCharset(m_Handle, 1.0, msdf_atlas::Charset::ASCII);

        for (auto& glyph : m_Glyphs) {
            glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, 3.0, 0);

            u8 codepoint = glyph.getCodepoint();
            i32 x, y, w, h;
            glyph.getBoxRect(x, y, w, h);

            BlGlyphInfo info;
            info.Value = codepoint;
            info.Rect = BlRec(x, y, w, h);
            info.AdvanceX = static_cast<f32>(glyph.getAdvance());

            m_UserGlyphs[codepoint] = info;
        }

        msdf_atlas::TightAtlasPacker packer;
        packer.setDimensionsConstraint(msdf_atlas::TightAtlasPacker::DimensionsConstraint::SQUARE);
        packer.setMinimumScale(32.0f);
        packer.setPixelRange(2.0);
        packer.setMiterLimit(1.0);
        packer.pack(m_Glyphs.data(), m_Glyphs.size());

        i32 width = 0, height = 0;
        packer.getDimensions(width, height);

        msdf_atlas::GeneratorAttributes atrribs;
        msdf_atlas::ImmediateAtlasGenerator<f32, 3, msdf_atlas::msdfGenerator, msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>> generator(width, height);
        generator.setAttributes(atrribs);
        generator.setThreadCount(4);
        generator.generate(m_Glyphs.data(), m_Glyphs.size());

        msdfgen::BitmapConstRef<msdfgen::byte, 3> atlas = generator.atlasStorage();
        Image image(atlas.pixels, width, height, ImageFormat::RGB8);
        image.WriteOut("test.png");
        
        m_Texture.Create(image);

        msdfgen::destroyFont(m_Handle);
    }

    BlTexture Font::GetTexture() {
        return m_Texture;
    }

    BlGlyphInfo Font::GetGlyphInfo(u8 codepoint) {
        if (!m_UserGlyphs.contains(codepoint)) {
            return BlGlyphInfo{};
        }

        return m_UserGlyphs.at(codepoint);
    }

} // namespace Blackberry