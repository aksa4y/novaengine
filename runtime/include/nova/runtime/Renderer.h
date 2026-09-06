#pragma once

#include <cstdint>
#include <memory>

#include <nova/rhi/RHI.h>
#include <nova/runtime/Mesh.h>

namespace Nova::Runtime {

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
