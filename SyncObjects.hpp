#ifndef SYNCOBJECTS_H
#define SYNCOBJECTS_H

#include "Swapchain.hpp"

namespace Engine {
class SyncObejcst {
public:
  SyncObejcst() = default;
  void init(Engine::Swapchain &swapchain, Engine::Device &device);
  [[nodiscard]] const std::vector<vk::raii::Fence> &getInFlightFence() const;
  [[nodiscard]] const std::vector<vk::raii::Semaphore> &
  getPresentCompleteSemaphores() const;
  [[nodiscard]] const std::vector<vk::raii::Semaphore> &
  getRenderFinishedSemaphores() const;

private:
  std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
  std::vector<vk::raii::Fence> inFlightFence;

  void createSyncObjects(Engine::Swapchain &swapchain, Engine::Device &device);
};
} // namespace Engine

#endif // !SYNCOBJECTS_H
