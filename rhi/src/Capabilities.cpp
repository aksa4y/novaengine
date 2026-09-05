#include "nova/rhi/Capabilities.h"

namespace Nova::RHI {

Capabilities query_capabilities(const Device& device) noexcept {
    Capabilities result;
    result.backend = device.backend();
    switch (device.backend()) {
        case Backend::Null:
            result.present = false;
            result.compute = false;
            break;
        case Backend::Sokol:
        case Backend::OpenGL:
        case Backend::WebGPU:
            result.present = true;
            result.compute = true;
            break;
        case Backend::Vulkan:
        case Backend::D3D11:
        case Backend::D3D12:
        case Backend::Metal:
            result.present = true;
            result.compute = true;
            result.debug_markers = true;
            break;
    }
    return result;
}

} // namespace Nova::RHI
