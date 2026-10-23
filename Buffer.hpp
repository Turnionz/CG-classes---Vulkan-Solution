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
  Buffer() = default;
  void init(Engine::Pipeline &pipeline,
            Engine::DescriptorLayout &descriptorLayout,
            Engine::Swapchain &swapchain, Engine::Device &device);
  void initCommandBuffers();
  [[nodiscard]] const std::vector<vk::raii::Buffer> &getUniformBuffers() const;
  [[nodiscard]] const std::vector<vk::raii::CommandBuffer> &
  getCommandBuffers() const;
  void updateUniformBuffer(uint32_t currentImage);
  void recordCommandBuffer(uint32_t imageIndex);
  const uint32_t getFrameIndex();
  void setFrameIndex(uint32_t index);

private:
  uint32_t frameIndex = 0;
  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;
  vk::raii::CommandPool commandPool = nullptr;
  Engine::Pipeline *pPipeline;
  Engine::DescriptorLayout *pDescriptorLayout;
  Engine::Swapchain *pSwapChain;
  Engine::Device *pDeivce;
  std::vector<vk::raii::CommandBuffer> commandBuffers;
  std::vector<vk::raii::Buffer> uniformBuffers;
  std::vector<void *> uniformBuffersMapped;
  std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;

  void createCommandPool();
  void createVertexBuffer();
  void createIndexBuffer();
  void createUniformBuffer();
  std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
  createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
               vk::MemoryPropertyFlags properties);
  uint32_t findMemoryType(uint32_t typeFilter,
                          vk::MemoryPropertyFlags properties);
  void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer,
                  vk::DeviceSize size);
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
