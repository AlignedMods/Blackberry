#pragma once

#include "blackberry/renderer/texture.hpp"

namespace Blackberry {

    Ref<Texture2D> GenerateSkybox(const FS::Path& hdr);

} // namespace Blackberry