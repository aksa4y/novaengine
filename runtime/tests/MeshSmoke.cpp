#include <nova/runtime/Mesh.h>
#include <nova/rhi/RHI.h>

#include <array>
#include <cassert>

int main() {
    auto device = Nova::RHI::create_null_device();
    assert(device);

    struct Vertex {
        float position[3];
    };

    constexpr std::array<Vertex, 4> vertices{{
        {{{-1.0f, -1.0f, 0.0f}}},
        {{{ 1.0f, -1.0f, 0.0f}}},
        {{{ 1.0f,  1.0f, 0.0f}}},
        {{{-1.0f,  1.0f, 0.0f}}},
    }};
    constexpr std::array<std::uint32_t, 6> indices{{0, 1, 2, 2, 3, 0}};

    Nova::Runtime::Mesh mesh;
    assert(mesh.upload(device ? *device : *Nova::RHI::create_null_device(),
                       vertices.data(), sizeof(vertices),
                       static_cast<std::uint32_t>(vertices.size()), sizeof(Vertex)));
    assert(mesh.is_ready());
    assert(!mesh.is_indexed());
    assert(mesh.vertex_count() == 4);

    assert(mesh.upload_indexed(*device,
                               vertices.data(), sizeof(vertices),
                               static_cast<std::uint32_t>(vertices.size()), sizeof(Vertex),
                               indices.data(), sizeof(indices),
                               static_cast<std::uint32_t>(indices.size())));
    assert(mesh.is_ready());
    assert(mesh.is_indexed());
    assert(mesh.index_count() == 6);
    assert(mesh.index_buffer() != nullptr);

    Nova::Runtime::Mesh invalid;
    assert(!invalid.upload_indexed(*device,
                                   vertices.data(), sizeof(vertices),
                                   4, sizeof(Vertex),
                                   indices.data(), sizeof(indices), 0));
    assert(!invalid.is_ready());

    mesh.release();
    assert(!mesh.is_ready());
    assert(!mesh.is_indexed());
    return 0;
}
