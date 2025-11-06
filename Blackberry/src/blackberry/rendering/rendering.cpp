#include "blackberry/rendering/rendering.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/scene/camera.hpp"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct BlShapeVertex { // used for quads and triangles
    BlVec3 Pos;
    BlVec4 Color;
    BlVec2 TexCoord;
    f32 TexIndex = 0.0f; // OpenGL is weird with passing integers to shaders (also 0 is a white texture)
};

struct BlCircleVertex {
    BlVec3 Pos;
    BlVec4 Color;
    BlVec2 TexCoord;
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

    struct Renderer2DState {
        // shader
        BlShader ShapeShader;
        BlShader CircleShader;

        BlTexture WhiteTexture;

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

        // for textured shaped
        u32 CurrentTexIndex = 1; // NOTE: 0 is reserved for a blank white texture!!!
        std::array<BlTexture, 16> CurrentAttachedTextures;

        SceneCamera Camera;
        SceneCamera DefaultCamera; // by default the camera is initialized to a basic 1x scale pixel-by-pixel orthographic projection

        BlRenderer2DInfo Info;
    };

    static Renderer2DState State;

    static void CalculateRotation(BlVec3& vertexPos, BlVec3 pos, f32 rotation) {
        f32 sinR = glm::sin(glm::radians(rotation));
        f32 cosR = glm::cos(glm::radians(rotation));
        
        vertexPos = BlVec3(
            pos.x + (vertexPos.x - pos.x) * cosR - (vertexPos.y - pos.y) * sinR,
            pos.y + (vertexPos.x - pos.x) * sinR + (vertexPos.y - pos.y) * cosR,
            pos.z
        );
    }

    static void SetupQuad(BlVec3 pos, BlVec2 dimensions, f32 rotation, BlColor color, BlVec3* bl, BlVec3* tr, BlVec3* br, BlVec3* tl) {
        BlVec3 _bl = BlVec3(pos.x - dimensions.x / 2.0f, pos.y + dimensions.y / 2.0f, pos.z);
        BlVec3 _tr = BlVec3(pos.x + dimensions.x / 2.0f, pos.y - dimensions.y / 2.0f, pos.z);
        BlVec3 _br = BlVec3(pos.x + dimensions.x / 2.0f, pos.y + dimensions.y / 2.0f, pos.z);
        BlVec3 _tl = BlVec3(pos.x - dimensions.x / 2.0f, pos.y - dimensions.y / 2.0f, pos.z);

        if (rotation != 0.0f) {
            CalculateRotation(_bl, pos, rotation);
            CalculateRotation(_tr, pos, rotation);
            CalculateRotation(_br, pos, rotation);
            CalculateRotation(_tl, pos, rotation);
        }

        *bl = _bl;
        *tr = _tr;
        *br = _br;
        *tl = _tl;
    }

    static void NormalizeColor(BlColor color, BlVec4* outColor) {
        *outColor = BlVec4(
            static_cast<f32>(color.r) / 255.0f,
            static_cast<f32>(color.g) / 255.0f,
            static_cast<f32>(color.b) / 255.0f,
            static_cast<f32>(color.a) / 255.0f
        );
    }

    void Renderer2D::Init() {
        State.ShapeShader.Create(s_VertexShaderShapeSource, s_FragmentShaderShapeSource);
        State.CircleShader.Create(s_VertexShaderCircleSource, s_FragmentShaderCircleSource);
        State.WhiteTexture.Create(s_WhiteTextureData, 1, 1, ImageFormat::RGBA8);

        State.CurrentAttachedTextures[0] = State.WhiteTexture; // 0 is reserved for white
    }

    void Renderer2D::Shutdown() {
        State.ShapeShader.Delete();
        State.CircleShader.Delete();
    }

    void Renderer2D::Clear(BlColor color) {
        auto& renderer = BL_APP.GetRenderer();

        renderer.Clear(color);
    }

    void Renderer2D::NewFrame() {
        State.Info.DrawCalls = 0;
        State.Info.Vertices = 0;
        State.Info.Indicies = 0;
        State.Info.ActiveTextures = 0;
    }

    void Renderer2D::DrawRectangle(BlVec3 pos, BlVec2 dimensions, BlColor color) {
        DrawRectangle(pos, dimensions, 0.0f, color);
    }

    void Renderer2D::DrawRectangle(BlVec3 pos, BlVec2 dimensions, f32 rotation, BlColor color) {
        DrawTexturedQuad(pos, dimensions, BlRec(0, 0, 1, 1), State.WhiteTexture, rotation, color);
    }

    void Renderer2D::DrawTriangle(BlVec3 pos, BlVec2 dimensions, f32 rotation, BlColor color) {
        DrawTexturedTriangle(pos, dimensions, BlRec(0, 0, 1, 1), State.WhiteTexture, rotation, color);
    }

    void Renderer2D::DrawCircle(BlVec3 pos, f32 radius, BlColor color) {
        DrawElipse(pos, BlVec2(radius, radius), 0.0f, color);
    }

    void Renderer2D::DrawElipse(BlVec3 pos, BlVec2 dimensions, f32 rotation, BlColor color) {
        BlVec3 bl, tr, br, tl;
        BlVec4 normalizedColor;

        SetupQuad(pos, dimensions, 0.0f, color, &bl, &tr, &br, &tl);
        NormalizeColor(color, &normalizedColor);

        if (rotation != 0.0f) {
            CalculateRotation(bl, pos, rotation);
            CalculateRotation(tr, pos, rotation);
            CalculateRotation(br, pos, rotation);
            CalculateRotation(tl, pos, rotation);
        }

        // quad
        BlCircleVertex vertexBL = BlCircleVertex(bl, normalizedColor, BlVec2(0, 1));
        BlCircleVertex vertexTR = BlCircleVertex(tr, normalizedColor, BlVec2(1, 0));
        BlCircleVertex vertexBR = BlCircleVertex(br, normalizedColor, BlVec2(1, 1));
        BlCircleVertex vertexTL = BlCircleVertex(tl, normalizedColor, BlVec2(0, 0));

        State.CircleVertices.push_back(vertexBL);
        State.CircleVertices.push_back(vertexTR);
        State.CircleVertices.push_back(vertexBR);
        State.CircleVertices.push_back(vertexTL);

        // first triangle (bl, tr, br)
        State.CircleIndices.push_back(State.CircleVertexCount + 0);
        State.CircleIndices.push_back(State.CircleVertexCount + 1);
        State.CircleIndices.push_back(State.CircleVertexCount + 2);

        // second triangle (bl, tl, tr)
        State.CircleIndices.push_back(State.CircleVertexCount + 0);
        State.CircleIndices.push_back(State.CircleVertexCount + 3);
        State.CircleIndices.push_back(State.CircleVertexCount + 1);

        State.CircleVertexCount += 4;
        State.CircleIndexCount += 6;
    }

    void Renderer2D::DrawTexture(BlVec3 pos, BlTexture texture, f32 rotation, BlColor color) {
        DrawTextureEx(pos, BlVec2(static_cast<f32>(texture.Width), static_cast<f32>(texture.Height)), texture, rotation, color);
    }

    void Renderer2D::DrawTextureEx(BlVec3 pos, BlVec2 dimensions, BlTexture texture, f32 rotation, BlColor color)  {
        DrawTextureArea(pos, dimensions, BlRec(0.0f, 0.0f, static_cast<f32>(texture.Width), static_cast<f32>(texture.Height)), texture, rotation, color);
    }

    void Renderer2D::DrawTextureArea(BlVec3 pos, BlVec2 dimensions, BlRec area, BlTexture texture, f32 rotation, BlColor color) {
        DrawTexturedQuad(pos, dimensions, area, texture, rotation, color);
    }

    void Renderer2D::DrawTexturedQuad(BlVec3 pos, BlVec2 dimensions, BlRec area, BlTexture texture, f32 rotation, BlColor color) {
        f32 texIndex = 0.0f;

        if (State.CurrentTexIndex >= 16) {
            Render();
        }

        bool texAlreadyExists = false;

        for (u32 i = 0; i < State.CurrentTexIndex; i++) {
            if (texture.ID == State.CurrentAttachedTextures[i].ID) {
                texIndex = static_cast<f32>(i);
                texAlreadyExists = true;

                break;
            }
        }

        if (!texAlreadyExists) {
            texIndex = static_cast<f32>(State.CurrentTexIndex);

            State.CurrentAttachedTextures[State.CurrentTexIndex] = texture;
            State.CurrentTexIndex++;
            texAlreadyExists = true;
        }
        
        BlVec3 bl, tr, br, tl;
        BlVec4 normalizedColor;

        SetupQuad(pos, dimensions, rotation, color, &bl, &tr, &br, &tl);
        NormalizeColor(color, &normalizedColor);

        BlShapeVertex vertexBL = BlShapeVertex(bl, normalizedColor, BlVec2(0.0f, 1.0f), texIndex);
        BlShapeVertex vertexTR = BlShapeVertex(tr, normalizedColor, BlVec2(1.0f, 0.0f), texIndex);
        BlShapeVertex vertexBR = BlShapeVertex(br, normalizedColor, BlVec2(1.0f, 1.0f), texIndex);
        BlShapeVertex vertexTL = BlShapeVertex(tl, normalizedColor, BlVec2(0.0f, 0.0f), texIndex);

        // push all vertices (bl, tr, br, tl)
        State.ShapeVertices.push_back(vertexBL);
        State.ShapeVertices.push_back(vertexTR);
        State.ShapeVertices.push_back(vertexBR);
        State.ShapeVertices.push_back(vertexTL);

        // first triangle (bl, tr, br)
        State.ShapeIndices.push_back(State.ShapeVertexCount + 0);
        State.ShapeIndices.push_back(State.ShapeVertexCount + 1);
        State.ShapeIndices.push_back(State.ShapeVertexCount + 2);

        // second triangle (bl, tl, tr)
        State.ShapeIndices.push_back(State.ShapeVertexCount + 0);
        State.ShapeIndices.push_back(State.ShapeVertexCount + 3);
        State.ShapeIndices.push_back(State.ShapeVertexCount + 1);

        State.ShapeIndexCount += 6;
        State.ShapeVertexCount += 4;
    }

    void Renderer2D::DrawTexturedTriangle(BlVec3 pos, BlVec2 dimensions, BlRec area, BlTexture texture, f32 rotation, BlColor color) {
        f32 texIndex = 0.0f;

        if (State.CurrentTexIndex >= 16) {
            Render();
        }

        bool texAlreadyExists = false;

        for (u32 i = 0; i < State.CurrentTexIndex; i++) {
            if (texture.ID == State.CurrentAttachedTextures[i].ID) {
                texIndex = static_cast<f32>(i);
                texAlreadyExists = true;

                break;
            }
        }

        if (!texAlreadyExists) {
            texIndex = static_cast<f32>(State.CurrentTexIndex);

            State.CurrentAttachedTextures[State.CurrentTexIndex] = texture;
            State.CurrentTexIndex++;
            texAlreadyExists = true;
        }

        BlVec3 bl = BlVec3(pos.x - dimensions.x / 2.0f, pos.y + dimensions.y / 2.0f, pos.z);
        BlVec3 t = BlVec3(pos.x, pos.y - dimensions.y / 2.0f, pos.z);
        BlVec3 br = BlVec3(pos.x + dimensions.x / 2.0f, pos.y + dimensions.y / 2.0f, pos.z);

        if (rotation != 0.0f) {
            CalculateRotation(bl, pos, rotation);
            CalculateRotation(t, pos, rotation);
            CalculateRotation(br, pos, rotation);
        }

        BlVec4 normalizedColor;
        NormalizeColor(color, &normalizedColor);

        BlShapeVertex vertexBL = BlShapeVertex(bl, normalizedColor, BlVec2(0.0f, 1.0f), texIndex);
        BlShapeVertex vertexT = BlShapeVertex(t, normalizedColor, BlVec2(0.5f, 0.0f), texIndex);
        BlShapeVertex vertexBR = BlShapeVertex(br, normalizedColor, BlVec2(1.0f, 1.0f), texIndex);

        // push all the vertices
        State.ShapeVertices.push_back(vertexBL);
        State.ShapeVertices.push_back(vertexT);
        State.ShapeVertices.push_back(vertexBR);

        State.ShapeIndices.push_back(State.ShapeVertexCount + 0);
        State.ShapeIndices.push_back(State.ShapeVertexCount + 1);
        State.ShapeIndices.push_back(State.ShapeVertexCount + 2);

        State.ShapeVertexCount += 3;
        State.ShapeIndexCount += 3;
    }

    void Renderer2D::DrawRenderTexture(BlVec3 pos, BlVec2 dimensions, BlRenderTexture texture) {
        DrawTextureArea(pos, dimensions, BlRec(0.0f, 0.0f, texture.Texture.Width, texture.Texture.Height * -1.0f), texture.Texture);
    }

    void Renderer2D::AttachRenderTexture(BlRenderTexture texture) {
        auto& renderer = BL_APP.GetRenderer();

        renderer.AttachRenderTexture(texture);
    }

    void Renderer2D::DetachRenderTexture() {
        auto& renderer = BL_APP.GetRenderer();

        renderer.DetachRenderTexture();
    }

    void Renderer2D::SetProjection(SceneCamera camera) {
        State.Camera = camera;
    }

    void Renderer2D::ResetProjection() {
        State.Camera = State.DefaultCamera;
    }

    void Renderer2D::Render() {
        State.Info.Vertices = State.ShapeVertexCount;
        State.Info.Indicies = State.ShapeIndexCount;

        auto& renderer = BL_APP.GetRenderer();

        // shape buffer
        if (State.ShapeIndexCount > 0) {
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

            State.ShapeDrawBuffer.Vertices = State.ShapeVertices.data();
            State.ShapeDrawBuffer.VertexCount = State.ShapeVertexCount;
            State.ShapeDrawBuffer.VertexSize = sizeof(BlShapeVertex);

            State.ShapeDrawBuffer.Indices = State.ShapeIndices.data();
            State.ShapeDrawBuffer.IndexCount = State.ShapeIndexCount;
            State.ShapeDrawBuffer.IndexSize = sizeof(u32);

            renderer.SubmitDrawBuffer(State.ShapeDrawBuffer);
            
            renderer.SetBufferLayout(vertPosLayout);
            renderer.SetBufferLayout(vertColorLayout);
            renderer.SetBufferLayout(vertTexCoordLayout);
            renderer.SetBufferLayout(vertTexIndexLayout);
            
            renderer.BindShader(State.ShapeShader);
            for (u32 i = 0; i < State.CurrentTexIndex; i++) {
                renderer.AttachTexture(State.CurrentAttachedTextures[i], i);
            }

            int samplers[16]; // opengl texture IDs
            for (u32 i = 0; i < State.CurrentAttachedTextures.size(); i++) {
                samplers[i] = i;
            }

            BlShader shader = State.ShapeShader;
            shader.SetIntArray("u_Textures", 16, samplers);
            shader.SetMatrix("u_Projection", State.Camera.GetCameraMatrixFloat());

            renderer.DrawIndexed(State.ShapeIndexCount);

            renderer.DetachTexture();

            State.Info.DrawCalls++;
            State.Info.ActiveTextures = State.CurrentTexIndex;
            State.Info.ReservedTextures = 1;

            // clear buffer after rendering
            State.ShapeIndices.clear();
            State.ShapeVertices.clear();

            // reserve memory again
            State.ShapeIndices.reserve(2048);
            State.ShapeVertices.reserve(2048);
            State.ShapeIndexCount = 0;
            State.ShapeVertexCount = 0;
            State.CurrentTexIndex = 1; // 0 is reserved and never changes
        }

        // circle buffer
        if (State.CircleIndexCount > 0) {
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

            State.CircleDrawBuffer.Vertices = State.CircleVertices.data();
            State.CircleDrawBuffer.VertexCount = State.CircleVertexCount;
            State.CircleDrawBuffer.VertexSize = sizeof(BlCircleVertex);

            State.CircleDrawBuffer.Indices = State.CircleIndices.data();
            State.CircleDrawBuffer.IndexCount = State.CircleIndexCount;
            State.CircleDrawBuffer.IndexSize = sizeof(u32);

            renderer.SubmitDrawBuffer(State.CircleDrawBuffer);

            renderer.SetBufferLayout(vertPosLayout);
            renderer.SetBufferLayout(vertColorLayout);
            renderer.SetBufferLayout(vertTexCoordLayout);

            renderer.BindShader(State.CircleShader);

            State.CircleShader.SetMatrix("u_Projection", State.Camera.GetCameraMatrixFloat());

            renderer.DrawIndexed(State.CircleIndexCount);

            State.Info.DrawCalls++;

            // clear buffer after rendering
            State.CircleIndices.clear();
            State.CircleVertices.clear();

            // reserve memory again
            State.CircleIndices.reserve(1024);
            State.CircleVertices.reserve(1024);
            State.CircleIndexCount = 0;
            State.CircleVertexCount = 0;
        }
    }

    BlRenderer2DInfo Renderer2D::GetRenderingInfo() {
        return State.Info;
    }

} // namespace Blackberry