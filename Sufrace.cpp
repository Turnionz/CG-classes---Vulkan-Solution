#include "Sufrace.hpp"

void Engine::Surface::init(Engine::Instance &instance, Engine::Window &window) {
  createSurface(instance, window);
}

void Engine::Surface::createSurface(Engine::Instance &instance,
                                    Engine::Window &window) {
  VkSurfaceKHR _surface;

  // Cast instance as poitner to convert from vk::raii::Instance to raw
  // VkInstance
  VkInstance rawInstance = *instance.getInstance();
  GLFWwindow *rawWindow = window.getWindow();

  VkResult err =
      glfwCreateWindowSurface(rawInstance, rawWindow, nullptr, &_surface);
  if (err != 0) {
    std::cout << "Vulkan Error Code:" << err << std::endl;
    throw std::runtime_error("Failed to create window surface!");
  }

  // Pass instance reference instead of a pointer
  surface = vk::raii::SurfaceKHR(instance.getInstance(), _surface);
}

const vk::raii::SurfaceKHR &Engine::Surface::getSurface() const {
  return surface;
}
