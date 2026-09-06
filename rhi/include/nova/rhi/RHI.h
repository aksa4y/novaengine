#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

#include <nova/rhi/Resources.h>

namespace Nova::RHI {

enum class Backend : std::uint8_t {
    Auto,
    Null,
    Sokol,
    Vulkan,
    D3D11,
    D3D12,
    Metal,
    OpenGL,
    WebGPU,
};

enum class NativeWindowType : std::uint8_t {
    None,
    Win32,
    X11,
    Wayland,
    Android,
    Cocoa,
    UIKit,
    Emscripten,
};

struct NativeWindowHandle {
    NativeWindowType type = NativeWindowType::None;
    std::uintptr_t display = 0;
    std::uintptr_t window = 0;
};

struct DeviceDesc {
    Backend backend = Backend::Auto;
    bool debug = false;
};

struct TextureDesc {
    std::uint32_t width = 1;
    std::uint32_t height = 1;
    std::uint32_t mip_levels = 1;
    TextureFormat format = TextureFormat::RGBA8_UNORM;
    std::uint32_t usage = static_cast<std::uint32_t>(TextureUsage::Sampled);
};

struct SwapchainDesc {
    std::uint32_t width = 1;
    std::uint32_t height = 1;
    TextureFormat format = TextureFormat::BGRA8_UNORM;
    std::uint32_t image_count = 2;
    NativeWindowHandle window{};
};

class Texture {
public:
    virtual ~Texture() = default;
    virtual bool upload(const void* data, std::size_t size) { return false; }
};

class Device {
public:
    virtual ~Device() = default;
    virtual Backend backend() const noexcept = 0;
    virtual std::string_view backend_name() const noexcept = 0;
    virtual std::unique_ptr<Buffer> create_buffer(const BufferDesc&) { return nullptr; }
    virtual std::unique_ptr<Texture> create_texture(const TextureDesc& desc) = 0;
    virtual std::unique_ptr<CommandBuffer> create_command_buffer() { return nullptr; }
    virtual std::unique_ptr<Pipeline> create_pipeline() { return nullptr; }
    virtual std::unique_ptr<Pipeline> create_pipeline(const GraphicsPipelineDesc&) { return nullptr; }
    virtual std::unique_ptr<Swapchain> create_swapchain(const SwapchainDesc&) { return nullptr; }

    std::unique_ptr<Swapchain> create_swapchain(std::uint32_t width, std::uint32_t height) {
        SwapchainDesc desc;
        desc.width = width;
        desc.height = height;
        return create_swapchain(desc);
    }

    virtual bool submit(CommandBuffer&, Swapchain* swapchain = nullptr) { return false; }
    virtual bool wait_idle() { return false; }

    virtual void begin_frame() = 0;
    virtual void end_frame() = 0;
};

Backend default_backend() noexcept;
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
