// Nova Engine public runtime entry point.
// SPDX-License-Identifier: MIT
#pragma once

#include <nova/rhi/RHI.h>
#include <nova/runtime/Runtime.h>

namespace Nova::Engine {

// Public composition point for applications and tools. This header intentionally
// exposes only the Nova Runtime/RHI surface and never includes Editor headers or
// the legacy engine implementation.
using RuntimeInstance = Nova::Runtime::Runtime;

inline bool initialize(RuntimeInstance& runtime, const RHI::DeviceDesc& device_desc = {}) {
    return runtime.initialize(device_desc);
}

inline void shutdown(RuntimeInstance& runtime) {
    runtime.shutdown();
}

inline bool initialized(const RuntimeInstance& runtime) noexcept {
    return runtime.is_initialized();
}

} // namespace Nova::Engine
