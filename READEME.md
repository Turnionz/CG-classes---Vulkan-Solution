## Vulkan based solution for Computer Graphics labs

To create he build folder use the following command
---
cmake -B build -G Ninja -S . -DCMAKE_TOOLCHAIN_FILE={path/to/glfw/installation} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
