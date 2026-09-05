#pragma once

#include "RHI.h"
#include <cstdint>

namespace Nova::RHI {

enum class Capability : std::uint8_t {
    Textures,
    RenderTargets,
    Compute,
    Instancing,
    DebugMarkers,
    Present,
};

struct Capabilities {
    Backend backend = Backend::Null;
    bool textures = true;
    bool render_targets = true;
    bool compute = false;
    bool instancing = true;
    bool debug_markers = false;
    bool present = false;
};

Capabilities query_capabilities(const Device& device) noexcept;

} // namespace Nova::RHI
