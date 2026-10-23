#include "DescriptorLayout.hpp"

const vk::raii::DescriptorSetLayout &
Engine::DescriptorLayout::getDescriptorSetLayout() const {
  return descriptorSetLayout;
};

const std::vector<vk::raii::DescriptorSet> &
Engine::DescriptorLayout::getDescriptorSets() const {
  return descriptorSets;
};

void Engine::DescriptorLayout::init(Engine::Device &device) {
  createDescriptorSetLayout(device);
}

void Engine::DescriptorLayout::createDescriptorSetLayout(
    Engine::Device &device) {
  vk::DescriptorSetLayoutBinding uboLayoutBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eVertex};
  vk::DescriptorSetLayoutCreateInfo layoutInfo{.bindingCount = 1,
                                               .pBindings = &uboLayoutBinding};
  descriptorSetLayout =
      vk::raii::DescriptorSetLayout(device.getDevice(), layoutInfo);
}

void Engine::DescriptorLayout::allocateSets(
    const vk::raii::Device &device,
    const vk::DescriptorSetAllocateInfo &allocInfo) {
  descriptorSets = device.allocateDescriptorSets(allocInfo);
}
