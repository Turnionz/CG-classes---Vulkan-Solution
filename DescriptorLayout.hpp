#ifndef DESCRIPTORLAYOUT_H
#define DESCRIPTORLAYOUT_H

#include "Device.hpp"

namespace Engine {
class DescriptorLayout {
public:
  DescriptorLayout() = default;
  void init(Engine::Device &device);
  [[nodiscard]] const vk::raii::DescriptorSetLayout &
  getDescriptorSetLayout() const;
  [[nodiscard]] const std::vector<vk::raii::DescriptorSet> &
  getDescriptorSets() const;
  void allocateSets(const vk::raii::Device &device,
                    const vk::DescriptorSetAllocateInfo &allocInfo);

private:
  vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
  std::vector<vk::raii::DescriptorSet> descriptorSets;

  void createDescriptorSetLayout(Engine::Device &device);
};
} // namespace Engine

#endif // !DESCRIPTOR_H
