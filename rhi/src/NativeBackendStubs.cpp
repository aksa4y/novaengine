#include "nova/rhi/BackendFactory.h"

// Portable fallback implementations. A platform-native adapter defines the
// corresponding NOVA_RHI_NATIVE_* macro and replaces this symbol.
namespace Nova::RHI {

#if !defined(NOVA_RHI_NATIVE_VULKAN)
std::unique_ptr<Device> create_vulkan_device(const DeviceDesc&) { return nullptr; }
#endif

#if !defined(NOVA_RHI_NATIVE_D3D11)
std::unique_ptr<Device> create_d3d11_device(const DeviceDesc&) { return nullptr; }
#endif

#if !defined(NOVA_RHI_NATIVE_D3D12)
std::unique_ptr<Device> create_d3d12_device(const DeviceDesc&) { return nullptr; }
#endif

#if !defined(NOVA_RHI_NATIVE_METAL)
std::unique_ptr<Device> create_metal_device(const DeviceDesc&) { return nullptr; }
#endif

#if !defined(NOVA_RHI_NATIVE_OPENGL)
std::unique_ptr<Device> create_opengl_device(const DeviceDesc&) { return nullptr; }
#endif

#if !defined(NOVA_RHI_NATIVE_WEBGPU)
std::unique_ptr<Device> create_webgpu_device(const DeviceDesc&) { return nullptr; }
#endif

} // namespace Nova::RHI
