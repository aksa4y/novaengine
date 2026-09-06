#include <nova/runtime/Renderer.h>

#include <algorithm>
#include <cmath>

namespace Nova::Runtime {

namespace {
void copy_clear_color(float destination[4], const float source[4]) {
    std::copy(source, source + 4, destination);
}

bool valid_clear_color(const float color[4]) {
    if (!color) return false;
    for (int i = 0; i < 4; ++i) {
        if (!std::isfinite(color[i])) return false;
    }
    return true;
}
}

bool Renderer::initialize(RHI::Device& device, const RHI::SwapchainDesc& swapchain_desc) {
    shutdown();
    if (!swapchain_desc.width || !swapchain_desc.height) return false;

    auto swapchain = device.create_swapchain(swapchain_desc);
    if (!swapchain) return false;

    auto command_buffer = device.create_command_buffer();
    if (!command_buffer) return false;

    device_ = &device;
    swapchain_ = std::move(swapchain);
    command_buffer_ = std::move(command_buffer);
    return true;
}

void Renderer::shutdown() {
    if (device_) device_->wait_idle();
    render_pass_active_ = false;
    frame_active_ = false;
    submitted_ = false;
    command_buffer_.reset();
    swapchain_.reset();
    device_ = nullptr;
}

bool Renderer::begin_frame(const float clear_color[4]) {
    if (!is_initialized() || frame_active_) return false;
    if (clear_color) {
        if (!valid_clear_color(clear_color)) return false;
        copy_clear_color(clear_color_, clear_color);
    }
    if (!swapchain_->acquire()) return false;

    command_buffer_->begin();
    frame_active_ = true;
    submitted_ = false;
    render_pass_active_ = false;
    return true;
}

bool Renderer::begin_render_pass() {
    if (!frame_active_ || render_pass_active_) return false;
    RHI::RenderPassDesc desc;
    desc.target = swapchain_.get();
    copy_clear_color(desc.clear_color, clear_color_);
    command_buffer_->begin_render_pass(desc);
    command_buffer_->set_viewport(0.0f, 0.0f,
                                  static_cast<float>(swapchain_->width()),
                                  static_cast<float>(swapchain_->height()));
    command_buffer_->set_scissor(0, 0, swapchain_->width(), swapchain_->height());
    render_pass_active_ = true;
    return true;
}

bool Renderer::end_render_pass() {
    if (!render_pass_active_) return false;
    command_buffer_->end_render_pass();
    render_pass_active_ = false;
    return true;
}

bool Renderer::draw_mesh(const Mesh& mesh, RHI::Pipeline& pipeline) {
    if (!render_pass_active_ || !frame_active_ || !mesh.is_ready()) return false;
    if (mesh.device() != device_) return false;
    command_buffer_->set_pipeline(&pipeline);
    command_buffer_->set_vertex_buffer(mesh.vertex_buffer(), 0);
    command_buffer_->draw(mesh.vertex_count());
    return true;
}

bool Renderer::submit() {
    if (!frame_active_ || render_pass_active_ || submitted_) return false;
    command_buffer_->end();
    if (!device_->submit(*command_buffer_, swapchain_.get())) return false;
    submitted_ = true;
    return true;
}

bool Renderer::end_frame() {
    if (!frame_active_ || !submitted_) return false;
    const bool presented = swapchain_->present();
    frame_active_ = false;
    submitted_ = false;
    return presented;
}

} // namespace Nova::Runtime
