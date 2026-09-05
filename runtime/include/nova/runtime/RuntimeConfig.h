#pragma once

#include <cstdint>
#include <string_view>

namespace Nova::Runtime {

enum class Platform : std::uint8_t {
    Windows,
    Linux,
    macOS,
    Android,
    iOS,
    Web,
    Unknown,
};

enum class BuildType : std::uint8_t {
    Editor,
    Game,
    DedicatedServer,
};

struct RuntimeConfig {
    BuildType build_type = BuildType::Game;
    bool enable_audio = true;
    bool enable_physics = true;
    bool enable_scripting = true;
    bool enable_editor_hooks = false;
};

constexpr std::string_view build_type_name(BuildType type) noexcept {
    switch (type) {
        case BuildType::Editor: return "Editor";
        case BuildType::Game: return "Game";
        case BuildType::DedicatedServer: return "DedicatedServer";
    }
    return "Unknown";
}

} // namespace Nova::Runtime
