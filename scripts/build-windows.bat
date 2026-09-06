@echo off
setlocal

set "ROOT=%~dp0.."
set "BUILD=%ROOT%\build-windows"

where cmake >nul 2>nul
if errorlevel 1 (
    echo CMake was not found. Install CMake and the Visual Studio C++ workload.
    exit /b 1
)

cmake -S "%ROOT%" -B "%BUILD%" -G "Visual Studio 17 2022" -A x64 ^
    -DNOVA_BUILD_TESTS=ON ^
    -DNOVA_BUILD_WIN32_GAME=ON ^
    -DNOVA_RHI_ENABLE_D3D12=OFF ^
    -DNOVA_RHI_ENABLE_D3D11=OFF ^
    -DNOVA_RHI_ENABLE_VULKAN=OFF
if errorlevel 1 exit /b %errorlevel%

cmake --build "%BUILD%" --config Release --target nova-win32 nova-runtime-entry-smoke nova-runtime-renderer-smoke nova-runtime-mesh-smoke
if errorlevel 1 exit /b %errorlevel%

echo.
echo NovaGame.exe:
echo %BUILD%\Release\NovaGame.exe
endlocal
