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
        InitFreeType();
    }

    Font::Font(const std::filesystem::path& path, u32 size) {
        InitFreeType();
        
        LoadFontFromFile(path, size);
    }

    Font::~Font() {
        FT_Done_Face(m_Face);
    }

    void Font::LoadFontFromFile(const std::filesystem::path& path, u32 size) {
        m_FontPath = path;

        CreateFont(size);
    }

    BlGlyphInfo Font::GetGlyphInfo(u32 value, u32 size) {
        SetSize(size);

        __BlFont font = m_Fonts.at(size);

        for (u32 i = 32; i < font.GlyphCount; i++) {
            if (font.Glyphs[i].Value == value) {
                return font.Glyphs[i];
            }
        }

        return BlGlyphInfo();
    }

    void Font::SetSize(u32 size) {
        if (m_Fonts.contains(size)) { return; }

        FT_Set_Pixel_Sizes(m_Face, 0, size);

        // to insert the size
        m_Fonts[size].GlyphCount = m_Face->num_glyphs;

        __BlFont& font = m_Fonts.at(size);

        // set up glyph array
        font.Glyphs = new BlGlyphInfo[255];

        u32 currentX = 0;
        u32 currentY = 0;
        u32 rowHeight = 0;
        u32 maxRowHeight = 0;

        u8* buffer = new u8[512 * 512];
        memset(buffer, 0, 512 * 512);

        for (u8 c = 32; c < 255; c++) {
            if (FT_Load_Char(m_Face, c, FT_LOAD_RENDER)) {
                Log(Log_Debug, "Failed to load codepoint %c", c);
                continue;
            }

            FT_GlyphSlot g = m_Face->glyph;
            FT_Bitmap bmp = g->bitmap;

            if (currentX + bmp.width >= 512) {
                currentX = 0;
                currentY += rowHeight;
                rowHeight = 0;
            }

            // put pixels in array
            for (u32 y = 0; y < bmp.rows; y++) {
                for (u32 x = 0; x < bmp.width; x++) {
                    u32 dstY = y + currentY;
                    u32 dstX = x + currentX;

                    buffer[dstY * 512 + dstX] = 
                        bmp.buffer[y * bmp.pitch + x];
                }
            }

            font.Glyphs[c].Rect = BlRec(currentX, currentY, bmp.width, bmp.rows);
            font.Glyphs[c].Left = g->bitmap_left;
            font.Glyphs[c].Top = g->bitmap_top;
            font.Glyphs[c].AdvanceX = g->advance.x >> 6;
            font.Glyphs[c].Value = c;

            currentX += bmp.width;
            if (bmp.rows > rowHeight) {
                rowHeight = bmp.rows;
                maxRowHeight = rowHeight;
            }
        }

        font.RowHeight = maxRowHeight;

        // create a rgba image out of grayscale
        u8* rgba = new u8[512 * 512 * 4];

        for (u32 i = 0; i < 512 * 512; i++) {
            rgba[i * 4 + 0] = 255;
            rgba[i * 4 + 1] = 255;
            rgba[i * 4 + 2] = 255;
            rgba[i * 4 + 3] = buffer[i];
        }

        // FT_Done_Face(face);

        delete[] buffer;
        buffer = rgba;

        font.Image = new Image(buffer, 512, 512, ImageFormat::RGBA8);
        font.Atlas = LoadTextureFromImage(*font.Image);

        // Image im(m_Buffer, 512, 512, ImageFormat::RGBA8);
        // 
        // font.Atlas = LoadTextureFromImage(im);
    }

    BlTexture Font::GetTexture(u32 size) {
        SetSize(size);

        return m_Fonts.at(size).Atlas;
    }

   void Font::GetImage(Image& image, u32 size) {
        SetSize(size);

        image = *m_Fonts.at(size).Image;
    }

   i32 Font::GetRowHeight(u32 size) {
       SetSize(size);

       return m_Fonts.at(size).RowHeight;
   }

   void Font::InitFreeType() {
        if (!s_IsInitialized) {
            if (FT_Init_FreeType(&s_FT)) {
                Log(Log_Critical, "Failed to initialze freetype!");
                exit(1);
            }

            s_IsInitialized = true;
        }
   }

   void Font::CreateFont(u32 size) {
        if (FT_New_Face(s_FT, m_FontPath.string().c_str(), 0, &m_Face)) {
            Log(Log_Error, "Failed to create font %s!", m_FontPath.string().c_str());
        }
   }

} // namespace Blackberry