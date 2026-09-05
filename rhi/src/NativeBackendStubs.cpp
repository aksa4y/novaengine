#include "nova/rhi/BackendFactory.h"

namespace Nova::RHI {

// These default implementations deliberately avoid pulling platform SDK
// headers into the portable RHI target. Platform modules replace them when
// their native backend is enabled by CMake.
std::unique_ptr<Device> create_vulkan_device(const DeviceDesc&) { return nullptr; }
std::unique_ptr<Device> create_d3d11_device(const DeviceDesc&) { return nullptr; }
std::unique_ptr<Device> create_d3d12_device(const DeviceDesc&) { return nullptr; }
std::unique_ptr<Device> create_metal_device(const DeviceDesc&) { return nullptr; }
std::unique_ptr<Device> create_opengl_device(const DeviceDesc&) { return nullptr; }
std::unique_ptr<Device> create_webgpu_device(const DeviceDesc&) { return nullptr; }

} // namespace Nova::RHI
