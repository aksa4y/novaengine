#include <nova/runtime/Mesh.h>

namespace Nova::Runtime {

bool Mesh::upload(RHI::Device& device,
                  const void* vertices,
                  std::size_t vertex_bytes,
                  std::uint32_t vertex_count,
                  std::uint32_t vertex_stride) {
    return upload_indexed(device, vertices, vertex_bytes, vertex_count, vertex_stride,
                          nullptr, 0, 0);
}

bool Mesh::upload_indexed(RHI::Device& device,
                          const void* vertices,
                          std::size_t vertex_bytes,
                          std::uint32_t vertex_count,
                          std::uint32_t vertex_stride,
                          const void* indices,
                          std::size_t index_bytes,
                          std::uint32_t index_count) {
    release();

    if (!vertices || vertex_bytes == 0 || vertex_count == 0 || vertex_stride == 0) {
        return false;
    }
    if (vertex_bytes / vertex_stride < vertex_count) {
        return false;
    }

    const bool indexed = index_count != 0 || index_bytes != 0 || indices != nullptr;
    if (indexed && (!indices || index_bytes == 0 || index_count == 0)) {
        return false;
    }
    if (indexed && index_bytes / sizeof(std::uint32_t) < index_count) {
        return false;
    }

    auto vertex_buffer = device.create_buffer({vertex_bytes, RHI::BufferUsage::Vertex, true});
    if (!vertex_buffer || !vertex_buffer->write(0, vertices, vertex_bytes)) {
        return false;
    }

    std::unique_ptr<RHI::Buffer> index_buffer;
    if (indexed) {
        index_buffer = device.create_buffer({index_bytes, RHI::BufferUsage::Index, true});
        if (!index_buffer || !index_buffer->write(0, indices, index_bytes)) {
            return false;
        }
    }

    device_ = &device;
    vertex_buffer_ = std::move(vertex_buffer);
    index_buffer_ = std::move(index_buffer);
    vertex_count_ = vertex_count;
    vertex_stride_ = vertex_stride;
    index_count_ = index_count;
    return true;
}

void Mesh::release() {
    index_buffer_.reset();
    vertex_buffer_.reset();
    index_count_ = 0;
    vertex_count_ = 0;
    vertex_stride_ = 0;
    device_ = nullptr;
}

} // namespace Nova::Runtime
