#ifndef DESCRIPTORMANAGER_H
#define DESCRIPTORMANAGER_H

#include "Texture.hpp"

namespace Engine {
class DescriptorManager {
public:
  DescriptorManager() = default;
  void init(Engine::Device &device, Engine::DescriptorLayout &descriptorLayout,
            std::vector<Engine::Buffer> &uniformBuffer,
            Engine::Texture &texture);

  [[nodiscard]] const vk::raii::PipelineLayout &getPipelineLayout() const;

private:
  vk::raii::DescriptorPool descriptorPool = nullptr;
  vk::raii::PipelineLayout pipelineLayout = nullptr;
  Engine::Texture *pTexture;
  void createDescriptorPool(Engine::Device &device);
  void createDescriptorSets(Engine::Device &device,
                            Engine::DescriptorLayout &descriptorLayout,
                            std::vector<Engine::Buffer> &uniformBuffer);
};
} // namespace Engine

#endif // !DESCRIPTORMANAGER_H
