#include "Window.hpp"

bool Engine::Window::isFramebufferResized() { return framebufferResized; }

void Engine::Window::initWindow() { // Initialize the glfw library for the
                                    // window
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Engine::Window::init() {
  try {
    initWindow();
  } catch (std::exception &e) {
    throw std::runtime_error("Failed to initialize the window!");
  }
}
void Engine::Window::setFramebufferRezised(bool status) {
  framebufferResized = status;
};

void Engine::Window::framebufferResizeCallback(GLFWwindow *window, int width,
                                               int height) {
  auto app =
      reinterpret_cast<Engine::Window *>(glfwGetWindowUserPointer(window));
  app->setFramebufferRezised(true);
}

GLFWwindow *Engine::Window::getWindow() const { return window; }

void Engine::Window::cleanup() {
  glfwDestroyWindow(window);

  glfwTerminate();
}
