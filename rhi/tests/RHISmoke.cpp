#include <cassert>

#include <nova/rhi/Capabilities.h>
#include <nova/rhi/RHI.h>

int main() {
    using namespace Nova::RHI;

    auto null_device = create_device({Backend::Null, true});
    assert(null_device);
    assert(null_device->backend() == Backend::Null);
    assert(null_device->backend_name() == "Null");

    auto null_caps = query_capabilities(*null_device);
    assert(null_caps.backend == Backend::Null);
    assert(null_caps.textures);
    assert(!null_caps.compute);
    assert(!null_caps.present);

    auto texture = null_device->create_texture({64, 64, 1});
    assert(texture);

    null_device->begin_frame();
    null_device->end_frame();

    auto sokol_device = create_device({Backend::Sokol, false});
    assert(sokol_device);
    assert(sokol_device->backend() == Backend::Sokol);
    assert(sokol_device->backend_name() == "Sokol");
    assert(sokol_device->create_texture({32, 32, 1}));
    assert(!sokol_device->create_texture({0, 32, 1}));

    auto sokol_caps = query_capabilities(*sokol_device);
    assert(sokol_caps.backend == Backend::Sokol);
    assert(sokol_caps.present);
    assert(sokol_caps.compute);

    // Unsupported backends must fail explicitly until their native adapters
    // are linked into Nova RHI.
    assert(!create_device({Backend::Vulkan, false}));
    assert(!create_device({Backend::D3D11, false}));
    assert(!create_device({Backend::D3D12, false}));
    assert(!create_device({Backend::Metal, false}));
    assert(!create_device({Backend::OpenGL, false}));
    assert(!create_device({Backend::WebGPU, false}));

    return 0;
}
