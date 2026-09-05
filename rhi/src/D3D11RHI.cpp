#include "nova/rhi/BackendFactory.h"

#if defined(_WIN32) && defined(NOVA_RHI_NATIVE_D3D11)
#include <d3d11.h>
#include <wrl/client.h>

namespace Nova::RHI {
namespace {
class D3D11Device final : public Device {
public:
    explicit D3D11Device(bool debug) {
        UINT flags = debug ? D3D11_CREATE_DEVICE_DEBUG : 0u;
        const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
        D3D_FEATURE_LEVEL selected{};
        D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
                          levels, 2, D3D11_SDK_VERSION, &device_, &selected, &context_);
    }

    bool valid() const noexcept { return device_ != nullptr && context_ != nullptr; }
    Backend backend() const noexcept override { return Backend::D3D11; }
    std::string_view backend_name() const noexcept override { return "D3D11"; }
    std::unique_ptr<Texture> create_texture(const TextureDesc&) override { return nullptr; }
    void begin_frame() override {}
    void end_frame() override {}

private:
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
};
}

std::unique_ptr<Device> create_d3d11_device(const DeviceDesc& desc) {
    auto device = std::make_unique<D3D11Device>(desc.debug);
    return device->valid() ? std::move(device) : nullptr;
}
}
#else
namespace Nova::RHI {
std::unique_ptr<Device> create_d3d11_device(const DeviceDesc&) { return nullptr; }
}
#endif
