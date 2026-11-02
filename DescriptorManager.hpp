#ifndef DESCRIPTORMANAGER_H
#define DESCRIPTORMANAGER_H

#include "Buffer.hpp"

namespace Engine {
class DescriptorManager {
public:
  DescriptorManager() = default;
  void init(Engine::Device &device, Engine::DescriptorLayout &descriptorLayout,
            std::vector<Engine::Buffer> &uniformBuffer);

  [[nodiscard]] const vk::raii::PipelineLayout &getPipelineLayout() const;

private:
  vk::raii::DescriptorPool descriptorPool = nullptr;
  vk::raii::PipelineLayout pipelineLayout = nullptr;
  void createDescriptorPool(Engine::Device &device);
  void createDescriptorSets(Engine::Device &device,
                            Engine::DescriptorLayout &descriptorLayout,
                            std::vector<Engine::Buffer> &uniformBuffer);
};
} // namespace Engine

#endif // !DESCRIPTORMANAGER_H
