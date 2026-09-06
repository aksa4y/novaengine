#include <nova/runtime/Mesh.h>

namespace Nova::Runtime {

bool Mesh::upload(RHI::Device& device, const void* vertices, std::size_t vertex_bytes,
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

void Mesh::release() {
    vertex_buffer_.reset();
    vertex_count_ = 0;
    vertex_stride_ = 0;
    device_ = nullptr;
}

} // namespace Nova::Runtime
