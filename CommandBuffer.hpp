#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include "DepthBuffer.hpp"
#include "Pipeline.hpp"

namespace Engine {
class CommandBuffer {
public:
  CommandBuffer(Engine::Pipeline &pipeline,
                Engine::DescriptorLayout &descriptorLayout,
                Engine::Swapchain &swapchain, Engine::Device &device,
                vk::raii::CommandPool &commandPool);

  const uint32_t getFrameIndex();
  void setFrameIndex(uint32_t index);
  [[nodiscard]] const std::vector<vk::raii::CommandBuffer> &
  getCommandBuffers() const;
  void recordCommandBuffers(uint32_t imageIndex, const vk::raii::Buffer &vertex,
                            const vk::raii::Buffer &index,
                            Engine::DepthBuffer &depthBuffer);

private:
  uint32_t frameIndex = 0;

  Engine::Pipeline *pPipeline;
  Engine::DescriptorLayout *pDescriptorLayout;
  Engine::Swapchain *pSwapChain;
  Engine::Device *pDevice;

  std::vector<vk::raii::CommandBuffer> commandBuffers;
  vk::raii::CommandPool *pCommandPool;

  void createCommandBuffers();
  void transition_image_layout(vk::Image image, vk::ImageLayout old_layout,
                               vk::ImageLayout new_layout,
                               vk::AccessFlags2 src_access_mask,
                               vk::AccessFlags2 dst_access_mask,
                               vk::PipelineStageFlags2 src_stage_mask,
                               vk::PipelineStageFlags2 dst_stage_mask,
                               vk::ImageAspectFlags aspectFlags);
};
} // namespace Engine

#endif // !COMMANDBUFFER_H
