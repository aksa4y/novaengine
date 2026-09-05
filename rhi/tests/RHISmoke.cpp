#include <cassert>

#include <nova/rhi/RHI.h>

int main() {
    using namespace Nova::RHI;

    auto device = create_device({Backend::Null, true});
    assert(device);
    assert(device->backend() == Backend::Null);
    assert(device->backend_name() == "Null");

    auto texture = device->create_texture({64, 64, 1});
    assert(texture);

    device->begin_frame();
    device->end_frame();

    // Unsupported backends must fail explicitly until their adapters are wired.
    assert(!create_device({Backend::Vulkan, false}));
    assert(!create_device({Backend::WebGPU, false}));

    return 0;
}
