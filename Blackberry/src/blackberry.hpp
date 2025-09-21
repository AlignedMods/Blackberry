// for use by applications using Blackberry, NOT for internal use!!!

#pragma once

// windowing, rendering, input
#include "blackberry/application/application.hpp"
#include "blackberry/application/window.hpp"
#include "blackberry/application/renderer.hpp"
#include "blackberry/application/layer.hpp"
#include "blackberry/input/keycodes.hpp"
#include "blackberry/input/mousecodes.hpp"

// events
#include "blackberry/event/event.hpp"
#include "blackberry/event/key_events.hpp"
#include "blackberry/event/mouse_events.hpp"
#include "blackberry/event/window_events.hpp"

// utils
#include "blackberry/log.hpp"
#include "blackberry/types.hpp"
#include "blackberry/util.hpp"

// rendering abstractions
#include "blackberry/rendering/rendering.hpp"

// NOTE: entry point should be defined only ONCE (in the file where your CreateApplication function is!)
#ifdef BL_ENTRYPOINT
#include "blackberry/application/entrypoint.hpp"
#endif
