#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

namespace Nova::RHI {

enum class Backend : std::uint8_t {
    Null,
    Sokol,
    Vulkan,
    D3D11,
    D3D12,
    Metal,
    OpenGL,
    WebGPU,
};

struct DeviceDesc {
    Backend backend = Backend::Null;
    bool debug = false;
};

struct TextureDesc {
    std::uint32_t width = 1;
    std::uint32_t height = 1;
    std::uint32_t mip_levels = 1;
};

struct Capabilities {
    bool graphics = false;
    bool compute = false;
    bool presentation = false;
    bool debug_markers = false;
};

class Texture {
public:
    virtual ~Texture() = default;
};

class Device {
public:
    virtual ~Device() = default;

    virtual Backend backend() const noexcept = 0;
    virtual std::string_view backend_name() const noexcept = 0;
    virtual Capabilities capabilities() const noexcept = 0;

    virtual std::unique_ptr<Texture> create_texture(const TextureDesc& desc) = 0;
    virtual void begin_frame() = 0;
    virtual void end_frame() = 0;
};

// Stable backend factories. The native implementations are supplied by
// platform modules; the portable RHI keeps nullptr fallbacks when a SDK is
// unavailable.
std::unique_ptr<Device> create_null_device();
std::unique_ptr<Device> create_sokol_device();
std::unique_ptr<Device> create_vulkan_device(const DeviceDesc& desc);
std::unique_ptr<Device> create_d3d11_device(const DeviceDesc& desc);
std::unique_ptr<Device> create_d3d12_device(const DeviceDesc& desc);
std::unique_ptr<Device> create_metal_device(const DeviceDesc& desc);
std::unique_ptr<Device> create_opengl_device(const DeviceDesc& desc);
std::unique_ptr<Device> create_webgpu_device(const DeviceDesc& desc);

std::unique_ptr<Device> create_device(const DeviceDesc& desc = {});

} // namespace Nova::RHI
