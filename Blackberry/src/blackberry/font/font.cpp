#include "font.hpp"
#include "blackberry/types.hpp"
#include "blackberry/log.hpp"
#include "blackberry/rendering/rendering.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H
#include "stb_image_write.h"

#include <fstream>
#include <iostream>

namespace Blackberry {

    static FT_Library s_FT;
    static bool s_IsInitialized = false;

    Font::Font() {
        if (!s_IsInitialized) {
            if (FT_Init_FreeType(&s_FT)) {
                Log(Log_Critical, "Failed to initialze freetype!");
                exit(1);
            }

            s_IsInitialized = true;
        }
    }

    Font::Font(const std::filesystem::path& path, u32 size) {
        if (!s_IsInitialized) {
            if (FT_Init_FreeType(&s_FT)) {
                Log(Log_Critical, "Failed to initialze freetype!");
                exit(1);
            }

            s_IsInitialized = true;
        }
        
        LoadFontFromFile(path, size);
    }

    Font::~Font() {
        
    }

    void Font::LoadFontFromFile(const std::filesystem::path& path, u32 size) {
        m_FontPath = path;

        SetSize(size);
    }

    BlGlyphInfo Font::GetGlyphInfo(u32 value, u32 size) {
        if (!m_Fonts.contains(size)) {
            SetSize(size);
        }

        __BlFont font = m_Fonts.at(size);

        for (int i = 32; i < font.GlyphCount; i++) {
            if (font.Glyphs[i].Value == value) {
                return font.Glyphs[i];
            }
        }

        return BlGlyphInfo();
    }

    void Font::SetSize(u32 size) {
        FT_Face face;
        if (FT_New_Face(s_FT, m_FontPath.string().c_str(), 0, &face)) {
            Log(Log_Error, "Failed to create font %s!", m_FontPath.string().c_str());
        }

        FT_Set_Pixel_Sizes(face, 0, size);

        // to insert the size
        m_Fonts[size].GlyphCount = face->num_glyphs;

        __BlFont& font = m_Fonts.at(size);

        // set up glyph array
        font.Glyphs = new BlGlyphInfo[font.GlyphCount + 32];

        u32 currentX = 0;
        u32 currentY = 0;
        u32 rowHeight = 0;

        u8* m_Buffer = new u8[512 * 512];

        // write the data into the image buffer and load glyphs
        for (u8 c = 32; c < 127; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                Log(Log_Debug, "Failed to load character %c", c);
                continue;
            }

            FT_GlyphSlot& g = face->glyph;
            FT_Bitmap& bm = g->bitmap;

            if (currentX + bm.width > 512) {
                currentX = 0;
                currentY += rowHeight + 1;
                rowHeight = 0;
            }

            font.Glyphs[c].Value = c;
            font.Glyphs[c].Rect = BlRec(currentX, currentY, bm.width, bm.rows);
            font.Glyphs[c].AdvanceX = g->advance.x >> 6;

            for (u32 y = 0; y < bm.rows; y++) {
                for (u32 x = 0; x < bm.width; x++) {
                    u32 dstX = x + currentX;
                    u32 dstY = y + currentY;

                    if (dstX < 512 && dstY < 512) {
                        m_Buffer[dstY * 512 + dstX] = bm.buffer[y * bm.pitch + x];
                    }
                }
            }

            currentX += bm.width;
            if (bm.rows > rowHeight) {
                rowHeight = bm.rows;
            }
        }

        u8* rgba = new u8[512 * 512 * 4];

        for (u32 i = 0; i < 512 * 512; i++) {
            rgba[i * 4 + 0] = 255;
            rgba[i * 4 + 1] = 255;
            rgba[i * 4 + 2] = 255;
            rgba[i * 4 + 3] = m_Buffer[i];
        }

        delete[] m_Buffer;
        m_Buffer = rgba;

        Image im(m_Buffer, 512, 512, ImageFormat::RGBA8);

        font.Atlas = LoadTextureFromImage(im);
    }

    BlTexture Font::GetTexture(u32 size) {
        if (!m_Fonts.contains(size)) {
            SetSize(size);
        }

        return m_Fonts.at(size).Atlas;
    }

} // namespace Blackberry