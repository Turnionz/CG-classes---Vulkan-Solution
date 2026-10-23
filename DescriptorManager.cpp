#include "DescriptorManager.hpp"

[[nodiscard]] const vk::raii::PipelineLayout &
Engine::DescriptorManager::getPipelineLayout() const {
  return pipelineLayout;
}

void Engine::DescriptorManager::init(Engine::Device &device,
                                     Engine::DescriptorLayout &descriptorLayout,
                                     Engine::Buffer &buffer) {
  createDescriptorPool(device);
  createDescriptorSets(device, descriptorLayout, buffer);
}

void Engine::DescriptorManager::createDescriptorPool(Engine::Device &device) {
  vk::DescriptorPoolSize poolSize{.type = vk::DescriptorType::eUniformBuffer,
                                  .descriptorCount = MAX_FRAMES_IN_FLIGHT};
  vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize};
  descriptorPool = vk::raii::DescriptorPool(device.getDevice(), poolInfo);
}

void Engine::DescriptorManager::createDescriptorSets(
    Engine::Device &device, Engine::DescriptorLayout &descriptorLayout,
    Engine::Buffer &buffer) {
  std::vector<vk::DescriptorSetLayout> layouts(
      MAX_FRAMES_IN_FLIGHT, *descriptorLayout.getDescriptorSetLayout());
  vk::DescriptorSetAllocateInfo allocInfo{
      .descriptorPool = descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data()};

  descriptorLayout.allocateSets(device.getDevice(), allocInfo);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DescriptorBufferInfo bufferInfo{
        .buffer = buffer.getUniformBuffers()[i],
        .offset = 0,
        .range = sizeof(Engine::Mesh::UniformBufferObject)};
    vk::WriteDescriptorSet descriptorWrite{
        .dstSet = descriptorLayout.getDescriptorSets()[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &bufferInfo};

    device.getDevice().updateDescriptorSets(descriptorWrite, {});
  }
}
