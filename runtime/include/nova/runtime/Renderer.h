#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include <nova/rhi/RHI.h>

namespace Nova::Runtime {

// Runtime-owned mesh resource. It intentionally stores only RHI resources and
// never depends on Editor or legacy engine types.
class Mesh final {
public:
    Mesh() = default;
    ~Mesh() = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    bool upload(RHI::Device& device, const void* vertices, std::size_t vertex_bytes,
                std::uint32_t vertex_count, std::uint32_t vertex_stride) {
        release();
        if (!vertices || !vertex_bytes || !vertex_count || !vertex_stride) return false;
        if (vertex_bytes / vertex_stride < vertex_count) return false;

        auto buffer = device.create_buffer({vertex_bytes, RHI::BufferUsage::Vertex, true});
        if (!buffer || !buffer->write(0, vertices, vertex_bytes)) return false;

        device_ = &device;
        vertex_buffer_ = std::move(buffer);
        vertex_count_ = vertex_count;
        vertex_stride_ = vertex_stride;
        return true;
    }

    void release() {
        vertex_buffer_.reset();
        vertex_count_ = 0;
        vertex_stride_ = 0;
        device_ = nullptr;
    }

    bool is_ready() const noexcept { return vertex_buffer_ != nullptr && vertex_count_ != 0; }
    RHI::Buffer* vertex_buffer() const noexcept { return vertex_buffer_.get(); }
    std::uint32_t vertex_count() const noexcept { return vertex_count_; }
    std::uint32_t vertex_stride() const noexcept { return vertex_stride_; }
    RHI::Device* device() const noexcept { return device_; }

private:
    RHI::Device* device_ = nullptr;
    std::unique_ptr<RHI::Buffer> vertex_buffer_;
    std::uint32_t vertex_count_ = 0;
    std::uint32_t vertex_stride_ = 0;
};

class Renderer final {
public:
    Renderer() = default;
    ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool initialize(RHI::Device& device, const RHI::SwapchainDesc& swapchain_desc);
    void shutdown();

    bool begin_frame(const float clear_color[4] = nullptr);
    bool begin_render_pass();
    bool end_render_pass();
    bool draw_mesh(const Mesh& mesh, RHI::Pipeline& pipeline);
    bool submit();
    bool end_frame();

    RHI::Device* device() const noexcept { return device_; }
    RHI::Swapchain* swapchain() const noexcept { return swapchain_.get(); }
    RHI::CommandBuffer* command_buffer() const noexcept { return command_buffer_.get(); }
    bool is_initialized() const noexcept {
        return device_ != nullptr && swapchain_ != nullptr && command_buffer_ != nullptr;
    }
    bool frame_active() const noexcept { return frame_active_; }

private:
    RHI::Device* device_ = nullptr;
    std::unique_ptr<RHI::Swapchain> swapchain_;
    std::unique_ptr<RHI::CommandBuffer> command_buffer_;
    bool frame_active_ = false;
    bool render_pass_active_ = false;
    bool submitted_ = false;
    float clear_color_[4] = {0.05f, 0.05f, 0.05f, 1.0f};
};

} // namespace Nova::Runtime
