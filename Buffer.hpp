#ifndef BUFFER_H
#define BUFFER_H

#include "DescriptorLayout.hpp"
#include "Mesh.hpp"
#include "Pipeline.hpp"
#include "Swapchain.hpp"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {
class Buffer {
public:
  Buffer(Engine::Device &device, vk::raii::CommandPool &commandPool,
         vk::DeviceSize size, vk::BufferUsageFlags usage,
         vk::MemoryPropertyFlags properties);

  void map();
  void unmap();
  void writeToBuffer(const void *data, size_t size);
  void copyBuffer(const vk::raii::Buffer &srcBuffer,
                  vk::raii::Buffer &dstBuffer, vk::DeviceSize size);
  // Copy to this buffer
  void copyBuffer(const vk::raii::Buffer &srcBuffer, vk::DeviceSize size);

  [[nodiscard]] const vk::raii::Buffer &getBuffer() const;

private:
  uint32_t frameIndex = 0;

  vk::raii::Buffer buffer = nullptr;
  vk::raii::DeviceMemory bufferMemory = nullptr;
  void *bufferMapped = nullptr;
  vk::DeviceSize bufferSize;
  vk::raii::CommandPool *pCommandPool;
  Engine::Device *pDevice;

  void createCommandPool();
  void createVertexBuffer();
  void createIndexBuffer();
  void createUniformBuffer();
  std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
  createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
               vk::MemoryPropertyFlags properties);
  uint32_t findMemoryType(uint32_t typeFilter,
                          vk::MemoryPropertyFlags properties);

  void createCommandBuffers();
  void transition_image_layout(uint32_t imageIndex, vk::ImageLayout old_layout,
                               vk::ImageLayout new_layout,
                               vk::AccessFlags2 src_access_mask,
                               vk::AccessFlags2 dst_access_mask,
                               vk::PipelineStageFlags2 src_stage_mask,
                               vk::PipelineStageFlags2 dst_stage_mask);
};
} // namespace Engine

#endif // !BUFFER_H
