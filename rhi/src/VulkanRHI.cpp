#include "nova/rhi/BackendFactory.h"

#if defined(NOVA_RHI_NATIVE_VULKAN) && __has_include(<vulkan/vulkan.h>)
#include <vulkan/vulkan.h>

#include <cstring>
#include <limits>
#include <vector>

namespace Nova::RHI {
namespace {

std::uint32_t find_graphics_queue_family(VkPhysicalDevice physical_device) {
    std::uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
    if (count == 0) return std::numeric_limits<std::uint32_t>::max();
    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, families.data());
    for (std::uint32_t i = 0; i < count; ++i) {
        if (families[i].queueCount > 0 && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) return i;
    }
    return std::numeric_limits<std::uint32_t>::max();
}

std::uint32_t find_memory_type(
    VkPhysicalDevice physical_device,
    std::uint32_t type_bits,
    VkMemoryPropertyFlags required_flags) {
    VkPhysicalDeviceMemoryProperties properties{};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);
    for (std::uint32_t i = 0; i < properties.memoryTypeCount; ++i) {
        if ((type_bits & (1u << i)) == 0) continue;
        if ((properties.memoryTypes[i].propertyFlags & required_flags) == required_flags) return i;
    }
    return std::numeric_limits<std::uint32_t>::max();
}

class VulkanBuffer final : public Buffer {
public:
    VulkanBuffer(VkDevice device, VkPhysicalDevice physical_device, const BufferDesc& desc)
        : device_(device), size_(desc.size) {
        if (size_ == 0) return;
        VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        info.size = size_;
        info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(device_, &info, nullptr, &buffer_) != VK_SUCCESS) return;

        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(device_, buffer_, &requirements);
        const VkMemoryPropertyFlags flags = desc.cpu_visible
            ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        const std::uint32_t memory_type = find_memory_type(
            physical_device, requirements.memoryTypeBits, flags);
        if (memory_type == std::numeric_limits<std::uint32_t>::max()) {
            vkDestroyBuffer(device_, buffer_, nullptr);
            buffer_ = VK_NULL_HANDLE;
            return;
        }

        VkMemoryAllocateInfo allocation{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocation.allocationSize = requirements.size;
        allocation.memoryTypeIndex = memory_type;
        if (vkAllocateMemory(device_, &allocation, nullptr, &memory_) != VK_SUCCESS) {
            vkDestroyBuffer(device_, buffer_, nullptr);
            buffer_ = VK_NULL_HANDLE;
            return;
        }
        if (vkBindBufferMemory(device_, buffer_, memory_, 0) != VK_SUCCESS) {
            vkFreeMemory(device_, memory_, nullptr);
            vkDestroyBuffer(device_, buffer_, nullptr);
            memory_ = VK_NULL_HANDLE;
            buffer_ = VK_NULL_HANDLE;
        }
    }

    ~VulkanBuffer() override {
        if (memory_ != VK_NULL_HANDLE) vkFreeMemory(device_, memory_, nullptr);
        if (buffer_ != VK_NULL_HANDLE) vkDestroyBuffer(device_, buffer_, nullptr);
    }

    bool valid() const noexcept { return buffer_ != VK_NULL_HANDLE && memory_ != VK_NULL_HANDLE; }
    std::size_t size() const noexcept override { return size_; }

    bool write(std::size_t offset, const void* data, std::size_t size) override {
        if (!valid() || !data || offset > size_ || size > size_ - offset) return false;
        void* mapped = nullptr;
        if (vkMapMemory(device_, memory_, offset, size, 0, &mapped) != VK_SUCCESS) return false;
        std::memcpy(mapped, data, size);
        vkUnmapMemory(device_, memory_);
        return true;
    }

private:
    VkDevice device_ = VK_NULL_HANDLE;
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory memory_ = VK_NULL_HANDLE;
    std::size_t size_ = 0;
};

class VulkanTexture final : public Texture {};
class VulkanPipeline final : public Pipeline {};

class VulkanCommandBuffer final : public CommandBuffer {
public:
    VulkanCommandBuffer(VkDevice device, VkCommandPool pool) : device_(device), pool_(pool) {
        VkCommandBufferAllocateInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        info.commandPool = pool_;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = 1;
        if (vkAllocateCommandBuffers(device_, &info, &command_buffer_) != VK_SUCCESS) {
            command_buffer_ = VK_NULL_HANDLE;
        }
    }

    ~VulkanCommandBuffer() override {
        if (command_buffer_ != VK_NULL_HANDLE) vkFreeCommandBuffers(device_, pool_, 1, &command_buffer_);
    }

    bool valid() const noexcept { return command_buffer_ != VK_NULL_HANDLE; }
    void begin() override {
        if (!valid()) return;
        vkResetCommandBuffer(command_buffer_, 0);
        VkCommandBufferBeginInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        recording_ = vkBeginCommandBuffer(command_buffer_, &info) == VK_SUCCESS;
    }
    void end() override {
        if (!recording_) return;
        recording_ = vkEndCommandBuffer(command_buffer_) != VK_SUCCESS;
    }
    void begin_render_pass() override { render_pass_ = recording_; }
    void end_render_pass() override { render_pass_ = false; }
    void set_pipeline(Pipeline*) override {}
    void set_vertex_buffer(Buffer*, std::uint32_t) override {}
    void draw(std::uint32_t, std::uint32_t) override {}

private:
    VkDevice device_ = VK_NULL_HANDLE;
    VkCommandPool pool_ = VK_NULL_HANDLE;
    VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;
    bool recording_ = false;
    bool render_pass_ = false;
};

class VulkanSwapchain final : public Swapchain {
public:
    VulkanSwapchain(std::uint32_t width, std::uint32_t height) : width_(width), height_(height) {}
    std::uint32_t width() const noexcept override { return width_; }
    std::uint32_t height() const noexcept override { return height_; }
    bool acquire() override { acquired_ = true; return true; }
    bool present() override {
        const bool result = acquired_;
        acquired_ = false;
        return result;
    }
private:
    std::uint32_t width_ = 1;
    std::uint32_t height_ = 1;
    bool acquired_ = false;
};

class VulkanDevice final : public Device {
public:
    explicit VulkanDevice(const DeviceDesc& desc) : debug_(desc.debug) {
        VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        app.pApplicationName = "Nova Engine";
        app.applicationVersion = 1;
        app.pEngineName = "Nova";
        app.engineVersion = 1;
        app.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instance_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        instance_info.pApplicationInfo = &app;
        if (vkCreateInstance(&instance_info, nullptr, &instance_) != VK_SUCCESS) return;

        std::uint32_t device_count = 0;
        if (vkEnumeratePhysicalDevices(instance_, &device_count, nullptr) != VK_SUCCESS || device_count == 0) return;
        std::vector<VkPhysicalDevice> devices(device_count);
        if (vkEnumeratePhysicalDevices(instance_, &device_count, devices.data()) != VK_SUCCESS) return;

        for (VkPhysicalDevice candidate : devices) {
            const std::uint32_t family = find_graphics_queue_family(candidate);
            if (family != std::numeric_limits<std::uint32_t>::max()) {
                physical_device_ = candidate;
                graphics_queue_family_ = family;
                break;
            }
        }
        if (physical_device_ == VK_NULL_HANDLE) return;

        const float priority = 1.0f;
        VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queue_info.queueFamilyIndex = graphics_queue_family_;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &priority;

        VkDeviceCreateInfo device_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        device_info.queueCreateInfoCount = 1;
        device_info.pQueueCreateInfos = &queue_info;
        if (vkCreateDevice(physical_device_, &device_info, nullptr, &device_) != VK_SUCCESS) return;
        vkGetDeviceQueue(device_, graphics_queue_family_, 0, &graphics_queue_);

        VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = graphics_queue_family_;
        if (vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_) != VK_SUCCESS) {
            vkDestroyDevice(device_, nullptr);
            device_ = VK_NULL_HANDLE;
        }
    }

    ~VulkanDevice() override {
        if (device_ != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(device_);
            if (command_pool_ != VK_NULL_HANDLE) vkDestroyCommandPool(device_, command_pool_, nullptr);
            vkDestroyDevice(device_, nullptr);
        }
        if (instance_ != VK_NULL_HANDLE) vkDestroyInstance(instance_, nullptr);
    }

    bool valid() const noexcept {
        return instance_ != VK_NULL_HANDLE && physical_device_ != VK_NULL_HANDLE &&
               device_ != VK_NULL_HANDLE && command_pool_ != VK_NULL_HANDLE;
    }

    Backend backend() const noexcept override { return Backend::Vulkan; }
    std::string_view backend_name() const noexcept override { return "Vulkan"; }

    std::unique_ptr<Buffer> create_buffer(const BufferDesc& desc) override {
        auto buffer = std::make_unique<VulkanBuffer>(device_, physical_device_, desc);
        return buffer->valid() ? std::move(buffer) : nullptr;
    }
    std::unique_ptr<Texture> create_texture(const TextureDesc& desc) override {
        if (desc.width == 0 || desc.height == 0 || desc.mip_levels == 0) return nullptr;
        return std::make_unique<VulkanTexture>();
    }
    std::unique_ptr<CommandBuffer> create_command_buffer() override {
        auto command_buffer = std::make_unique<VulkanCommandBuffer>(device_, command_pool_);
        return command_buffer->valid() ? std::move(command_buffer) : nullptr;
    }
    std::unique_ptr<Pipeline> create_pipeline() override { return std::make_unique<VulkanPipeline>(); }
    std::unique_ptr<Swapchain> create_swapchain(std::uint32_t width, std::uint32_t height) override {
        // Placeholder until RHI exposes an explicit platform-neutral native
        // window/surface descriptor. No fake VkSwapchainKHR is created here.
        return (width && height) ? std::make_unique<VulkanSwapchain>(width, height) : nullptr;
    }
    void begin_frame() override {}
    void end_frame() override {}

private:
    bool debug_ = false;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkCommandPool command_pool_ = VK_NULL_HANDLE;
    std::uint32_t graphics_queue_family_ = std::numeric_limits<std::uint32_t>::max();
};

} // namespace

std::unique_ptr<Device> create_vulkan_device(const DeviceDesc& desc) {
    auto device = std::make_unique<VulkanDevice>(desc);
    return device->valid() ? std::move(device) : nullptr;
}

} // namespace Nova::RHI
#else
namespace Nova::RHI {
std::unique_ptr<Device> create_vulkan_device(const DeviceDesc&) { return nullptr; }
}
#endif
