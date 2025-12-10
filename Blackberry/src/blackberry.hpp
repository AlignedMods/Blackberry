// for use by applications using Blackberry, NOT for internal use!!!

#pragma once

// windowing, rendering, input
#include "blackberry/application/application.hpp"
#include "blackberry/application/window.hpp"
#include "blackberry/application/renderer.hpp"
#include "blackberry/application/layer.hpp"
#include "blackberry/input/keycodes.hpp"
#include "blackberry/input/mousebuttons.hpp"
#include "blackberry/input/input.hpp"
#include "blackberry/font/font.hpp"

// events
#include "blackberry/event/event.hpp"
#include "blackberry/event/key_events.hpp"
#include "blackberry/event/mouse_events.hpp"
#include "blackberry/event/window_events.hpp"

// ecs
#include "blackberry/ecs/components.hpp"

// meshes
#include "blackberry/model/mesh.hpp"

// scenes
#include "blackberry/scene/scene.hpp"
#include "blackberry/scene/entity.hpp"
#include "blackberry/scene/scene_renderer.hpp"

// utils
#include "blackberry/core/log.hpp"
#include "blackberry/core/types.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/core/memory.hpp"

// rendering abstractions
#include "blackberry/renderer/renderer3d.hpp"
#include "blackberry/renderer/texture.hpp"
#include "blackberry/renderer/shader.hpp"

// asset manager
#include "blackberry/assets/asset_manager.hpp"

// serializing
#include "blackberry/scene/scene_serializer.hpp"

// os
#include "blackberry/os/os.hpp"

// projects
#include "blackberry/project/project.hpp"

// imgui
#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/misc/cpp/imgui_stdlib.h"

// json
#include "../vendor/json/json.hpp"
using json = nlohmann::json;

// NOTE: entry point should be defined only ONCE (in the file where your CreateApplication function is!)
#ifdef BL_ENTRYPOINT
#include "blackberry/application/entrypoint.hpp"
#endif
