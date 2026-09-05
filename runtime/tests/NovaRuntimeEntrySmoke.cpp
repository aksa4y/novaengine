#include <nova/runtime/NovaEngine.h>

int main() {
    Nova::Engine::RuntimeInstance runtime;
    if (Nova::Engine::initialized(runtime)) {
        return 1;
    }

    if (!Nova::Engine::initialize(runtime, {Nova::RHI::Backend::Null, false})) {
        return 2;
    }

    if (!Nova::Engine::initialized(runtime)) {
        return 3;
    }

    Nova::Engine::shutdown(runtime);
    return Nova::Engine::initialized(runtime) ? 4 : 0;
}
