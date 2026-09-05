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

std::unique_ptr<Device> create_device(const DeviceDesc& desc) {
    // The first migration step intentionally supports only Null. Existing
    // Doriax/Sokol rendering remains untouched until the adapter is introduced.
    if (desc.backend == Backend::Null || desc.backend == Backend::Sokol) {
        return std::make_unique<NullDevice>();
    }
    return nullptr;
}

} // namespace Nova::RHI
