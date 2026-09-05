#pragma once

#include <nova/runtime/Runtime.h>

namespace Nova::Editor {

// Public Editor boundary. Runtime never depends on this header.
class EditorBoundary final {
public:
    EditorBoundary() = default;
    ~EditorBoundary() = default;

    EditorBoundary(const EditorBoundary&) = delete;
    EditorBoundary& operator=(const EditorBoundary&) = delete;

    bool initialize(Runtime::Runtime& runtime) noexcept {
        runtime_ = &runtime;
        return runtime_->is_initialized();
    }

    void shutdown() noexcept { runtime_ = nullptr; }

    bool is_initialized() const noexcept { return runtime_ != nullptr; }
    Runtime::Runtime* runtime() noexcept { return runtime_; }
    const Runtime::Runtime* runtime() const noexcept { return runtime_; }

private:
    Runtime::Runtime* runtime_ = nullptr;
};

} // namespace Nova::Editor
