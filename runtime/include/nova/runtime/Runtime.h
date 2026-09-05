#pragma once

#include <cstdint>
#include <nova/rhi/RHI.h>

namespace Nova::Runtime {

// Public runtime-facing entry point. The editor and game projects should
// include this header instead of reaching into editor implementation details.
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

private:
    std::unique_ptr<RHI::Device> device_;
};

} // namespace Nova::Runtime
