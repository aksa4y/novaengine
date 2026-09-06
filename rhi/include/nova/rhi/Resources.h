#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace Nova::RHI {

enum class BufferUsage : std::uint8_t {
    Vertex,
    Index,
    Uniform,
    Storage,
    Transfer,
};

enum class TextureFormat : std::uint8_t {
    RGBA8_UNORM,
    BGRA8_UNORM,
    RGBA16_FLOAT,
    Depth24Stencil8,
};

enum class TextureUsage : std::uint8_t {
    Sampled = 1u << 0u,
    RenderTarget = 1u << 1u,
    DepthStencil = 1u << 2u,
    Transfer = 1u << 3u,
};

struct BufferDesc {
    std::size_t size = 0;
    BufferUsage usage = BufferUsage::Vertex;
    bool cpu_visible = false;
};

enum class VertexFormat : std::uint8_t {
    Float1,
    Float2,
    Float3,
    Float4,
};

struct VertexAttribute {
    std::uint32_t location = 0;
    std::uint32_t binding = 0;
    std::uint32_t offset = 0;
    VertexFormat format = VertexFormat::Float3;
};

struct ShaderStageDesc {
    const std::uint32_t* code = nullptr;
    std::size_t word_count = 0;
};

struct GraphicsPipelineDesc {
    ShaderStageDesc vertex_shader{};
    ShaderStageDesc fragment_shader{};
    std::uint32_t vertex_stride = 0;
    std::vector<VertexAttribute> vertex_attributes;
};

struct TextureResourceDesc {
    std::uint32_t width = 1;
    std::uint32_t height = 1;
    std::uint32_t mip_levels = 1;
    TextureFormat format = TextureFormat::RGBA8_UNORM;
    std::uint32_t usage = static_cast<std::uint32_t>(TextureUsage::Sampled);
};

class Buffer {
public:
    virtual ~Buffer() = default;
    virtual std::size_t size() const noexcept = 0;
    virtual bool write(std::size_t offset, const void* data, std::size_t size) = 0;
};

class Pipeline {
public:
    virtual ~Pipeline() = default;
};

class Swapchain;

struct RenderPassDesc {
    Swapchain* target = nullptr;
    float clear_color[4] = {0.05f, 0.05f, 0.05f, 1.0f};
};

class CommandBuffer {
public:
    virtual ~CommandBuffer() = default;
    virtual void begin() = 0;
    virtual void end() = 0;
    virtual void begin_render_pass() = 0;
    virtual void begin_render_pass(const RenderPassDesc& desc) { (void)desc; begin_render_pass(); }
    virtual void end_render_pass() = 0;
    virtual void set_pipeline(Pipeline* pipeline) = 0;
    virtual void set_vertex_buffer(Buffer* buffer, std::uint32_t slot = 0) = 0;
    virtual void set_index_buffer(Buffer* buffer) { (void)buffer; }
    virtual void draw(std::uint32_t vertex_count, std::uint32_t first_vertex = 0) = 0;
    virtual void draw_indexed(std::uint32_t index_count, std::uint32_t first_index = 0,
                              std::int32_t vertex_offset = 0) {
        (void)index_count;
        (void)first_index;
        (void)vertex_offset;
    }
};

class Swapchain {
public:
    virtual ~Swapchain() = default;
    virtual std::uint32_t width() const noexcept = 0;
    virtual std::uint32_t height() const noexcept = 0;
    virtual bool acquire() = 0;
    virtual bool present() = 0;
};

} // namespace Nova::RHI
