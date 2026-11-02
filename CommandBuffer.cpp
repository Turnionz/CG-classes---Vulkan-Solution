#include "CommandBuffer.hpp"

Engine::CommandBuffer::CommandBuffer(Engine::Pipeline &pipeline,
                                     Engine::DescriptorLayout &descriptorLayout,
                                     Engine::Swapchain &swapchain,
                                     Engine::Device &device,
                                     vk::raii::CommandPool &commandPool) {
  pPipeline = &pipeline;
  pDescriptorLayout = &descriptorLayout;
  pSwapChain = &swapchain;
  pDevice = &device;
  pCommandPool = &commandPool;
  createCommandBuffers();
}

const uint32_t Engine::CommandBuffer::getFrameIndex() { return frameIndex; }
void Engine::CommandBuffer::setFrameIndex(uint32_t index) {
  frameIndex = index;
}
const std::vector<vk::raii::CommandBuffer> &
Engine::CommandBuffer::getCommandBuffers() const {
  return commandBuffers;
};

void Engine::CommandBuffer::createCommandBuffers() {
  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = **pCommandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
  commandBuffers =
      std::move(vk::raii::CommandBuffers(pDevice->getDevice(), allocInfo));
};

void Engine::CommandBuffer::recordCommandBuffers(
    uint32_t imageIndex, const vk::raii::Buffer &vertex,
    const vk::raii::Buffer &index) {
  auto &commandBuffer = commandBuffers[frameIndex];
  commandBuffer.begin({});

  VkExtent2D swapChainExtent = pSwapChain->getSwapChainExtent();

  // Before we start rendering we need to transition the swapchain image to
  // vk::ImageLayout::eColorAttachmentOutput
  transition_image_layout(imageIndex, vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eColorAttachmentOptimal, {},
                          vk::AccessFlagBits2::eColorAttachmentWrite,
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput);

  vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
  vk::RenderingAttachmentInfo attachmentInfo = {
      .imageView = pSwapChain->getSwapChainImageViews()[imageIndex],
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = clearColor};

  vk::RenderingInfo renderingInfo = {
      .renderArea = {.offset = {0, 0},
                     .extent = pSwapChain->getSwapChainExtent()},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo};

  commandBuffer.beginRendering(renderingInfo);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             *pPipeline->getGraphicsPipeline());
  // NOTE: Flipping the height by setting height to be negative; uniform buffers
  // is the reason (no further explanation will be given for now, live with it)
  commandBuffer.setViewport(
      0, vk::Viewport(
             0.0f, static_cast<float>(pSwapChain->getSwapChainExtent().height),
             static_cast<float>(pSwapChain->getSwapChainExtent().width),
             -static_cast<float>(pSwapChain->getSwapChainExtent().height), 0.0f,
             1.0f));
  commandBuffer.setScissor(
      0, vk::Rect2D(vk::Offset2D(0, 0), pSwapChain->getSwapChainExtent()));
  commandBuffer.bindVertexBuffers(0, *vertex, {0});
  commandBuffer.bindIndexBuffer(*index, 0, vk::IndexType::eUint32);
  commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, pPipeline->getPipelineLayout(), 0,
      *pDescriptorLayout->getDescriptorSets()[frameIndex], nullptr);
  commandBuffer.drawIndexed(static_cast<uint32_t>(indicies.size()), 1, 0, 0, 0);
  commandBuffer.endRendering();

  // After rendering, we need to transition the swapchain image to
  // vk::ImageLayout::ePresentSrcKHR
  transition_image_layout(imageIndex, vk::ImageLayout::eColorAttachmentOptimal,
                          vk::ImageLayout::ePresentSrcKHR,
                          vk::AccessFlagBits2::eColorAttachmentWrite, {},
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                          vk::PipelineStageFlagBits2::eBottomOfPipe);

  commandBuffer.end();
};

void Engine::CommandBuffer::transition_image_layout(
    uint32_t imageIndex, vk::ImageLayout old_layout, vk::ImageLayout new_layout,
    vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask,
    vk::PipelineStageFlags2 src_stage_mask,
    vk::PipelineStageFlags2 dst_stage_mask) {
  vk::ImageMemoryBarrier2 barrier = {
      .srcStageMask = src_stage_mask,
      .srcAccessMask = src_access_mask,
      .dstStageMask = dst_stage_mask,
      .dstAccessMask = dst_access_mask,
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = pSwapChain->getSwapChainImages()[imageIndex],
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};

  vk::DependencyInfo dependency_info = {.dependencyFlags = {},
                                        .imageMemoryBarrierCount = 1,
                                        .pImageMemoryBarriers = &barrier};

  commandBuffers[frameIndex].pipelineBarrier2(dependency_info);
}
