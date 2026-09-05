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
    switch (desc.backend) {
        case Backend::Null:
            return create_null_device();
        case Backend::Sokol:
            return create_sokol_device();
        default:
            return nullptr;
    }
}

} // namespace Nova::RHI
