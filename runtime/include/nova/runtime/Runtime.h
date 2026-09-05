#pragma once

#include <memory>

#include <nova/rhi/Capabilities.h>
#include <nova/rhi/RHI.h>

namespace Nova::Runtime {

// Public runtime-facing entry point. Game code and the editor consume this
// API without depending on editor implementation details.
class Runtime final {
public:
    Runtime() = default;
    ~Runtime() = default;

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    bool initialize(const RHI::DeviceDesc& device_desc = {});
    void shutdown();

    bool is_initialized() const noexcept { return device_ != nullptr; }
    RHI::Device* device() noexcept { return device_.get(); }
    const RHI::Device* device() const noexcept { return device_.get(); }

    RHI::Capabilities capabilities() const noexcept {
        if (!device_) {
            return {};
        }
        return RHI::query_capabilities(*device_);
    }

private:
    std::unique_ptr<RHI::Device> device_;
};

} // namespace Nova::Runtime
