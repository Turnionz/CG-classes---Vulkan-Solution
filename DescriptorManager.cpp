#include "DescriptorManager.hpp"

[[nodiscard]] const vk::raii::PipelineLayout &
Engine::DescriptorManager::getPipelineLayout() const {
  return pipelineLayout;
}

void Engine::DescriptorManager::init(Engine::Device &device,
                                     Engine::DescriptorLayout &descriptorLayout,
                                     std::vector<Engine::Buffer> &uniformBuffer,
                                     Engine::Texture &texture) {
  pTexture = &texture;

  createDescriptorPool(device);
  createDescriptorSets(device, descriptorLayout, uniformBuffer);
}

void Engine::DescriptorManager::createDescriptorPool(Engine::Device &device) {
  std::array<vk::DescriptorPoolSize, 2> poolSize{
      {{.type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = MAX_FRAMES_IN_FLIGHT},
       {.type = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = MAX_FRAMES_IN_FLIGHT}}};
  vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = static_cast<uint32_t>(poolSize.size()),
      .pPoolSizes = poolSize.data()};
  descriptorPool = vk::raii::DescriptorPool(device.getDevice(), poolInfo);
}

void Engine::DescriptorManager::createDescriptorSets(
    Engine::Device &device, Engine::DescriptorLayout &descriptorLayout,
    std::vector<Engine::Buffer> &uniformBuffer) {
  std::vector<vk::DescriptorSetLayout> layouts(
      MAX_FRAMES_IN_FLIGHT, *descriptorLayout.getDescriptorSetLayout());
  vk::DescriptorSetAllocateInfo allocInfo{
      .descriptorPool = descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data()};

  descriptorLayout.allocateSets(device.getDevice(), allocInfo);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DescriptorBufferInfo bufferInfo{
        .buffer = uniformBuffer[i].getBuffer(),
        .offset = 0,
        .range = sizeof(Engine::Mesh::UniformBufferObject)};
    vk::DescriptorImageInfo imageInfo{
        .sampler = pTexture->getTextureSampler(),
        .imageView = pTexture->getTextureImageView(),
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};
    std::array<vk::WriteDescriptorSet, 2> descriptorWrite{
        {{.dstSet = descriptorLayout.getDescriptorSets()[i],
          .dstBinding = 0,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = vk::DescriptorType::eUniformBuffer,
          .pBufferInfo = &bufferInfo},
         {.dstSet = descriptorLayout.getDescriptorSets()[i],
          .dstBinding = 1,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .pImageInfo = &imageInfo}}};

    device.getDevice().updateDescriptorSets(descriptorWrite, {});
  }
}
