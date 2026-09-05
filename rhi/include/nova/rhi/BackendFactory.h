#pragma once

#include <memory>
#include <nova/rhi/RHI.h>

namespace Nova::RHI {

// Native adapter entry points. Each platform-specific translation unit may
// return nullptr when its API/SDK is unavailable, keeping the RHI portable.
std::unique_ptr<Device> create_vulkan_device(const DeviceDesc& desc);
std::unique_ptr<Device> create_d3d11_device(const DeviceDesc& desc);
std::unique_ptr<Device> create_d3d12_device(const DeviceDesc& desc);
std::unique_ptr<Device> create_metal_device(const DeviceDesc& desc);
std::unique_ptr<Device> create_opengl_device(const DeviceDesc& desc);
std::unique_ptr<Device> create_webgpu_device(const DeviceDesc& desc);

} // namespace Nova::RHI
