#include "MemoryManager.hpp"

vk::raii::CommandBuffer Engine::MemoryManager::beginSingleTimeCommands() {
  vk::CommandBufferAllocateInfo allocInfo{.commandPool = **this->pCommandPool,
                                          .level =
                                              vk::CommandBufferLevel::ePrimary,
                                          .commandBufferCount = 1};
  vk::raii::CommandBuffer commandBuffer = std::move(
      vk::raii::CommandBuffers(pDevice->getDevice(), allocInfo).front());

  vk::CommandBufferBeginInfo beginInfo{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  commandBuffer.begin(beginInfo);

  return std::move(commandBuffer);
}

void Engine::MemoryManager::endSingleTimeCommands(
    vk::raii::CommandBuffer &&commandBuffer) {
  commandBuffer.end();

  vk::SubmitInfo submitInfo{.commandBufferCount = 1,
                            .pCommandBuffers = &*commandBuffer};
  this->pDevice->getQueue().submit(submitInfo, nullptr);
  this->pDevice->getQueue().waitIdle();
}

void Engine::MemoryManager::copyBuffer(const vk::raii::Buffer &srcBuffer,
                                       vk::raii::Buffer &dstBuffer,
                                       vk::DeviceSize size) {
  vk::raii::CommandBuffer commandCopyBuffer = beginSingleTimeCommands();
  commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer,
                               vk::BufferCopy{.size = size});
  endSingleTimeCommands(std::move(commandCopyBuffer));
}

uint32_t
Engine::MemoryManager::findMemoryType(uint32_t typeFilter,
                                      vk::MemoryPropertyFlags properties) {
  vk::PhysicalDeviceMemoryProperties memProperties =
      this->pDevice->getPhysicalDevice().getMemoryProperties();

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}
