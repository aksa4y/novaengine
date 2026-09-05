#include <nova/runtime/Runtime.h>

namespace Nova::Runtime {

bool Runtime::initialize(const RHI::DeviceDesc& device_desc) {
    shutdown();
    device_ = RHI::create_device(device_desc);
    return device_ != nullptr;
}

void Runtime::shutdown() {
    device_.reset();
}

} // namespace Nova::Runtime
