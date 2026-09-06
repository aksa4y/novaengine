#include "nova/rhi/BackendFactory.h"

#if defined(NOVA_RHI_NATIVE_VULKAN) && __has_include(<vulkan/vulkan.h>)

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#  if __has_include(<X11/Xlib.h>)
#    define VK_USE_PLATFORM_XLIB_KHR
#  endif
#  if __has_include(<wayland-client.h>)
#    define VK_USE_PLATFORM_WAYLAND_KHR
#  endif
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_METAL_EXT
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include <X11/Xlib.h>
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include <wayland-client.h>
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/native_window.h>
#endif
#include <vulkan/vulkan.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <vector>

namespace Nova::RHI {
namespace {
constexpr std::uint32_t invalid_index = std::numeric_limits<std::uint32_t>::max();

bool instance_extension_supported(const char* name) {
    std::uint32_t count = 0;
    if (vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr) != VK_SUCCESS) return false;
    std::vector<VkExtensionProperties> props(count);
    if (vkEnumerateInstanceExtensionProperties(nullptr, &count, props.data()) != VK_SUCCESS) return false;
    for (const auto& prop : props) if (std::strcmp(prop.extensionName, name) == 0) return true;
    return false;
}

bool device_extension_supported(VkPhysicalDevice device, const char* name) {
    std::uint32_t count = 0;
    if (vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr) != VK_SUCCESS) return false;
    std::vector<VkExtensionProperties> props(count);
    if (vkEnumerateDeviceExtensionProperties(device, nullptr, &count, props.data()) != VK_SUCCESS) return false;
    for (const auto& prop : props) if (std::strcmp(prop.extensionName, name) == 0) return true;
    return false;
}

std::uint32_t graphics_queue_family(VkPhysicalDevice physical) {
    std::uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, nullptr);
    if (!count) return invalid_index;
    std::vector<VkQueueFamilyProperties> props(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, props.data());
    for (std::uint32_t i = 0; i < count; ++i)
        if (props[i].queueCount && (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) return i;
    return invalid_index;
}

std::uint32_t memory_type(VkPhysicalDevice physical, std::uint32_t bits, VkMemoryPropertyFlags flags) {
    VkPhysicalDeviceMemoryProperties props{};
    vkGetPhysicalDeviceMemoryProperties(physical, &props);
    for (std::uint32_t i = 0; i < props.memoryTypeCount; ++i)
        if ((bits & (1u << i)) && (props.memoryTypes[i].propertyFlags & flags) == flags) return i;
    return invalid_index;
}

VkFormat vk_format(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::BGRA8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::RGBA16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::Depth24Stencil8: return VK_FORMAT_D24_UNORM_S8_UINT;
    }
    return VK_FORMAT_R8G8B8A8_UNORM;
}

std::size_t pixel_size(TextureFormat format) {
    return format == TextureFormat::RGBA16_FLOAT ? 8u : 4u;
}

bool create_surface(VkInstance instance, const NativeWindowHandle& handle, VkSurfaceKHR& surface) {
    switch (handle.type) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        case NativeWindowType::Win32: {
            if (!handle.display || !handle.window) return false;
            VkWin32SurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
            info.hinstance = reinterpret_cast<HINSTANCE>(handle.display);
            info.hwnd = reinterpret_cast<HWND>(handle.window);
            return vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface) == VK_SUCCESS;
        }
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        case NativeWindowType::X11: {
            if (!handle.display || !handle.window) return false;
            VkXlibSurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR};
            info.dpy = reinterpret_cast<Display*>(handle.display);
            info.window = static_cast<::Window>(handle.window);
            return vkCreateXlibSurfaceKHR(instance, &info, nullptr, &surface) == VK_SUCCESS;
        }
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        case NativeWindowType::Wayland: {
            if (!handle.display || !handle.window) return false;
            VkWaylandSurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR};
            info.display = reinterpret_cast<wl_display*>(handle.display);
            info.surface = reinterpret_cast<wl_surface*>(handle.window);
            return vkCreateWaylandSurfaceKHR(instance, &info, nullptr, &surface) == VK_SUCCESS;
        }
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        case NativeWindowType::Android: {
            if (!handle.window) return false;
            VkAndroidSurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
            info.window = reinterpret_cast<ANativeWindow*>(handle.window);
            return vkCreateAndroidSurfaceKHR(instance, &info, nullptr, &surface) == VK_SUCCESS;
        }
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
        case NativeWindowType::Cocoa:
        case NativeWindowType::UIKit: {
            if (!handle.window) return false;
            VkMetalSurfaceCreateInfoEXT info{VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT};
            info.pLayer = reinterpret_cast<const void*>(handle.window);
            const auto fn = reinterpret_cast<PFN_vkCreateMetalSurfaceEXT>(vkGetInstanceProcAddr(instance, "vkCreateMetalSurfaceEXT"));
            return fn && fn(instance, &info, nullptr, &surface) == VK_SUCCESS;
        }
#endif
        default: return false;
    }
}

class VulkanBuffer final : public Buffer {
public:
    VulkanBuffer(VkDevice device, VkPhysicalDevice physical, const BufferDesc& desc) : device_(device), size_(desc.size) {
        if (!size_) return;
        VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        info.size = size_;
        info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(device_, &info, nullptr, &buffer_) != VK_SUCCESS) return;
        VkMemoryRequirements req{};
        vkGetBufferMemoryRequirements(device_, buffer_, &req);
        const auto flags = desc.cpu_visible
            ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        const auto type = memory_type(physical, req.memoryTypeBits, flags);
        if (type == invalid_index) { destroy(); return; }
        VkMemoryAllocateInfo alloc{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        alloc.allocationSize = req.size;
        alloc.memoryTypeIndex = type;
        if (vkAllocateMemory(device_, &alloc, nullptr, &memory_) != VK_SUCCESS || vkBindBufferMemory(device_, buffer_, memory_, 0) != VK_SUCCESS) destroy();
    }
    ~VulkanBuffer() override { destroy(); }
    bool valid() const noexcept { return buffer_ && memory_; }
    std::size_t size() const noexcept override { return size_; }
    bool write(std::size_t offset, const void* data, std::size_t size) override {
        if (!valid() || !data || offset > size_ || size > size_ - offset) return false;
        void* mapped = nullptr;
        if (vkMapMemory(device_, memory_, offset, size, 0, &mapped) != VK_SUCCESS) return false;
        std::memcpy(mapped, data, size);
        vkUnmapMemory(device_, memory_);
        return true;
    }
    VkBuffer handle() const noexcept { return buffer_; }
private:
    void destroy() {
        if (memory_) vkFreeMemory(device_, memory_, nullptr);
        if (buffer_) vkDestroyBuffer(device_, buffer_, nullptr);
        memory_ = VK_NULL_HANDLE;
        buffer_ = VK_NULL_HANDLE;
    }
    VkDevice device_ = VK_NULL_HANDLE;
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory memory_ = VK_NULL_HANDLE;
    std::size_t size_ = 0;
};

class VulkanTexture final : public Texture {
public:
    VulkanTexture(VkDevice device, VkPhysicalDevice physical, VkQueue queue, VkCommandPool pool, const TextureDesc& desc)
        : device_(device), physical_(physical), queue_(queue), pool_(pool), desc_(desc) {
        if (!desc_.width || !desc_.height || !desc_.mip_levels) return;
        VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = vk_format(desc_.format);
        info.extent = {desc_.width, desc_.height, 1};
        info.mipLevels = desc_.mip_levels;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (desc_.usage & static_cast<std::uint32_t>(TextureUsage::Sampled)) info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (desc_.usage & static_cast<std::uint32_t>(TextureUsage::RenderTarget)) info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (desc_.usage & static_cast<std::uint32_t>(TextureUsage::DepthStencil)) info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (vkCreateImage(device_, &info, nullptr, &image_) != VK_SUCCESS) return;
        VkMemoryRequirements req{};
        vkGetImageMemoryRequirements(device_, image_, &req);
        const auto type = memory_type(physical_, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (type == invalid_index) { destroy(); return; }
        VkMemoryAllocateInfo alloc{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        alloc.allocationSize = req.size;
        alloc.memoryTypeIndex = type;
        if (vkAllocateMemory(device_, &alloc, nullptr, &memory_) != VK_SUCCESS || vkBindImageMemory(device_, image_, memory_, 0) != VK_SUCCESS) { destroy(); return; }
        VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view.image = image_;
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = info.format;
        view.subresourceRange.aspectMask = desc_.format == TextureFormat::Depth24Stencil8 ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.levelCount = 1;
        view.subresourceRange.layerCount = 1;
        if (vkCreateImageView(device_, &view, nullptr, &view_) != VK_SUCCESS) destroy();
    }
    ~VulkanTexture() override { destroy(); }
    bool valid() const noexcept { return image_ && memory_ && view_; }
    bool upload(const void* data, std::size_t size) override {
        if (!valid() || !data || desc_.mip_levels != 1) return false;
        if (size != static_cast<std::size_t>(desc_.width) * desc_.height * pixel_size(desc_.format)) return false;
        VulkanBuffer staging(device_, physical_, {size, BufferUsage::Transfer, true});
        if (!staging.valid() || !staging.write(0, data, size)) return false;
        VkCommandBufferAllocateInfo alloc{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        alloc.commandPool = pool_;
        alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc.commandBufferCount = 1;
        VkCommandBuffer cmd = VK_NULL_HANDLE;
        if (vkAllocateCommandBuffers(device_, &alloc, &cmd) != VK_SUCCESS) return false;
        VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        bool ok = vkBeginCommandBuffer(cmd, &begin) == VK_SUCCESS;
        const VkImageAspectFlags aspect = desc_.format == TextureFormat::Depth24Stencil8 ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        if (ok) {
            VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.image = image_;
            barrier.subresourceRange = {aspect, 0, 1, 0, 1};
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            VkBufferImageCopy region{};
            region.imageSubresource.aspectMask = aspect;
            region.imageSubresource.layerCount = 1;
            region.imageExtent = {desc_.width, desc_.height, 1};
            vkCmdCopyBufferToImage(cmd, staging.handle(), image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = desc_.usage & static_cast<std::uint32_t>(TextureUsage::Sampled) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = barrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ? VK_ACCESS_SHADER_READ_BIT : VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            const auto stage = barrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ? VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT : VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            ok = vkEndCommandBuffer(cmd) == VK_SUCCESS;
        }
        if (ok) {
            VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &cmd;
            ok = vkQueueSubmit(queue_, 1, &submit, VK_NULL_HANDLE) == VK_SUCCESS;
            if (ok) ok = vkQueueWaitIdle(queue_) == VK_SUCCESS;
        }
        vkFreeCommandBuffers(device_, pool_, 1, &cmd);
        return ok;
    }
private:
    void destroy() {
        if (view_) vkDestroyImageView(device_, view_, nullptr);
        if (memory_) vkFreeMemory(device_, memory_, nullptr);
        if (image_) vkDestroyImage(device_, image_, nullptr);
        view_ = VK_NULL_HANDLE; memory_ = VK_NULL_HANDLE; image_ = VK_NULL_HANDLE;
    }
    VkDevice device_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_ = VK_NULL_HANDLE;
    VkQueue queue_ = VK_NULL_HANDLE;
    VkCommandPool pool_ = VK_NULL_HANDLE;
    TextureDesc desc_{};
    VkImage image_ = VK_NULL_HANDLE;
    VkDeviceMemory memory_ = VK_NULL_HANDLE;
    VkImageView view_ = VK_NULL_HANDLE;
};

class VulkanCommandBuffer final : public CommandBuffer {
public:
    VulkanCommandBuffer(VkDevice device, VkCommandPool pool) : device_(device), pool_(pool) {
        VkCommandBufferAllocateInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        info.commandPool = pool_; info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; info.commandBufferCount = 1;
        if (vkAllocateCommandBuffers(device_, &info, &cmd_) != VK_SUCCESS) cmd_ = VK_NULL_HANDLE;
    }
    ~VulkanCommandBuffer() override { if (cmd_) vkFreeCommandBuffers(device_, pool_, 1, &cmd_); }
    bool valid() const noexcept { return cmd_ != VK_NULL_HANDLE; }
    void begin() override { if (!valid()) return; vkResetCommandBuffer(cmd_, 0); VkCommandBufferBeginInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; recording_ = vkBeginCommandBuffer(cmd_, &info) == VK_SUCCESS; }
    void end() override { if (recording_) recording_ = vkEndCommandBuffer(cmd_) == VK_SUCCESS; }
    void begin_render_pass() override { render_pass_ = recording_; }
    void end_render_pass() override { render_pass_ = false; }
    void set_pipeline(Pipeline*) override {}
    void set_vertex_buffer(Buffer*, std::uint32_t) override {}
    void draw(std::uint32_t, std::uint32_t) override {}
private:
    VkDevice device_ = VK_NULL_HANDLE; VkCommandPool pool_ = VK_NULL_HANDLE; VkCommandBuffer cmd_ = VK_NULL_HANDLE;
    bool recording_ = false; bool render_pass_ = false;
};

class VulkanPipeline final : public Pipeline {};

class VulkanSwapchain final : public Swapchain {
public:
    VulkanSwapchain(VkInstance instance, VkDevice device, VkSurfaceKHR surface, VkSwapchainKHR swapchain,
                    std::uint32_t width, std::uint32_t height, std::vector<VkImage> images,
                    std::vector<VkImageView> views, VkSemaphore available, VkQueue queue)
        : instance_(instance), device_(device), surface_(surface), swapchain_(swapchain), width_(width), height_(height),
          images_(std::move(images)), views_(std::move(views)), image_available_(available), queue_(queue) {}
    ~VulkanSwapchain() override {
        if (image_available_) vkDestroySemaphore(device_, image_available_, nullptr);
        for (auto view : views_) vkDestroyImageView(device_, view, nullptr);
        if (swapchain_) vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        if (surface_) vkDestroySurfaceKHR(instance_, surface_, nullptr);
    }
    std::uint32_t width() const noexcept override { return width_; }
    std::uint32_t height() const noexcept override { return height_; }
    bool acquire() override {
        if (!swapchain_) return false;
        const auto result = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, image_available_, VK_NULL_HANDLE, &image_index_);
        acquired_ = result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR;
        return acquired_;
    }
    bool present() override {
        if (!acquired_) return false;
        VkPresentInfoKHR info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        info.waitSemaphoreCount = 1; info.pWaitSemaphores = &image_available_;
        info.swapchainCount = 1; info.pSwapchains = &swapchain_; info.pImageIndices = &image_index_;
        const auto result = vkQueuePresentKHR(queue_, &info);
        acquired_ = false;
        return result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR;
    }
private:
    VkInstance instance_ = VK_NULL_HANDLE; VkDevice device_ = VK_NULL_HANDLE; VkSurfaceKHR surface_ = VK_NULL_HANDLE; VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    std::uint32_t width_ = 1, height_ = 1; std::vector<VkImage> images_; std::vector<VkImageView> views_;
    VkSemaphore image_available_ = VK_NULL_HANDLE; VkQueue queue_ = VK_NULL_HANDLE; std::uint32_t image_index_ = 0; bool acquired_ = false;
};

class VulkanDevice final : public Device {
public:
    explicit VulkanDevice(const DeviceDesc&) {
        VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        app.pApplicationName = "Nova Engine"; app.applicationVersion = 1; app.pEngineName = "Nova"; app.engineVersion = 1; app.apiVersion = VK_API_VERSION_1_0;
        std::vector<const char*> extensions;
        if (instance_extension_supported(VK_KHR_SURFACE_EXTENSION_NAME)) extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        if (instance_extension_supported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)) extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        if (instance_extension_supported(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)) extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        if (instance_extension_supported(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME)) extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        if (instance_extension_supported(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
        if (instance_extension_supported(VK_EXT_METAL_SURFACE_EXTENSION_NAME)) extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#endif
        VkInstanceCreateInfo instance_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        instance_info.pApplicationInfo = &app; instance_info.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()); instance_info.ppEnabledExtensionNames = extensions.data();
        if (vkCreateInstance(&instance_info, nullptr, &instance_) != VK_SUCCESS) return;
        std::uint32_t count = 0;
        if (vkEnumeratePhysicalDevices(instance_, &count, nullptr) != VK_SUCCESS || !count) return;
        std::vector<VkPhysicalDevice> devices(count); if (vkEnumeratePhysicalDevices(instance_, &count, devices.data()) != VK_SUCCESS) return;
        for (auto candidate : devices) {
            const auto family = graphics_queue_family(candidate);
            if (family != invalid_index) { physical_ = candidate; graphics_family_ = family; swapchain_supported_ = device_extension_supported(candidate, VK_KHR_SWAPCHAIN_EXTENSION_NAME); break; }
        }
        if (!physical_) return;
        const float priority = 1.0f;
        VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queue_info.queueFamilyIndex = graphics_family_; queue_info.queueCount = 1; queue_info.pQueuePriorities = &priority;
        const char* swapchain_ext = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        VkDeviceCreateInfo device_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        device_info.queueCreateInfoCount = 1; device_info.pQueueCreateInfos = &queue_info;
        if (swapchain_supported_) { device_info.enabledExtensionCount = 1; device_info.ppEnabledExtensionNames = &swapchain_ext; }
        if (vkCreateDevice(physical_, &device_info, nullptr, &device_) != VK_SUCCESS) return;
        vkGetDeviceQueue(device_, graphics_family_, 0, &queue_);
        VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; pool_info.queueFamilyIndex = graphics_family_;
        if (vkCreateCommandPool(device_, &pool_info, nullptr, &pool_) != VK_SUCCESS) { vkDestroyDevice(device_, nullptr); device_ = VK_NULL_HANDLE; }
    }
    ~VulkanDevice() override {
        if (device_) { vkDeviceWaitIdle(device_); if (pool_) vkDestroyCommandPool(device_, pool_, nullptr); vkDestroyDevice(device_, nullptr); }
        if (instance_) vkDestroyInstance(instance_, nullptr);
    }
    bool valid() const noexcept { return instance_ && physical_ && device_ && pool_; }
    Backend backend() const noexcept override { return Backend::Vulkan; }
    std::string_view backend_name() const noexcept override { return "Vulkan"; }
    std::unique_ptr<Buffer> create_buffer(const BufferDesc& desc) override { auto value = std::make_unique<VulkanBuffer>(device_, physical_, desc); return value->valid() ? std::move(value) : nullptr; }
    std::unique_ptr<Texture> create_texture(const TextureDesc& desc) override { auto value = std::make_unique<VulkanTexture>(device_, physical_, queue_, pool_, desc); return value->valid() ? std::move(value) : nullptr; }
    std::unique_ptr<CommandBuffer> create_command_buffer() override { auto value = std::make_unique<VulkanCommandBuffer>(device_, pool_); return value->valid() ? std::move(value) : nullptr; }
    std::unique_ptr<Pipeline> create_pipeline() override { return std::make_unique<VulkanPipeline>(); }
    std::unique_ptr<Swapchain> create_swapchain(const SwapchainDesc& desc) override {
        if (!valid() || !swapchain_supported_ || !desc.width || !desc.height) return nullptr;
        VkSurfaceKHR surface = VK_NULL_HANDLE; if (!create_surface(instance_, desc.window, surface)) return nullptr;
        VkBool32 present = VK_FALSE;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(physical_, graphics_family_, surface, &present) != VK_SUCCESS || !present) { vkDestroySurfaceKHR(instance_, surface, nullptr); return nullptr; }
        std::uint32_t format_count = 0, mode_count = 0; VkSurfaceCapabilitiesKHR caps{};
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_, surface, &caps) != VK_SUCCESS || vkGetPhysicalDeviceSurfaceFormatsKHR(physical_, surface, &format_count, nullptr) != VK_SUCCESS || vkGetPhysicalDeviceSurfacePresentModesKHR(physical_, surface, &mode_count, nullptr) != VK_SUCCESS || !format_count || !mode_count) { vkDestroySurfaceKHR(instance_, surface, nullptr); return nullptr; }
        std::vector<VkSurfaceFormatKHR> formats(format_count); std::vector<VkPresentModeKHR> modes(mode_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_, surface, &format_count, formats.data()); vkGetPhysicalDeviceSurfacePresentModesKHR(physical_, surface, &mode_count, modes.data());
        VkSurfaceFormatKHR format = formats.front(); const auto requested = vk_format(desc.format);
        for (const auto& candidate : formats) if (candidate.format == requested && candidate.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { format = candidate; break; }
        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR; for (const auto mode : modes) if (mode == VK_PRESENT_MODE_MAILBOX_KHR) { present_mode = mode; break; }
        VkExtent2D extent{};
        if (caps.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) extent = caps.currentExtent;
        else { extent.width = std::clamp(desc.width, caps.minImageExtent.width, caps.maxImageExtent.width); extent.height = std::clamp(desc.height, caps.minImageExtent.height, caps.maxImageExtent.height); }
        std::uint32_t image_count = std::max(desc.image_count, caps.minImageCount); if (caps.maxImageCount) image_count = std::min(image_count, caps.maxImageCount);
        VkSwapchainCreateInfoKHR info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        info.surface = surface; info.minImageCount = image_count; info.imageFormat = format.format; info.imageColorSpace = format.colorSpace; info.imageExtent = extent; info.imageArrayLayers = 1; info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; info.preTransform = caps.currentTransform; info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; info.presentMode = present_mode; info.clipped = VK_TRUE;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        if (vkCreateSwapchainKHR(device_, &info, nullptr, &swapchain) != VK_SUCCESS) { vkDestroySurfaceKHR(instance_, surface, nullptr); return nullptr; }
        std::uint32_t actual = 0; vkGetSwapchainImagesKHR(device_, swapchain, &actual, nullptr); if (!actual) { vkDestroySwapchainKHR(device_, swapchain, nullptr); vkDestroySurfaceKHR(instance_, surface, nullptr); return nullptr; }
        std::vector<VkImage> images(actual); vkGetSwapchainImagesKHR(device_, swapchain, &actual, images.data()); std::vector<VkImageView> views; views.reserve(actual);
        for (auto image : images) { VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO}; vi.image = image; vi.viewType = VK_IMAGE_VIEW_TYPE_2D; vi.format = format.format; vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; vi.subresourceRange.levelCount = 1; vi.subresourceRange.layerCount = 1; VkImageView view = VK_NULL_HANDLE; if (vkCreateImageView(device_, &vi, nullptr, &view) != VK_SUCCESS) { for (auto v : views) vkDestroyImageView(device_, v, nullptr); vkDestroySwapchainKHR(device_, swapchain, nullptr); vkDestroySurfaceKHR(instance_, surface, nullptr); return nullptr; } views.push_back(view); }
        VkSemaphoreCreateInfo semaphore{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO}; VkSemaphore available = VK_NULL_HANDLE; if (vkCreateSemaphore(device_, &semaphore, nullptr, &available) != VK_SUCCESS) { for (auto v : views) vkDestroyImageView(device_, v, nullptr); vkDestroySwapchainKHR(device_, swapchain, nullptr); vkDestroySurfaceKHR(instance_, surface, nullptr); return nullptr; }
        return std::make_unique<VulkanSwapchain>(instance_, device_, surface, swapchain, extent.width, extent.height, std::move(images), std::move(views), available, queue_);
    }
    void begin_frame() override {}
    void end_frame() override {}
private:
    bool swapchain_supported_ = false; VkInstance instance_ = VK_NULL_HANDLE; VkPhysicalDevice physical_ = VK_NULL_HANDLE; VkDevice device_ = VK_NULL_HANDLE; VkQueue queue_ = VK_NULL_HANDLE; VkCommandPool pool_ = VK_NULL_HANDLE; std::uint32_t graphics_family_ = invalid_index;
};

} // namespace
std::unique_ptr<Device> create_vulkan_device(const DeviceDesc& desc) { auto value = std::make_unique<VulkanDevice>(desc); return value->valid() ? std::move(value) : nullptr; }
} // namespace Nova::RHI
#else
namespace Nova::RHI { std::unique_ptr<Device> create_vulkan_device(const DeviceDesc&) { return nullptr; } }
#endif
