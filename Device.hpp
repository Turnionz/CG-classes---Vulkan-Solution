#ifndef DEVICE_H
#define DEVICE_H

#include "Instance.hpp"
#include "Sufrace.hpp"

namespace Engine {
class Device {
public:
  Device() = default;
  void init(Engine::Instance &instance, Engine::Surface &surface);

  [[nodiscard]] const vk::raii::PhysicalDevice &getPhysicalDevice() const;
  [[nodiscard]] const vk::raii::Device &getDevice() const;
  [[nodiscard]] const vk::raii::Queue &getQueue() const;
  uint32_t getQueueIndex();

private:
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device device = nullptr;
  vk::raii::Queue queue = nullptr;
  uint32_t queueIndex = ~0;
  std::vector<const char *> requiredDeviceExtension = {
      vk::KHRSwapchainExtensionName};

  void pickPhysicalDevice(Engine::Instance &instance);
  bool isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice);
  void createLogicalDevice(Engine::Surface &surface);
};
} // namespace Engine

#endif // !DEVICE_H
