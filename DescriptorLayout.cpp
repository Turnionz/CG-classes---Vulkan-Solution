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
  std::array<vk::DescriptorSetLayoutBinding, 2> bindings{
      {{.binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex},
       {.binding = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = MAX_TEXTURES,
        .stageFlags = vk::ShaderStageFlagBits::eFragment}}};

  std::array<vk::DescriptorBindingFlags, 2> bindingFlags{
      vk::DescriptorBindingFlags{},
      vk::DescriptorBindingFlagBits::ePartiallyBound |
          vk::DescriptorBindingFlagBits::eUpdateAfterBind};

  vk::StructureChain<vk::DescriptorSetLayoutCreateInfo,
                     vk::DescriptorSetLayoutBindingFlagsCreateInfo>
      layoutChain{
          vk::DescriptorSetLayoutCreateInfo{
              .flags =
                  vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
              .bindingCount = static_cast<uint32_t>(bindings.size()),
              .pBindings = bindings.data()},
          vk::DescriptorSetLayoutBindingFlagsCreateInfo{
              .bindingCount = static_cast<uint32_t>(bindingFlags.size()),
              .pBindingFlags = bindingFlags.data()}};

  descriptorSetLayout = vk::raii::DescriptorSetLayout(
      device.getDevice(), layoutChain.get<vk::DescriptorSetLayoutCreateInfo>());
}

void Engine::DescriptorLayout::allocateSets(
    const vk::raii::Device &device,
    const vk::DescriptorSetAllocateInfo &allocInfo) {
  descriptorSets = device.allocateDescriptorSets(allocInfo);
}
