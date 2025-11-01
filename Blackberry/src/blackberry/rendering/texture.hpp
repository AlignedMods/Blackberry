#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/image/image.hpp"
// #include "blackberry/assets/asset_manager.hpp"

struct BlTexture {
    BlTexture();

    void Create(u32 width, u32 height);
    void Create(const std::filesystem::path& path);
    void Create(const Blackberry::Image& image);
    void Create(void* pixels, u32 width, u32 height, Blackberry::ImageFormat pixelFormat);
    void Delete();

    void* ReadPixels();

    u32 ID = 0;
    u32 Width = 0;
    u32 Height = 0;
    Blackberry::ImageFormat Format = Blackberry::ImageFormat::RGBA8;
};

struct BlRenderTexture {
    BlRenderTexture();

    void Create(u32 width, u32 height);
    void Resize(u32 width, u32 height);
    void Delete();

    u32 ID = 0;
    BlTexture Texture;
    BlTexture DepthTexture;
};