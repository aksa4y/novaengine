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

std::uint32_t find_graphics_queue_family(VkPhysicalDevice physical_device) {
    std::uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
    if (count == 0) return invalid_index;
    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, families.data());
    for (std::uint32_t i = 0; i < count; ++i) {
        if (families[i].queueCount > 0 && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) return i;
    }
    return invalid_index;
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
    return invalid_index;
}

VkFormat to_vk_format(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::BGRA8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::RGBA16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::Depth24Stencil8: return VK_FORMAT_D24_UNORM_S8_UINT;
    }
    return VK_FORMAT_R8G8B8A8_UNORM;
}

bool make_surface(VkInstance instance, const NativeWindowHandle& handle, VkSurfaceKHR& surface) {
    switch (handle.type) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        case NativeWindowType::Win32: {
            if (!handle.window || !handle.display) return false;
            VkWin32SurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
            info.hinstance = reinterpret_cast<HINSTANCE>(handle.display);
            info.hwnd = reinterpret_cast<HWND>(handle.window);
            return vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface) == VK_SUCCESS;
        }
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        case NativeWindowType::X11: {
            if (!handle.window || !handle.display) return false;
            VkXlibSurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR};
            info.dpy = reinterpret_cast<Display*>(handle.display);
            info.window = static_cast< ::Window >(handle.window);
            return vkCreateXlibSurfaceKHR(instance, &info, nullptr, &surface) == VK_SUCCESS;
        }
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        case NativeWindowType::Wayland: {
            if (!handle.window || !handle.display) return false;
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
            auto create_metal_surface = reinterpret_cast<PFN_vkCreateMetalSurfaceEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateMetalSurfaceEXT"));
            return create_metal_surface != nullptr &&
                   create_metal_surface(instance, &info, nullptr, &surface) == VK_SUCCESS;
        }
#endif
        default:
            return false;
    }
}

void append_surface_extensions(std::vector<const char*>& extensions, NativeWindowType type) {
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    switch (type) {
        case NativeWindowType::Win32:
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
            break;
        case NativeWindowType::X11:
#if defined(VK_USE_PLATFORM_XLIB_KHR)
            extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
            break;
        case NativeWindowType::Wayland:
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
            extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
            break;
        case NativeWindowType::Android:
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
            extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
            break;
        case NativeWindowType::Cocoa:
        case NativeWindowType::UIKit:
#if defined(VK_USE_PLATFORM_METAL_EXT)
            extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#endif
            break;
        default:
            break;
    }
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
                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(device_, &info, nullptr, &buffer_) != VK_SUCCESS) return;
        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(device_, buffer_, &requirements);
        const VkMemoryPropertyFlags flags = desc.cpu_visible
            ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        const std::uint32_t type = find_memory_type(physical_device, requirements.memoryTypeBits, flags);
        if (type == invalid_index) { destroy(); return; }
        VkMemoryAllocateInfo allocation{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocation.allocationSize = requirements.size;
        allocation.memoryTypeIndex = type;
        if (vkAllocateMemory(device_, &allocation, nullptr, &memory_) != VK_SUCCESS) { destroy(); return; }
        if (vkBindBufferMemory(device_, buffer_, memory_, 0) != VK_SUCCESS) { destroy(); return; }
    }
    ~VulkanBuffer() override { destroy(); }
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
    VkBuffer handle() const noexcept { return buffer_; }
private:
    void destroy() {
        if (memory_ != VK_NULL_HANDLE) vkFreeMemory(device_, memory_, nullptr);
        if (buffer_ != VK_NULL_HANDLE) vkDestroyBuffer(device_, buffer_, nullptr);
        memory_ = VK_NULL_HANDLE;
        buffer_ = VK_NULL_HANDLE;
    }
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
        if (vkAllocateCommandBuffers(device_, &info, &command_buffer_) != VK_SUCCESS) command_buffer_ = VK_NULL_HANDLE;
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
        if (recording_) recording_ = vkEndCommandBuffer(command_buffer_) != VK_SUCCESS;
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
    VulkanSwapchain(
        VkDevice device,
        VkPhysicalDevice physical_device,
        VkQueue present_queue,
        const SwapchainDesc& desc,
        VkSurfaceKHR surface,
        VkSwapchainKHR swapchain,
        std::vector<VkImage> images,
        std::vector<VkImageView> views,
        VkSemaphore image_available)
        : device_(device), physical_device_(physical_device), present_queue_(present_queue),
          width_(desc.width), height_(desc.height), surface_(surface), swapchain_(swapchain),
          images_(std::move(images)), views_(std::move(views)), image_available_(image_available) {}

    ~VulkanSwapchain() override {
        if (image_available_ != VK_NULL_HANDLE) vkDestroySemaphore(device_, image_available_, nullptr);
        for (VkImageView view : views_) vkDestroyImageView(device_, view, nullptr);
        if (swapchain_ != VK_NULL_HANDLE) vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        if (surface_ != VK_NULL_HANDLE) vkDestroySurfaceKHR(instance_, surface_, nullptr);
    }

    void set_instance(VkInstance instance) noexcept { instance_ = instance; }
    std::uint32_t width() const noexcept override { return width_; }
    std::uint32_t height() const noexcept override { return height_; }

    bool acquire() override {
        if (swapchain_ == VK_NULL_HANDLE) return false;
        const VkResult result = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, image_available_, VK_NULL_HANDLE, &image_index_);
        acquired_ = result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR;
        return acquired_;
    }

    bool present() override {
        if (!acquired_ || swapchain_ == VK_NULL_HANDLE) return false;
        VkPresentInfoKHR info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_available_;
        info.swapchainCount = 1;
        info.pSwapchains = &swapchain_;
        info.pImageIndices = &image_index_;
        const VkResult result = vkQueuePresentKHR(present_queue_, &info);
        acquired_ = false;
        return result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR;
    }

private:
    VkDevice device_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;
    VkInstance instance_ = VK_NULL_HANDLE;
    std::uint32_t width_ = 1;
    std::uint32_t height_ = 1;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    std::vector<VkImage> images_;
    std::vector<VkImageView> views_;
    VkSemaphore image_available_ = VK_NULL_HANDLE;
    std::uint32_t image_index_ = 0;
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

        VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        info.pApplicationInfo = &app;
        if (vkCreateInstance(&info, nullptr, &instance_) != VK_SUCCESS) return;

        std::uint32_t device_count = 0;
        if (vkEnumeratePhysicalDevices(instance_, &device_count, nullptr) != VK_SUCCESS || device_count == 0) return;
        std::vector<VkPhysicalDevice> devices(device_count);
        if (vkEnumeratePhysicalDevices(instance_, &device_count, devices.data()) != VK_SUCCESS) return;
        for (VkPhysicalDevice candidate : devices) {
            const std::uint32_t family = find_graphics_queue_family(candidate);
            if (family != invalid_index) { physical_device_ = candidate; graphics_queue_family_ = family; break; }
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
    std::unique_ptr<Swapchain> create_swapchain(const SwapchainDesc& desc) override {
        if (!valid() || desc.width == 0 || desc.height == 0 || desc.window.type == NativeWindowType::None) return nullptr;

        std::vector<const char*> extensions;
        append_surface_extensions(extensions, desc.window.type);
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        if (!make_surface(instance_, desc.window, surface)) return nullptr;

        VkBool32 present_supported = VK_FALSE;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, graphics_queue_family_, surface, &present_supported) != VK_SUCCESS || !present_supported) {
            vkDestroySurfaceKHR(instance_, surface, nullptr);
            return nullptr;
        }

        std::uint32_t format_count = 0;
        std::uint32_t mode_count = 0;
        VkSurfaceCapabilitiesKHR capabilities{};
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface, &capabilities) != VK_SUCCESS ||
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface, &format_count, nullptr) != VK_SUCCESS ||
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface, &mode_count, nullptr) != VK_SUCCESS ||
            format_count == 0 || mode_count == 0) {
            vkDestroySurfaceKHR(instance_, surface, nullptr);
            return nullptr;
        }
        std::vector<VkSurfaceFormatKHR> formats(format_count);
        std::vector<VkPresentModeKHR> modes(mode_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface, &format_count, formats.data());
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface, &mode_count, modes.data());

        VkSurfaceFormatKHR chosen = formats.front();
        const VkFormat requested = to_vk_format(desc.format);
        for (const auto& format : formats) {
            if (format.format == requested && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { chosen = format; break; }
        }
        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
        for (VkPresentModeKHR mode : modes) if (mode == VK_PRESENT_MODE_MAILBOX_KHR) { present_mode = mode; break; }

        VkExtent2D extent{};
        if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
            extent = capabilities.currentExtent;
        } else {
            extent.width = std::clamp(desc.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(desc.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }
        std::uint32_t image_count = std::max(desc.image_count, capabilities.minImageCount);
        if (capabilities.maxImageCount != 0) image_count = std::min(image_count, capabilities.maxImageCount);

        VkSwapchainCreateInfoKHR swap_info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        swap_info.surface = surface;
        swap_info.minImageCount = image_count;
        swap_info.imageFormat = chosen.format;
        swap_info.imageColorSpace = chosen.colorSpace;
        swap_info.imageExtent = extent;
        swap_info.imageArrayLayers = 1;
        swap_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swap_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swap_info.preTransform = capabilities.currentTransform;
        swap_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swap_info.presentMode = present_mode;
        swap_info.clipped = VK_TRUE;

        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        if (vkCreateSwapchainKHR(device_, &swap_info, nullptr, &swapchain) != VK_SUCCESS) {
            vkDestroySurfaceKHR(instance_, surface, nullptr);
            return nullptr;
        }

        std::uint32_t image_count_actual = 0;
        vkGetSwapchainImagesKHR(device_, swapchain, &image_count_actual, nullptr);
        std::vector<VkImage> images(image_count_actual);
        vkGetSwapchainImagesKHR(device_, swapchain, &image_count_actual, images.data());
        std::vector<VkImageView> views;
        views.reserve(images.size());
        for (VkImage image : images) {
            VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            view_info.image = image;
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = chosen.format;
            view_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;
            VkImageView view = VK_NULL_HANDLE;
            if (vkCreateImageView(device_, &view_info, nullptr, &view) != VK_SUCCESS) {
                for (VkImageView created : views) vkDestroyImageView(device_, created, nullptr);
                vkDestroySwapchainKHR(device_, swapchain, nullptr);
                vkDestroySurfaceKHR(instance_, surface, nullptr);
                return nullptr;
            }
            views.push_back(view);
        }

        VkSemaphoreCreateInfo semaphore_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VkSemaphore image_available = VK_NULL_HANDLE;
        if (vkCreateSemaphore(device_, &semaphore_info, nullptr, &image_available) != VK_SUCCESS) {
            for (VkImageView view : views) vkDestroyImageView(device_, view, nullptr);
            vkDestroySwapchainKHR(device_, swapchain, nullptr);
            vkDestroySurfaceKHR(instance_, surface, nullptr);
            return nullptr;
        }

        auto result = std::make_unique<VulkanSwapchain>(device_, physical_device_, graphics_queue_, desc,
                                                         surface, swapchain, std::move(images), std::move(views), image_available);
        result->set_instance(instance_);
        return result;
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
    std::uint32_t graphics_queue_family_ = invalid_index;
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
