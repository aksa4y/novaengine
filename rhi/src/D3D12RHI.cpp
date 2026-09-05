#include "nova/rhi/BackendFactory.h"

#if defined(_WIN32) && defined(NOVA_RHI_NATIVE_D3D12)
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace Nova::RHI {
namespace {
class D3D12Device final : public Device {
public:
    explicit D3D12Device(bool debug) {
        Microsoft::WRL::ComPtr<IDXGIFactory6> factory;
        UINT flags = debug ? DXGI_CREATE_FACTORY_DEBUG : 0u;
        if (CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory)) != S_OK) return;

        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
        for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC1 desc{};
            adapter->GetDesc1(&desc);
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                adapter.Reset();
                continue;
            }
            if (D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                  IID_PPV_ARGS(&device_)) == S_OK) {
                break;
            }
            adapter.Reset();
        }
    }

    bool valid() const noexcept { return device_ != nullptr; }
    Backend backend() const noexcept override { return Backend::D3D12; }
    std::string_view backend_name() const noexcept override { return "D3D12"; }
    std::unique_ptr<Texture> create_texture(const TextureDesc&) override { return nullptr; }
    void begin_frame() override {}
    void end_frame() override {}

private:
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
};
}

std::unique_ptr<Device> create_d3d12_device(const DeviceDesc& desc) {
    auto device = std::make_unique<D3D12Device>(desc.debug);
    return device->valid() ? std::move(device) : nullptr;
}
}
#else
namespace Nova::RHI {
std::unique_ptr<Device> create_d3d12_device(const DeviceDesc&) { return nullptr; }
}
#endif
