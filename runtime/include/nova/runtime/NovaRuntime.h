#pragma once

#include <nova/runtime/Runtime.h>
#include <nova/runtime/RuntimeConfig.h>

namespace Nova::Runtime {

// Stable umbrella header for game-facing Runtime APIs.
// New code should include this header instead of legacy engine headers.
using RuntimeInstance = Runtime;

} // namespace Nova::Runtime
