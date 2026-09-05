// Nova Engine public runtime entry point.
// SPDX-License-Identifier: MIT
#pragma once

#include <nova/rhi/RHI.h>
#include <nova/runtime/Runtime.h>

namespace Nova {

// Public composition point for applications and tools. This header intentionally
// exposes only the Nova Runtime/RHI surface and never includes Editor headers or
// the legacy engine implementation.
using Runtime = Nova::Runtime::Runtime;

namespace Engine {

inline bool initialize(Runtime& runtime, const RHI::DeviceDesc& device_desc = {}) {
    return runtime.initialize(device_desc);
}

inline void shutdown(Runtime& runtime) {
    runtime.shutdown();
}

inline bool initialized(const Runtime& runtime) noexcept {
    return runtime.is_initialized();
}

} // namespace Engine
} // namespace Nova
