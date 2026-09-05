#include "nova/rhi/RHI.h"

namespace Nova::RHI {
namespace {

// Transitional Sokol backend seam. Resource and frame ownership are being moved
// out of the legacy renderer without exposing Sokol types in the public RHI ABI.
class SokolTexture final : public Texture {
public:
    explicit SokolTexture(TextureDesc desc) : desc_(desc) {}

private:
    TextureDesc desc_{};
};

class SokolDevice final : public Device {
public:
    Backend backend() const noexcept override { return Backend::Sokol; }
    std::string_view backend_name() const noexcept override { return "Sokol"; }

    std::unique_ptr<Texture> create_texture(const TextureDesc& desc) override {
        if (desc.width == 0 || desc.height == 0 || desc.mip_levels == 0) {
            return nullptr;
        }
        return std::make_unique<SokolTexture>(desc);
    }

    void begin_frame() override {}
    void end_frame() override {}
};

} // namespace

std::unique_ptr<Device> create_sokol_device() {
    return std::make_unique<SokolDevice>();
}

} // namespace Nova::RHI
