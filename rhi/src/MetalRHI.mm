#include "nova/rhi/BackendFactory.h"

#if defined(__APPLE__) && defined(NOVA_RHI_NATIVE_METAL)
#import <Metal/Metal.h>

namespace Nova::RHI {
namespace {
class MetalDevice final : public Device {
public:
    explicit MetalDevice(bool debug) : debug_(debug), device_(MTLCreateSystemDefaultDevice()) {}

    bool valid() const noexcept { return device_ != nil; }
    Backend backend() const noexcept override { return Backend::Metal; }
    std::string_view backend_name() const noexcept override { return "Metal"; }
    std::unique_ptr<Texture> create_texture(const TextureDesc&) override { return nullptr; }
    void begin_frame() override {}
    void end_frame() override {}

private:
    bool debug_ = false;
    id<MTLDevice> device_ = nil;
};
}

std::unique_ptr<Device> create_metal_device(const DeviceDesc& desc) {
    auto device = std::make_unique<MetalDevice>(desc.debug);
    return device->valid() ? std::move(device) : nullptr;
}
}
#else
namespace Nova::RHI {
std::unique_ptr<Device> create_metal_device(const DeviceDesc&) { return nullptr; }
}
#endif
