#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "Device.hpp"

namespace Engine {
class MemoryManager {
public:
  void copyBuffer(const vk::raii::Buffer &srcBuffer,
                  vk::raii::Buffer &dstBuffer, vk::DeviceSize size);
  uint32_t findMemoryType(uint32_t typeFilter,
                          vk::MemoryPropertyFlags properties);
  vk::raii::CommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(vk::raii::CommandBuffer &&commandBuffer);

protected:
  Engine::Device *pDevice;
  vk::raii::CommandPool *pCommandPool;
};
} // namespace Engine
#endif // MEMORYMANAGER_H
