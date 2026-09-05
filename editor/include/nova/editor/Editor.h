#pragma once

namespace Nova::Editor {

// Editor-facing shell. The implementation stays in editor/; Runtime never
// includes this header or links against editor code.
class Editor final {
public:
    Editor() = default;
    ~Editor() = default;

    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;

    bool initialize();
    void shutdown();
    void tick(float delta_seconds);
};

} // namespace Nova::Editor
