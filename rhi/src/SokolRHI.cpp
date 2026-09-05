#include "nova/rhi/RHI.h"

namespace Nova::RHI {
namespace {

// Transitional Sokol backend seam.
//
// The legacy engine already owns the real Sokol renderer. This adapter keeps
// the public Nova RHI backend identity and validates resources while ownership
// is migrated out of the legacy renderer.
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
    Capabilities capabilities() const noexcept override {
        return {true, false, true, false};
    }

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
