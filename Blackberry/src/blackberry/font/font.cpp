#include "font.hpp"
#include "blackberry/types.hpp"
#include "blackberry/log.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H
#include "stb_image_write.h"

#include <fstream>

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

    Font::Font(const std::filesystem::path& path) {
        if (!s_IsInitialized) {
            if (FT_Init_FreeType(&s_FT)) {
                Log(Log_Critical, "Failed to initialze freetype!");
                exit(1);
            }

            s_IsInitialized = true;
        }
        
        LoadFontFromFile(path);
    }

    Font::~Font() {
    
    }

    Image Font::LoadFontFromFile(const std::filesystem::path& path) {
        std::stringstream stream;
        std::string buffer;

        std::ifstream file(path);

        stream << file.rdbuf();
        buffer = stream.str();
        stream.flush();

        FT_Face face;
        if (FT_New_Face(s_FT, path.string().c_str(), 0, &face)) {
            Log(Log_Error, "Failed to create font %s!", path.string().c_str());
        }

        FT_Set_Pixel_Sizes(face, 0, 48);

        FT_GlyphSlot g = face->glyph;

        std::vector<u8> atlas(512 * 512, 0);

        i32 penX = 0;
        i32 penY = 0;
        i32 rowHeight = 0;

        for (unsigned char c = 32; c < 127; ++c) { // Printable ASCII range
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                Log(Log_Error, "Failed to load char: %c", c);
                continue;
            }

            FT_Bitmap& bmp = g->bitmap;

            // New row if needed
            if (penX + bmp.width >= 512) {
                penX = 0;
                penY += rowHeight + 1;
                rowHeight = 0;
            }

            // Copy glyph bitmap into atlas
            for (int y = 0; y < bmp.rows; ++y) {
                for (int x = 0; x < bmp.width; ++x) {
                    int dstX = penX + x;
                    int dstY = penY + y;
                    if (dstX < 512 && dstY < 512) {
                        atlas[dstY * 512 + dstX] = 
                            bmp.buffer[y * bmp.pitch + x];
                    }
                }
            }

            // Advance pen
            penX += bmp.width + 1;
            if (bmp.rows > rowHeight)
                rowHeight = bmp.rows;
        }

        if (!stbi_write_png("test.png", 512, 512, 1, atlas.data(), 512)) {
            Log(Log_Error, "Failed to write font atlas!");
        }

        // return Image(bitmap, width, height);
        return Image();
    }

} // namespace Blackberry