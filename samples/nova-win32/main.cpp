#include <windows.h>

#include <nova/runtime/NovaEngine.h>

namespace {
constexpr wchar_t kWindowClass[] = L"NovaEngineWin32Sample";
constexpr wchar_t kWindowTitle[] = L"Nova Engine - Windows Runtime";

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd, message, wparam, lparam);
    }
}

bool create_window(HINSTANCE instance, HWND& hwnd) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.hInstance = instance;
    wc.lpfnWndProc = window_proc;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kWindowClass;
    if (!RegisterClassExW(&wc)) return false;

    hwnd = CreateWindowExW(
        0, kWindowClass, kWindowTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        nullptr, nullptr, instance, nullptr);
    if (!hwnd) {
        UnregisterClassW(kWindowClass, instance);
        return false;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    return true;
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show_command) {
    Nova::Engine::RuntimeInstance runtime;
    if (!Nova::Engine::initialize(runtime, {Nova::RHI::Backend::Null, false})) {
        MessageBoxW(nullptr, L"Nova Runtime initialization failed.", L"Nova Engine", MB_ICONERROR | MB_OK);
        return 1;
    }

    HWND hwnd = nullptr;
    if (!create_window(instance, hwnd)) {
        Nova::Engine::shutdown(runtime);
        MessageBoxW(nullptr, L"Win32 window creation failed.", L"Nova Engine", MB_ICONERROR | MB_OK);
        return 2;
    }

    ShowWindow(hwnd, show_command);

    MSG message{};
    while (true) {
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                Nova::Engine::shutdown(runtime);
                UnregisterClassW(kWindowClass, instance);
                return static_cast<int>(message.wParam);
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        Sleep(1);
    }
}
