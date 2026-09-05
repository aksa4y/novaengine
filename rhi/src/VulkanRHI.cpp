#include "nova/rhi/BackendFactory.h"

#if defined(NOVA_RHI_NATIVE_VULKAN) && __has_include(<vulkan/vulkan.h>)
#include <vulkan/vulkan.h>

namespace Nova::RHI {
namespace {
class VulkanDevice final : public Device {
public:
    explicit VulkanDevice(bool debug) : debug_(debug) {
        VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        app.pApplicationName = "Nova Engine";
        app.applicationVersion = 1;
        app.pEngineName = "Nova";
        app.engineVersion = 1;
        app.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        info.pApplicationInfo = &app;
        if (vkCreateInstance(&info, nullptr, &instance_) != VK_SUCCESS) {
            instance_ = VK_NULL_HANDLE;
        }
    }

    ~VulkanDevice() override {
        if (instance_ != VK_NULL_HANDLE) {
            vkDestroyInstance(instance_, nullptr);
        }
    }

    bool valid() const noexcept { return instance_ != VK_NULL_HANDLE; }
    Backend backend() const noexcept override { return Backend::Vulkan; }
    std::string_view backend_name() const noexcept override { return "Vulkan"; }

    std::unique_ptr<Texture> create_texture(const TextureDesc&) override { return nullptr; }
    void begin_frame() override {}
    void end_frame() override {}

private:
    bool debug_ = false;
    VkInstance instance_ = VK_NULL_HANDLE;
};
}

std::unique_ptr<Device> create_vulkan_device(const DeviceDesc& desc) {
    auto device = std::make_unique<VulkanDevice>(desc.debug);
    return device->valid() ? std::move(device) : nullptr;
}
}
#else
namespace Nova::RHI {
std::unique_ptr<Device> create_vulkan_device(const DeviceDesc&) { return nullptr; }
}
#endif
