#ifndef SURFACE_H
#define SURFACE_H

#include "Instance.hpp"
#include "Window.hpp"

namespace Engine {
class Surface {
public:
  Surface() = default;
  void init(Engine::Instance &instance, Engine::Window &window);
  [[nodiscard]] const vk::raii::SurfaceKHR &getSurface() const;

private:
  vk::raii::SurfaceKHR surface = nullptr;

  void createSurface(Engine::Instance &instance, Engine::Window &window);
};
} // namespace Engine

#endif // !SURFACE_H
