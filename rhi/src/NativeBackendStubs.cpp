#include "nova/rhi/BackendFactory.h"

namespace Nova::RHI {

// Keep fallback entry points only for backends that do not have a native
// translation unit active. This avoids duplicate symbols when a native
// adapter is enabled through CMake.
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
