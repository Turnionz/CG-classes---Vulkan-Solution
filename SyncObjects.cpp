#include "SyncObjects.hpp"

const std::vector<vk::raii::Fence> &
Engine::SyncObejcst::getInFlightFence() const {
  return inFlightFence;
}

const std::vector<vk::raii::Semaphore> &
Engine::SyncObejcst::getPresentCompleteSemaphores() const {
  return presentCompleteSemaphores;
}

const std::vector<vk::raii::Semaphore> &
Engine::SyncObejcst::getRenderFinishedSemaphores() const {
  return renderFinishedSemaphores;
}

void Engine::SyncObejcst::init(Engine::Swapchain &swapchain,
                               Engine::Device &device) {
  createSyncObjects(swapchain, device);
}

void Engine::SyncObejcst::createSyncObjects(Engine::Swapchain &swapchain,
                                            Engine::Device &device) {
  assert(presentCompleteSemaphores.empty() &&
         renderFinishedSemaphores.empty() && inFlightFence.empty());

  for (size_t i = 0; i < swapchain.getSwapChainImages().size(); i++) {
    renderFinishedSemaphores.emplace_back(device.getDevice(),
                                          vk::SemaphoreCreateInfo());
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    presentCompleteSemaphores.emplace_back(device.getDevice(),
                                           vk::SemaphoreCreateInfo());
    inFlightFence.emplace_back(
        device.getDevice(),
        vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
  }
}
