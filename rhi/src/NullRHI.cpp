#include "nova/rhi/BackendFactory.h"
#include "nova/rhi/RHI.h"

namespace Nova::RHI {
namespace {
class NullTexture final : public Texture {};

class NullDevice final : public Device {
public:
    Backend backend() const noexcept override { return Backend::Null; }
    std::string_view backend_name() const noexcept override { return "Null"; }

    std::unique_ptr<Texture> create_texture(const TextureDesc&) override {
        return std::make_unique<NullTexture>();
    }

    void begin_frame() override {}
    void end_frame() override {}
};
} // namespace

std::unique_ptr<Device> create_null_device() {
    return std::make_unique<NullDevice>();
}

std::unique_ptr<Device> create_device(const DeviceDesc& desc) {
    if (desc.backend == Backend::Auto) {
        const Backend preferred = default_backend();
        if (preferred != Backend::Auto) {
            auto device = create_device({preferred, desc.debug});
            if (device) {
                return device;
            }
        }
        return create_sokol_device();
    }

    switch (desc.backend) {
        case Backend::Null:    return create_null_device();
        case Backend::Sokol:   return create_sokol_device();
        case Backend::Vulkan:  return create_vulkan_device(desc);
        case Backend::D3D11:   return create_d3d11_device(desc);
        case Backend::D3D12:   return create_d3d12_device(desc);
        case Backend::Metal:   return create_metal_device(desc);
        case Backend::OpenGL:  return create_opengl_device(desc);
        case Backend::WebGPU:  return create_webgpu_device(desc);
        case Backend::Auto:    break;
    }
    return nullptr;
}

} // namespace Nova::RHI
