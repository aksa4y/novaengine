#include <cassert>

#include <nova/runtime/Renderer.h>

int main() {
    auto device = Nova::RHI::create_device({Nova::RHI::Backend::Null, false});
    assert(device);

    Nova::Runtime::Renderer renderer;
    Nova::RHI::SwapchainDesc desc;
    desc.width = 1280;
    desc.height = 720;

    assert(renderer.initialize(*device, desc));
    assert(renderer.is_initialized());
    assert(renderer.swapchain()->width() == 1280);
    assert(renderer.swapchain()->height() == 720);

    const float clear_color[4] = {0.1f, 0.2f, 0.3f, 1.0f};
    assert(renderer.begin_frame(clear_color));
    assert(renderer.frame_active());
    assert(renderer.begin_render_pass());
    assert(renderer.command_buffer());
    assert(renderer.end_render_pass());
    assert(renderer.submit());
    assert(renderer.end_frame());
    assert(!renderer.frame_active());

    renderer.shutdown();
    assert(!renderer.is_initialized());
    return 0;
}
