#include "nova/rhi/BackendFactory.h"
#include "nova/rhi/RHI.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace Nova::RHI {
namespace {
class NullBuffer final : public Buffer {
public:
    explicit NullBuffer(const BufferDesc& desc) : data_(desc.size) {}
    std::size_t size() const noexcept override { return data_.size(); }
    bool write(std::size_t offset, const void* data, std::size_t size) override {
        if (!data || offset > data_.size() || size > data_.size() - offset) return false;
        std::memcpy(data_.data() + offset, data, size);
        return true;
    }
private:
    std::vector<unsigned char> data_;
};

class NullTexture final : public Texture {};
class NullPipeline final : public Pipeline {};

class NullCommandBuffer final : public CommandBuffer {
public:
    void begin() override { recording_ = true; }
    void end() override { recording_ = false; }
    void begin_render_pass() override { render_pass_ = true; }
    void end_render_pass() override { render_pass_ = false; }
    void set_pipeline(Pipeline*) override {}
    void set_vertex_buffer(Buffer*, std::uint32_t) override {}
    void draw(std::uint32_t, std::uint32_t) override {}
private:
    bool recording_ = false;
    bool render_pass_ = false;
};

class NullSwapchain final : public Swapchain {
public:
    NullSwapchain(std::uint32_t width, std::uint32_t height) : width_(width), height_(height) {}
    std::uint32_t width() const noexcept override { return width_; }
    std::uint32_t height() const noexcept override { return height_; }
    bool acquire() override { acquired_ = true; return true; }
    bool present() override { const bool ok = acquired_; acquired_ = false; return ok; }
private:
    std::uint32_t width_ = 1;
    std::uint32_t height_ = 1;
    bool acquired_ = false;
};

class NullDevice final : public Device {
public:
    Backend backend() const noexcept override { return Backend::Null; }
    std::string_view backend_name() const noexcept override { return "Null"; }
    std::unique_ptr<Buffer> create_buffer(const BufferDesc& desc) override {
        return std::make_unique<NullBuffer>(desc);
    }
    std::unique_ptr<Texture> create_texture(const TextureDesc&) override {
        return std::make_unique<NullTexture>();
    }
    std::unique_ptr<CommandBuffer> create_command_buffer() override {
        return std::make_unique<NullCommandBuffer>();
    }
    std::unique_ptr<Pipeline> create_pipeline() override {
        return std::make_unique<NullPipeline>();
    }
    std::unique_ptr<Swapchain> create_swapchain(std::uint32_t width, std::uint32_t height) override {
        if (width == 0 || height == 0) return nullptr;
        return std::make_unique<NullSwapchain>(width, height);
    }
    void begin_frame() override {}
    void end_frame() override {}
};
} // namespace

std::unique_ptr<Device> create_null_device() {
    return std::make_unique<NullDevice>();
}

std::unique_ptr<Device> create_device(const DeviceDesc& desc) {
    if (desc.backend == Backend::Auto) {
        const Backend preferred = default_backend();
        if (preferred != Backend::Auto) {
            auto device = create_device({preferred, desc.debug});
            if (device) return device;
        }
        return create_sokol_device();
    }
    switch (desc.backend) {
        case Backend::Null: return create_null_device();
        case Backend::Sokol: return create_sokol_device();
        case Backend::Vulkan: return create_vulkan_device(desc);
        case Backend::D3D11: return create_d3d11_device(desc);
        case Backend::D3D12: return create_d3d12_device(desc);
        case Backend::Metal: return create_metal_device(desc);
        case Backend::OpenGL: return create_opengl_device(desc);
        case Backend::WebGPU: return create_webgpu_device(desc);
        case Backend::Auto: break;
    }
    return nullptr;
}

} // namespace Nova::RHI
