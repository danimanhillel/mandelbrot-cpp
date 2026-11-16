# Mandelbrot Explorer - C++

High-performance multi-threaded C++ Mandelbrot set explorer using SDL2.

Uses native double precision for deeper zoom than the WebGL version.

## Features
- Multi-threaded rendering (uses all CPU cores)
- Smooth pan & zoom
- Adjustable iterations, hue, saturation
- Fast and accurate with double precision

## Controls
- **Mouse drag**: Pan
- **Scroll wheel**: Zoom
- **R**: Reset view
- **Up/Down arrows**: Increase/Decrease max iterations
- **Left/Right arrows**: Change hue
- **+/-**: Change saturation

## Build on Windows (recommended with MinGW or MSVC + vcpkg)

### Option 1: With vcpkg (easiest)
```bash
vcpkg install sdl2
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

### Option 2: With MSYS2 / MinGW
```bash
pacman -S mingw-w64-x86_64-sdl2 mingw-w64-x86_64-cmake
cmake -B build -S .
cmake --build build
```

The executable will be in `build/Release/mandelbrot.exe` or `build/mandelbrot.exe`.

## Build on Linux
```bash
sudo apt install libsdl2-dev cmake
cmake -B build -S .
cmake --build build
./build/mandelbrot
```

Created with Grok.
