#include "blackberry/renderer/renderer3d.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/scene/camera.hpp"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct BlShapeVertex { // used for quads and triangles
    BlVec3<f32> Pos;
    BlVec4<f32> Color;
    BlVec2<f32> TexCoord;
    f32 TexIndex = 0.0f; // OpenGL is weird with passing integers to shaders (also 0 is a white texture)
};

struct BlCircleVertex {
    BlVec3<f32> Pos;
    BlVec4<f32> Color;
    BlVec2<f32> TexCoord;
};

struct BlFontVertex {
    BlVec3<f32> Pos;
    BlVec4<f32> Color;
    BlVec2<f32> TexCoord;
};

namespace Blackberry {

    static u8 s_WhiteTextureData[] = {0xff, 0xff, 0xff, 0xff}; // dead simple white texture (1x1 pixel)

    static const char* s_VertexShaderShapeSource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec3 a_Pos; // BlShapeVertex.Pos (also we can have comments like this because preproccesor ignores them!)
        layout (location = 1) in vec4 a_Color; // BlShapeVertex.Color
        layout (location = 2) in vec2 a_TexCoord; // BlShapeVertex.TexCoord
        layout (location = 3) in float a_TexIndex; // BlShapeVertex.TexIndex

        uniform mat4 u_Projection;

        layout (location = 0) out vec4 o_Color;
        layout (location = 1) out vec2 o_TexCoord;
        layout (location = 2) out flat float o_TexIndex;

        void main() {
            gl_Position = u_Projection * vec4(a_Pos, 1.0f);

            o_Color = a_Color;
            o_TexCoord = a_TexCoord;
            o_TexIndex = a_TexIndex;
        }
    );

    static const char* s_FragmentShaderShapeSource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec4 a_Color;
        layout (location = 1) in vec2 a_TexCoord;
        layout (location = 2) in flat float a_TexIndex;

        uniform sampler2D u_Textures[16];

        out vec4 o_FragColor;

        void main() {
            vec4 texColor = a_Color;

            switch(int(a_TexIndex))
	        {
	        	case  0: texColor *= texture(u_Textures[ 0], a_TexCoord); break;
	        	case  1: texColor *= texture(u_Textures[ 1], a_TexCoord); break;
	        	case  2: texColor *= texture(u_Textures[ 2], a_TexCoord); break;
	        	case  3: texColor *= texture(u_Textures[ 3], a_TexCoord); break;
	        	case  4: texColor *= texture(u_Textures[ 4], a_TexCoord); break;
	        	case  5: texColor *= texture(u_Textures[ 5], a_TexCoord); break;
	        	case  6: texColor *= texture(u_Textures[ 6], a_TexCoord); break;
	        	case  7: texColor *= texture(u_Textures[ 7], a_TexCoord); break;
	        	case  8: texColor *= texture(u_Textures[ 8], a_TexCoord); break;
	        	case  9: texColor *= texture(u_Textures[ 9], a_TexCoord); break;
	        	case 10: texColor *= texture(u_Textures[10], a_TexCoord); break;
	        	case 11: texColor *= texture(u_Textures[11], a_TexCoord); break;
	        	case 12: texColor *= texture(u_Textures[12], a_TexCoord); break;
	        	case 13: texColor *= texture(u_Textures[13], a_TexCoord); break;
	        	case 14: texColor *= texture(u_Textures[14], a_TexCoord); break;
	        	case 15: texColor *= texture(u_Textures[15], a_TexCoord); break;
	        }

            if (texColor.a == 0.0) { discard; }

            o_FragColor = texColor;
        }
    );

    static const char* s_VertexShaderCircleSource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec3 a_Pos;
        layout (location = 1) in vec4 a_Color;
        layout (location = 2) in vec2 a_TexCoord;

        uniform mat4 u_Projection;

        layout (location = 0) out vec4 o_Color;
        layout (location = 1) out vec2 o_TexCoord;

        void main() {
            gl_Position = u_Projection * vec4(a_Pos, 1.0);

            o_Color = a_Color;
            o_TexCoord = a_TexCoord;
        }
    );

    static const char* s_FragmentShaderCircleSource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec4 a_Color;
        layout (location = 1) in vec2 a_TexCoord;

        out vec4 o_FragColor;

        void main() {
            vec2 uv = a_TexCoord * 2.0 - 1.0;
            
            // Calculate distance (acts as an alpha channel)
            float distance = 1.0 - length(uv);
            distance = step(0.0, distance);

            // Set output color
            o_FragColor = a_Color * distance;
        }
    );

    static const char* s_VertexShaderFontSource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec3 a_Pos; // BlShapeVertex.Pos (also we can have comments like this because preproccesor ignores them!)
        layout (location = 1) in vec4 a_Color; // BlShapeVertex.Color
        layout (location = 2) in vec2 a_TexCoord; // BlShapeVertex.TexCoord

        uniform mat4 u_Projection;

        layout (location = 0) out vec4 o_Color;
        layout (location = 1) out vec2 o_TexCoord;

        void main() {
            gl_Position = u_Projection * vec4(a_Pos, 1.0f);

            o_Color = a_Color;
            o_TexCoord = a_TexCoord;
        }
    );

    static const char* s_FragmentShaderFontSource = BL_STR(
        \x23version 460 core\n

        layout (location = 0) in vec4 a_Color;
        layout (location = 1) in vec2 a_TexCoord;

        uniform sampler2D u_FontAtlas;

        out vec4 o_FragColor;

        float screenPxRange() {
            const float pxRange = 2.0;
            vec2 unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlas, 0));
            vec2 screenTexSize = vec2(1.0) / fwidth(a_TexCoord);
            return max(0.5*dot(unitRange, screenTexSize), 1.0);
        }

        float median(float r, float g, float b) {
            return max(min(r, g), min(max(r, g), b));
        }

        void main() {
            vec4 texelColor = texture(u_FontAtlas, a_TexCoord) * a_Color;

            // msdf thing
            vec3 msd = texture(u_FontAtlas, a_TexCoord).rgb;
            float sd = median(msd.r, msd.g, msd.b);
            float screenPxDistance = screenPxRange() * (sd - 0.5);
            float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
            if (opacity == 0.0) { discard; }

            vec4 bgColor = vec4(0.0);
            o_FragColor = mix(bgColor, a_Color, opacity);
        }
    );

    struct _Renderer3DState {
        // shader
        BlShader ShapeShader;
        BlShader CircleShader;
        BlShader FontShader;

        Texture2D WhiteTexture;

        // Shape buffer data
        u32 ShapeIndexCount = 0;
        u32 ShapeVertexCount = 0;
        std::vector<BlShapeVertex> ShapeVertices;
        std::vector<u32> ShapeIndices;
        BlDrawBuffer ShapeDrawBuffer;

        // Circle buffer data
        u32 CircleIndexCount = 0;
        u32 CircleVertexCount = 0;
        std::vector<BlCircleVertex> CircleVertices;
        std::vector<u32> CircleIndices;
        BlDrawBuffer CircleDrawBuffer;

        // Font buffer data
        u32 FontIndexCount = 0;
        u32 FontVertexCount = 0;
        std::vector<BlFontVertex> FontVertices;
        std::vector<u32> FontIndices;
        BlDrawBuffer FontDrawBuffer;
        Texture2D CurrentFontAtlas;

        // for textures
        u32 CurrentTexIndex = 1; // NOTE: 0 is reserved for a blank white texture!!!
        std::array<Texture2D, 16> CurrentAttachedTextures;

        SceneCamera Camera;
        SceneCamera DefaultCamera; // by default the camera is initialized to a basic 1x scale pixel-by-pixel orthographic projection (outdated now!!)

        const std::array<glm::vec4, 4> QuadVertexPositions = {{
            { -0.5f,  0.5f, 0.0f, 1.0f },
            {  0.5f, -0.5f, 0.0f, 1.0f },
            {  0.5f,  0.5f, 0.0f, 1.0f },
            { -0.5f, -0.5f, 0.0f, 1.0f }
        }};
        const std::array<u32, 6> QuadIndices = {{ 0, 1, 2, 0, 3, 1 }};

        const std::array<glm::vec4, 3> TriangleVertexPositions = {{
            { -0.5f,  0.5f, 0.0f, 1.0f },
            {  0.0f, -0.5f, 0.0f, 1.0f },
            {  0.5f,  0.5f, 0.0f, 1.0f }
        }};
        const std::array<u32, 3> TriangleIndices = {{ 0, 1, 2 }};

        const std::array<glm::vec4, 8> CubeVertexPositions = {{
            { -0.5f, -0.5f, -0.5f, 1.0f },
            {  0.5f, -0.5f, -0.5f, 1.0f },
            {  0.5f,  0.5f, -0.5f, 1.0f },
            { -0.5f,  0.5f, -0.5f, 1.0f },
            { -0.5f, -0.5f,  0.5f, 1.0f },
            {  0.5f, -0.5f,  0.5f, 1.0f },
            {  0.5f,  0.5f,  0.5f, 1.0f },
            { -0.5f,  0.5f,  0.5f, 1.0f }
        }};
        const std::array<u32, 36> CubeIndices = {{
            0, 1, 2, 2, 3, 0,
            1, 5, 6, 6, 2, 1,
            5, 4, 7, 7, 6, 5,
            4, 0, 3, 3, 7, 4,
            3, 2, 6, 6, 7, 3,
            4, 5, 1, 1, 0, 4
        }};

        Renderer3DStats Stats;
    };

    static _Renderer3DState Renderer3DState;

    static f32 GetTexIndex(Texture2D texture) {
        f32 texIndex = 0.0f;

        if (Renderer3DState.CurrentTexIndex >= 16) {
            Renderer3D::Render();
        }

        bool texAlreadyExists = false;

        for (u32 i = 0; i < Renderer3DState.CurrentTexIndex; i++) {
            if (texture.ID == Renderer3DState.CurrentAttachedTextures[i].ID) {
                texIndex = static_cast<f32>(i);
                texAlreadyExists = true;

                break;
            }
        }

        if (!texAlreadyExists) {
            texIndex = static_cast<f32>(Renderer3DState.CurrentTexIndex);

            Renderer3DState.CurrentAttachedTextures[Renderer3DState.CurrentTexIndex] = texture;
            Renderer3DState.CurrentTexIndex++;
            texAlreadyExists = true;
        }

        return texIndex;
    }

    static BlVec4<f32> NormalizeColor(BlColor color) {
        return BlVec4<f32>(
            static_cast<f32>(color.r) / 255.0f,
            static_cast<f32>(color.g) / 255.0f,
            static_cast<f32>(color.b) / 255.0f,
            static_cast<f32>(color.a) / 255.0f
        );
    }

    void Renderer3D::Init() {
        Renderer3DState.ShapeShader.Create(s_VertexShaderShapeSource, s_FragmentShaderShapeSource);
        Renderer3DState.CircleShader.Create(s_VertexShaderCircleSource, s_FragmentShaderCircleSource);
        Renderer3DState.FontShader.Create(s_VertexShaderFontSource, s_FragmentShaderFontSource);
        Renderer3DState.WhiteTexture = Texture2D::Create(s_WhiteTextureData, 1, 1, ImageFormat::RGBA8);

        Renderer3DState.CurrentAttachedTextures[0] = Renderer3DState.WhiteTexture; // 0 is reserved for white

        Renderer3DState.DefaultCamera.Transform = { BlVec3(0.0f), BlVec3(0.0f), BlVec3(1920, 1280, 0)};
        Renderer3DState.Camera = Renderer3DState.DefaultCamera;
    }

    void Renderer3D::Shutdown() {
        Renderer3DState.ShapeShader.Delete();
        Renderer3DState.CircleShader.Delete();
    }

    void Renderer3D::Clear(BlColor color) {
        auto& renderer = BL_APP.GetRenderer();

        renderer.Clear(color);
    }

    void Renderer3D::NewFrame() {
        Renderer3DState.Stats.DrawCalls = 0;
        Renderer3DState.Stats.Vertices = 0;
        Renderer3DState.Stats.Indicies = 0;
        Renderer3DState.Stats.ActiveTextures = 0;
    }

    void Renderer3D::DrawRectangle(BlVec3<f32> pos, BlVec2<f32> dimensions, BlColor color) {
        DrawRectangle(pos, dimensions, 0.0f, color);
    }

    void Renderer3D::DrawRectangle(BlVec3<f32> position, BlVec2<f32> dimensions, f32 rotation, BlColor color) {
        glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(dimensions.x, dimensions.y, 1.0f));

        glm::mat4 transform = pos * rot * scale;
        DrawTexturedQuad(transform, BlRec(0, 0, 1, 1), Renderer3DState.WhiteTexture, color);
    }

    void Renderer3D::DrawRectangle(const glm::mat4& transform, BlColor color) {
        DrawTexturedQuad(transform, BlRec(0, 0, 1, 1), Renderer3DState.WhiteTexture, color);
    }

    void Renderer3D::DrawTriangle(BlVec3<f32> position, BlVec2<f32> dimensions, f32 rotation, BlColor color) {
        glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(dimensions.x, dimensions.y, 1.0f));

        glm::mat4 transform = pos * rot * scale;

        DrawTexturedTriangle(transform, BlRec(0, 0, 1, 1), Renderer3DState.WhiteTexture, color);
    }

    void Renderer3D::DrawTriangle(const glm::mat4& transform, BlColor color) {
        DrawTexturedTriangle(transform, BlRec(0, 0, 1, 1), Renderer3DState.WhiteTexture, color);
    }

    void Renderer3D::DrawCircle(BlVec3<f32> pos, f32 radius, BlColor color) {
        // DrawElipse(pos, BlVec2<f32>(radius, radius), 0.0f, color);
    }

    void Renderer3D::DrawElipse(const glm::mat4& transform, BlColor color) {
        BlVec4 normalizedColor = NormalizeColor(color);

        const BlVec2<f32> texCoords[4] = { BlVec2<f32>(0, 1), BlVec2<f32>(1, 0), BlVec2<f32>(1, 1), BlVec2<f32>(0, 0) };

        // vertices
        for (u32 i = 0; i < Renderer3DState.QuadVertexPositions.size(); i++) {
            glm::vec4 pos = transform * Renderer3DState.QuadVertexPositions[i];
            BlCircleVertex vert = BlCircleVertex(BlVec3<f32>(pos.x, pos.y, pos.z), normalizedColor, texCoords[i]);
            Renderer3DState.CircleVertices.push_back(vert);
        }

        // indices
        for (u32 i = 0; i < Renderer3DState.QuadIndices.size(); i++) {
            const u32 vertexCount = Renderer3DState.CircleVertexCount;
            Renderer3DState.CircleIndices.push_back(Renderer3DState.QuadIndices[i] + vertexCount);
        }

        Renderer3DState.CircleVertexCount += 4;
        Renderer3DState.CircleIndexCount += 6;
    }

    void Renderer3D::DrawTexture(BlVec3<f32> pos, Texture2D texture, f32 rotation, BlColor color) {
        DrawTextureEx(pos, BlVec2<f32>(static_cast<f32>(texture.Size.x), static_cast<f32>(texture.Size.y)), texture, rotation, color);
    }

    void Renderer3D::DrawTextureEx(BlVec3<f32> pos, BlVec2<f32> dimensions, Texture2D texture, f32 rotation, BlColor color)  {
        DrawTextureArea(pos, dimensions, BlRec(0.0f, 0.0f, static_cast<f32>(texture.Size.x), static_cast<f32>(texture.Size.y)), texture, rotation, color);
    }

    void Renderer3D::DrawTextureArea(BlVec3<f32> position, BlVec2<f32> dimensions, BlRec area, Texture2D texture, f32 rotation, BlColor color) {
        glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(dimensions.x, dimensions.y, 1.0f));

        glm::mat4 transform = pos * rot * scale;

        DrawTexturedQuad(transform, area, texture, color);
    }

    void Renderer3D::DrawText(BlVec3<f32> position, f32 fontSize, const std::string& text, Font& font, TextParams params, BlColor color) {
        glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(fontSize / font.LineHeight, fontSize / font.LineHeight, 1.0f));

        glm::mat4 transform = pos * scale;

        DrawText(transform, text, font, params, color);
    }

    void Renderer3D::DrawText(const glm::mat4& transform, const std::string& text, Font& font, TextParams params, BlColor color) {
        Texture2D tex = font.TextureAtlas;
        f32 fsScale = 1.0f / (font.Ascender - font.Descender);
        f32 currentX = 0.0f;
        f32 currentY = 0.0f;

        BlVec2<f32> textSize = MeasureText(text, font, params);

        // glm::mat4 finalTextTransform = glm::scale(transform, glm::vec3(1.0f / textSize.x, 1.0f / textSize.y, 1.0f));
        glm::mat4 finalTextTransform = glm::scale(transform, glm::vec3(2.0f, 2.0f, 1.0f));

        BlColor currentColor = color;

        for (u32 c = 0; c < text.length(); c++) {
            if (text.at(c) == '\n') {
                currentY -= fsScale * font.LineHeight + params.LineSpacing;
                currentX = 0;
            } else {
                GlyphInfo glyph = font.GetGlyphInfo(text.at(c), 0);
                BlRec atlasBounds = glyph.AtlasRect;
                BlRec planeBounds = glyph.PlaneRect;

                if (tex.ID != Renderer3DState.CurrentFontAtlas.ID) {
                    Render();
                    Renderer3DState.CurrentFontAtlas = tex;
                }

                BlVec2<f32> texCoordMin(atlasBounds.x, atlasBounds.y);
                BlVec2<f32> texCoordMax(atlasBounds.w, atlasBounds.h);

                BlVec2<f32> quadMin = BlVec2<f32>(glyph.PlaneRect.x, glyph.PlaneRect.y);
                BlVec2<f32> quadMax = BlVec2<f32>(glyph.PlaneRect.w, glyph.PlaneRect.h);

                quadMin *= BlVec2<f32>(fsScale); quadMax *= BlVec2<f32>(fsScale);
                quadMin += BlVec2<f32>(currentX, currentY); quadMax += BlVec2<f32>(currentX, currentY);

                // make the text be perfectly centered (since the whole string must fit in a rectangle this is easy)
                // quadMin -= textSize * BlVec2<f32>(0.5f); quadMax -= textSize * BlVec2<f32>(0.5f);
                // quadMin -= BlVec2<f32>(0.5f); quadMax -= BlVec2<f32>(0.5f);
                quadMin.x -= 0.25f; quadMax.x -= 0.25f;
                quadMin.y -= 0.25f; quadMax.y -= 0.25f;

                f32 texelWidth = 1.0f / tex.Size.x;
                f32 texelHeight = 1.0f / tex.Size.y;
                texCoordMin *= BlVec2<f32>(texelWidth, texelHeight);
                texCoordMax *= BlVec2<f32>(texelWidth, texelHeight);

                BlVec4 normalizedColor = NormalizeColor(color);

                // vertices
                glm::vec4 pos = finalTextTransform * glm::vec4(quadMin.x, quadMin.y, 0.0f, 1.0f);
                BlFontVertex vert = BlFontVertex(BlVec3<f32>(pos.x, pos.y, pos.z), normalizedColor, BlVec2<f32>(texCoordMin.x, texCoordMax.y));
                Renderer3DState.FontVertices.push_back(vert);
                
                pos = finalTextTransform * glm::vec4(quadMax.x, quadMax.y, 0.0f, 1.0f);
                vert = BlFontVertex(BlVec3<f32>(pos.x, pos.y, pos.z), normalizedColor, BlVec2<f32>(texCoordMax.x, texCoordMin.y));
                Renderer3DState.FontVertices.push_back(vert);

                pos = finalTextTransform * glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
                vert = BlFontVertex(BlVec3<f32>(pos.x, pos.y, pos.z), normalizedColor, BlVec2<f32>(texCoordMax.x, texCoordMax.y));
                Renderer3DState.FontVertices.push_back(vert);

                pos = finalTextTransform * glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
                vert = BlFontVertex(BlVec3<f32>(pos.x, pos.y, pos.z), normalizedColor, BlVec2<f32>(texCoordMin.x, texCoordMin.y));
                Renderer3DState.FontVertices.push_back(vert);

                // indices
                for (u32 i = 0; i < Renderer3DState.QuadIndices.size(); i++) {
                    const u32 vertexCount = Renderer3DState.FontVertexCount;
                    Renderer3DState.FontIndices.push_back(Renderer3DState.QuadIndices[i] + vertexCount);
                }

                Renderer3DState.CurrentFontAtlas = tex;

                Renderer3DState.FontIndexCount += 6;
                Renderer3DState.FontVertexCount += 4;

                currentX += fsScale * (glyph.AdvanceX + params.Kerning);
            }
        }
    }

    void Renderer3D::DrawTexturedQuad(const glm::mat4& transform, BlRec area, Texture2D texture, BlColor color) {
        BlVec4 normalizedColor = NormalizeColor(color);
        f32 texIndex = GetTexIndex(texture);

        BlVec2<f32> texSize = BlVec2<f32>(static_cast<f32>(texture.Size.x), static_cast<f32>(texture.Size.y));

        const BlVec2<f32> texCoords[4] = { BlVec2<f32>(area.x / texSize.x, area.y / texSize.y), BlVec2<f32>((area.w + area.x) / texSize.x, (area.h + area.y) / texSize.y),
                                           BlVec2<f32>((area.w + area.x) / texSize.x, area.y / texSize.y), BlVec2<f32>(area.x / texSize.x, (area.h + area.y) / texSize.y)};

        // vertices
        for (u32 i = 0; i < Renderer3DState.QuadVertexPositions.size(); i++) {
            glm::vec4 pos = transform * Renderer3DState.QuadVertexPositions[i];
            BlShapeVertex vert = BlShapeVertex(BlVec3<f32>(pos.x, pos.y, pos.z), normalizedColor, texCoords[i], texIndex);
            Renderer3DState.ShapeVertices.push_back(vert);
        }

        // indices
        for (u32 i = 0; i < Renderer3DState.QuadIndices.size(); i++) {
            const u32 vertexCount = Renderer3DState.ShapeVertexCount;
            Renderer3DState.ShapeIndices.push_back(Renderer3DState.QuadIndices[i] + vertexCount);
        }

        Renderer3DState.ShapeIndexCount += 6;
        Renderer3DState.ShapeVertexCount += 4;
    }

    void Renderer3D::DrawTexturedTriangle(const glm::mat4& transform, BlRec area, Texture2D texture, BlColor color) {
        BlVec4 normalizedColor = NormalizeColor(color);
        f32 texIndex = GetTexIndex(texture);

        BlVec2<f32> texSize = BlVec2<f32>(static_cast<f32>(texture.Size.x), static_cast<f32>(texture.Size.y));
        const BlVec2<f32> texCoords[3] = { BlVec2<f32>(0.0f, 1.0f), BlVec2<f32>(0.5f, 0.0f), BlVec2<f32>(1.0f, 1.0f) };

        // vertices
        for (u32 i = 0; i < Renderer3DState.TriangleVertexPositions.size(); i++) {
            glm::vec4 pos = transform * Renderer3DState.TriangleVertexPositions[i];
            BlShapeVertex vert = BlShapeVertex(BlVec3<f32>(pos.x, pos.y, pos.z), normalizedColor, texCoords[i], texIndex);
            Renderer3DState.ShapeVertices.push_back(vert);
        }

        // indices
        for (u32 i = 0; i < Renderer3DState.TriangleIndices.size(); i++) {
            const u32 vertexCount = Renderer3DState.ShapeVertexCount;
            Renderer3DState.ShapeIndices.push_back(Renderer3DState.TriangleIndices[i] + vertexCount);
        }

        Renderer3DState.ShapeVertexCount += 3;
        Renderer3DState.ShapeIndexCount += 3;
    }

    void Renderer3D::DrawMesh(const glm::mat4& transform, Mesh& mesh, BlColor color) {
        BlVec4 normalizedColor = NormalizeColor(color);

        // vertices
        for (u32 i = 0; i < mesh.Positions.size(); i++) {
            glm::vec4 pos = transform * glm::vec4(mesh.Positions[i].x, mesh.Positions[i].y, mesh.Positions[i].z, 1.0f);
            BlShapeVertex vert = BlShapeVertex(BlVec3<f32>(pos.x, pos.y, pos.z), normalizedColor, BlVec2<f32>(0.0f), 0.0f);
            Renderer3DState.ShapeVertices.push_back(vert);
        }

        // indices
        for (u32 i = 0; i < mesh.Indices.size(); i++) {
            const u32 vertexCount = Renderer3DState.ShapeVertexCount;
            Renderer3DState.ShapeIndices.push_back(mesh.Indices[i] + vertexCount);
        }

        Renderer3DState.ShapeIndexCount += mesh.Indices.size();
        Renderer3DState.ShapeVertexCount += mesh.Positions.size();
    }

    void Renderer3D::DrawRenderTexture(BlVec3<f32> pos, BlVec2<f32> dimensions, RenderTexture texture) {
        DrawTextureArea(pos, dimensions, BlRec(0.0f, 0.0f, static_cast<f32>(texture.Size.x), static_cast<f32>(texture.Size.y) * -1.0f), texture.ColorAttachment);
    }

    void Renderer3D::BindRenderTexture(RenderTexture texture) {
        auto& renderer = BL_APP.GetRenderer();

        renderer.BindRenderTexture(texture);
    }

    void Renderer3D::UnBindRenderTexture() {
        auto& renderer = BL_APP.GetRenderer();

        renderer.UnBindRenderTexture();
    }

    void Renderer3D::SetProjection(SceneCamera camera) {
        Renderer3DState.Camera = camera;
    }

    void Renderer3D::ResetProjection() {
        Renderer3DState.Camera = Renderer3DState.DefaultCamera;
    }

    void Renderer3D::Render() {
        Renderer3DState.Stats.Vertices = Renderer3DState.ShapeVertexCount;
        Renderer3DState.Stats.Indicies = Renderer3DState.ShapeIndexCount;

        auto& renderer = BL_APP.GetRenderer();

        // shape buffer
        if (Renderer3DState.ShapeIndexCount > 0) {
            BlDrawBufferLayout vertPosLayout;
            vertPosLayout.Index = 0;
            vertPosLayout.Count = 3;
            vertPosLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertPosLayout.Stride = sizeof(BlShapeVertex);
            vertPosLayout.Offset = offsetof(BlShapeVertex, Pos);

            BlDrawBufferLayout vertColorLayout;
            vertColorLayout.Index = 1;
            vertColorLayout.Count = 4;
            vertColorLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertColorLayout.Stride = sizeof(BlShapeVertex);
            vertColorLayout.Offset = offsetof(BlShapeVertex, Color);

            BlDrawBufferLayout vertTexCoordLayout;
            vertTexCoordLayout.Index = 2;
            vertTexCoordLayout.Count = 2;
            vertTexCoordLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertTexCoordLayout.Stride = sizeof(BlShapeVertex);
            vertTexCoordLayout.Offset = offsetof(BlShapeVertex, TexCoord);

            BlDrawBufferLayout vertTexIndexLayout;
            vertTexIndexLayout.Index = 3;
            vertTexIndexLayout.Count = 1;
            vertTexIndexLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertTexIndexLayout.Stride = sizeof(BlShapeVertex);
            vertTexIndexLayout.Offset = offsetof(BlShapeVertex, TexIndex);

            Renderer3DState.ShapeDrawBuffer.Vertices = Renderer3DState.ShapeVertices.data();
            Renderer3DState.ShapeDrawBuffer.VertexCount = Renderer3DState.ShapeVertexCount;
            Renderer3DState.ShapeDrawBuffer.VertexSize = sizeof(BlShapeVertex);

            Renderer3DState.ShapeDrawBuffer.Indices = Renderer3DState.ShapeIndices.data();
            Renderer3DState.ShapeDrawBuffer.IndexCount = Renderer3DState.ShapeIndexCount;
            Renderer3DState.ShapeDrawBuffer.IndexSize = sizeof(u32);

            renderer.SubmitDrawBuffer(Renderer3DState.ShapeDrawBuffer);
            
            renderer.SetBufferLayout(vertPosLayout);
            renderer.SetBufferLayout(vertColorLayout);
            renderer.SetBufferLayout(vertTexCoordLayout);
            renderer.SetBufferLayout(vertTexIndexLayout);
            
            renderer.BindShader(Renderer3DState.ShapeShader);
            for (u32 i = 0; i < Renderer3DState.CurrentTexIndex; i++) {
                renderer.BindTexture(Renderer3DState.CurrentAttachedTextures[i], i);
            }

            int samplers[16]; // opengl texture IDs
            for (u32 i = 0; i < Renderer3DState.CurrentAttachedTextures.size(); i++) {
                samplers[i] = i;
            }

            BlShader shader = Renderer3DState.ShapeShader;
            shader.SetIntArray("u_Textures", 16, samplers);
            shader.SetMatrix("u_Projection", Renderer3DState.Camera.GetCameraMatrixFloat());

            renderer.DrawIndexed(Renderer3DState.ShapeIndexCount);

            renderer.UnBindTexture();

            Renderer3DState.Stats.DrawCalls++;
            Renderer3DState.Stats.ActiveTextures = Renderer3DState.CurrentTexIndex;
            Renderer3DState.Stats.ReservedTextures = 1;

            // clear buffer after rendering
            Renderer3DState.ShapeIndices.clear();
            Renderer3DState.ShapeVertices.clear();

            // reserve memory again
            Renderer3DState.ShapeIndices.reserve(2048);
            Renderer3DState.ShapeVertices.reserve(2048);
            Renderer3DState.ShapeIndexCount = 0;
            Renderer3DState.ShapeVertexCount = 0;
            Renderer3DState.CurrentTexIndex = 1; // 0 is reserved and never changes
        }

        // circle buffer
        if (Renderer3DState.CircleIndexCount > 0) {
            BlDrawBufferLayout vertPosLayout;
            vertPosLayout.Index = 0;
            vertPosLayout.Count = 3;
            vertPosLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertPosLayout.Stride = sizeof(BlCircleVertex);
            vertPosLayout.Offset = offsetof(BlCircleVertex, Pos);

            BlDrawBufferLayout vertColorLayout;
            vertColorLayout.Index = 1;
            vertColorLayout.Count = 4;
            vertColorLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertColorLayout.Stride = sizeof(BlCircleVertex);
            vertColorLayout.Offset = offsetof(BlCircleVertex, Color);

            BlDrawBufferLayout vertTexCoordLayout;
            vertTexCoordLayout.Index = 2;
            vertTexCoordLayout.Count = 2;
            vertTexCoordLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertTexCoordLayout.Stride = sizeof(BlCircleVertex);
            vertTexCoordLayout.Offset = offsetof(BlCircleVertex, TexCoord);

            Renderer3DState.CircleDrawBuffer.Vertices = Renderer3DState.CircleVertices.data();
            Renderer3DState.CircleDrawBuffer.VertexCount = Renderer3DState.CircleVertexCount;
            Renderer3DState.CircleDrawBuffer.VertexSize = sizeof(BlCircleVertex);

            Renderer3DState.CircleDrawBuffer.Indices = Renderer3DState.CircleIndices.data();
            Renderer3DState.CircleDrawBuffer.IndexCount = Renderer3DState.CircleIndexCount;
            Renderer3DState.CircleDrawBuffer.IndexSize = sizeof(u32);

            renderer.SubmitDrawBuffer(Renderer3DState.CircleDrawBuffer);

            renderer.SetBufferLayout(vertPosLayout);
            renderer.SetBufferLayout(vertColorLayout);
            renderer.SetBufferLayout(vertTexCoordLayout);

            renderer.BindShader(Renderer3DState.CircleShader);

            Renderer3DState.CircleShader.SetMatrix("u_Projection", Renderer3DState.Camera.GetCameraMatrixFloat());

            renderer.DrawIndexed(Renderer3DState.CircleIndexCount);

            Renderer3DState.Stats.DrawCalls++;

            // clear buffer after rendering
            Renderer3DState.CircleIndices.clear();
            Renderer3DState.CircleVertices.clear();

            // reserve memory again
            Renderer3DState.CircleIndices.reserve(1024);
            Renderer3DState.CircleVertices.reserve(1024);
            Renderer3DState.CircleIndexCount = 0;
            Renderer3DState.CircleVertexCount = 0;
        }

        // font buffer
        if (Renderer3DState.FontIndexCount > 0) {
            BlDrawBufferLayout vertPosLayout;
            vertPosLayout.Index = 0;
            vertPosLayout.Count = 3;
            vertPosLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertPosLayout.Stride = sizeof(BlFontVertex);
            vertPosLayout.Offset = offsetof(BlFontVertex, Pos);

            BlDrawBufferLayout vertColorLayout;
            vertColorLayout.Index = 1;
            vertColorLayout.Count = 4;
            vertColorLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertColorLayout.Stride = sizeof(BlFontVertex);
            vertColorLayout.Offset = offsetof(BlFontVertex, Color);

            BlDrawBufferLayout vertTexCoordLayout;
            vertTexCoordLayout.Index = 2;
            vertTexCoordLayout.Count = 2;
            vertTexCoordLayout.Type = BlDrawBufferLayout::ElementType::Float;
            vertTexCoordLayout.Stride = sizeof(BlFontVertex);
            vertTexCoordLayout.Offset = offsetof(BlFontVertex, TexCoord);

            Renderer3DState.FontDrawBuffer.Vertices = Renderer3DState.FontVertices.data();
            Renderer3DState.FontDrawBuffer.VertexCount = Renderer3DState.FontVertexCount;
            Renderer3DState.FontDrawBuffer.VertexSize = sizeof(BlFontVertex);
                            
            Renderer3DState.FontDrawBuffer.Indices = Renderer3DState.FontIndices.data();
            Renderer3DState.FontDrawBuffer.IndexCount = Renderer3DState.FontIndexCount;
            Renderer3DState.FontDrawBuffer.IndexSize = sizeof(u32);

            renderer.SubmitDrawBuffer(Renderer3DState.FontDrawBuffer);
            
            renderer.SetBufferLayout(vertPosLayout);
            renderer.SetBufferLayout(vertColorLayout);
            renderer.SetBufferLayout(vertTexCoordLayout);
            
            renderer.BindShader(Renderer3DState.FontShader);
            renderer.BindTexture(Renderer3DState.CurrentFontAtlas);

            BlShader shader = Renderer3DState.FontShader;
            shader.SetMatrix("u_Projection", Renderer3DState.Camera.GetCameraMatrixFloat());

            renderer.DrawIndexed(Renderer3DState.FontIndexCount);

            renderer.UnBindTexture();

            Renderer3DState.Stats.DrawCalls++;
            // Renderer3DState.Info.ActiveTextures = Renderer3DState.CurrentFontIndex;
            // Renderer3DState.Info.ReservedTextures = 1;

            // clear buffer after rendering
            Renderer3DState.FontIndices.clear();
            Renderer3DState.FontVertices.clear();

            // reserve memory again
            Renderer3DState.FontIndices.reserve(2048);
            Renderer3DState.FontVertices.reserve(2048);
            Renderer3DState.FontIndexCount = 0;
            Renderer3DState.FontVertexCount = 0;
            Renderer3DState.CurrentTexIndex = 0;
        }
    }

    BlVec2<f32> Renderer3D::MeasureText(const std::string& text, Font& font, TextParams parameters) {
        f32 fsScale = 1.0f / (font.Ascender - font.Descender);
        f32 currentX = 0.0f;
        f32 currentY = 0.0f;

        for (u32 c = 0; c < text.length(); c++) {
            auto glyph = font.GetGlyphInfo(text.at(c), 0);
            BlRec& pb = glyph.PlaneRect;

            currentX += fsScale * (glyph.AdvanceX + parameters.Kerning);
            currentY = std::max(currentY, fsScale * (glyph.PlaneRect.y - glyph.PlaneRect.h));
        }

        return BlVec2<f32>(currentX, currentY);
    }

    Renderer3DStats Renderer3D::GetRendererStats() {
        return Renderer3DState.Stats;
    }

} // namespace Blackberry