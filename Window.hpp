#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

namespace Engine {
class Window {
public:
  [[nodiscard]] GLFWwindow *getWindow() const;
  bool isFramebufferResized();

  void setFramebufferRezised(bool status);
  Window() = default;
  void init();

  void cleanup();

private:
  GLFWwindow *window = nullptr;

  bool framebufferResized = false;
  void initWindow();
  static void framebufferResizeCallback(GLFWwindow *window, int width,
                                        int height);
};
} // namespace Engine

#endif // !WINDOW_H
