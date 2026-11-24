#include "DescriptorManager.hpp"

[[nodiscard]] const vk::raii::PipelineLayout &
Engine::DescriptorManager::getPipelineLayout() const {
  return pipelineLayout;
}

void Engine::DescriptorManager::init(Engine::Device &device,
                                     Engine::DescriptorLayout &descriptorLayout,
                                     std::vector<Engine::Buffer> &uniformBuffer,
                                     std::vector<Engine::Texture> &textures) {
  pTextures = &textures;

  createDescriptorPool(device);
  createDescriptorSets(device, descriptorLayout, uniformBuffer);
}

void Engine::DescriptorManager::createDescriptorPool(Engine::Device &device) {
  std::array<vk::DescriptorPoolSize, 2> poolSize{
      {{.type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = MAX_FRAMES_IN_FLIGHT},
       {.type = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = MAX_FRAMES_IN_FLIGHT * MAX_TEXTURES}}};
  vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet |
               vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
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

  std::vector<vk::DescriptorImageInfo> imageInfos;
  imageInfos.reserve(pTextures->size());
  for (auto &tex : *pTextures) {
    imageInfos.push_back(vk::DescriptorImageInfo{
        .sampler = tex.getTextureSampler(),
        .imageView = tex.getTextureImageView(),
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal});
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DescriptorBufferInfo bufferInfo{
        .buffer = uniformBuffer[i].getBuffer(),
        .offset = 0,
        .range = sizeof(Engine::Mesh::UniformBufferObject)};

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
          .descriptorCount = static_cast<uint32_t>(imageInfos.size()),
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .pImageInfo = imageInfos.data()}}};

    device.getDevice().updateDescriptorSets(descriptorWrite, {});
  }
}
