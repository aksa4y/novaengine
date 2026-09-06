#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include <nova/rhi/RHI.h>

namespace Nova::Runtime {

class Mesh final {
public:
    Mesh() = default;
    ~Mesh() = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    bool upload(RHI::Device& device, const void* vertices, std::size_t vertex_bytes,
                std::uint32_t vertex_count, std::uint32_t vertex_stride);
    void release();

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

} // namespace Nova::Runtime
