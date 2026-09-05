#include "nova/rhi/RHI.h"

namespace Nova::RHI {

Backend default_backend() noexcept {
#if defined(__EMSCRIPTEN__)
    return Backend::WebGPU;
#elif defined(__ANDROID__)
    return Backend::Vulkan;
#elif defined(_WIN32)
    return Backend::D3D12;
#elif defined(__APPLE__)
    return Backend::Metal;
#elif defined(__linux__)
    return Backend::Vulkan;
#else
    return Backend::Sokol;
#endif
}

} // namespace Nova::RHI
