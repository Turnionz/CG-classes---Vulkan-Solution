#include "Buffer.hpp"

void Engine::Buffer::init(Engine::Pipeline &pipeline,
                          Engine::DescriptorLayout &descriptorLayout,
                          Engine::Swapchain &swapchain,
                          Engine::Device &device) {
  pPipeline = &pipeline;
  pDescriptorLayout = &descriptorLayout;
  pDeivce = &device;
  pSwapChain = &swapchain;
  createCommandPool();
  createVertexBuffer();
  createIndexBuffer();
  createUniformBuffer();
};

void Engine::Buffer::initCommandBuffers() { createCommandBuffers(); }

const std::vector<vk::raii::Buffer> &Engine::Buffer::getUniformBuffers() const {
  return uniformBuffers;
};

const std::vector<vk::raii::CommandBuffer> &
Engine::Buffer::getCommandBuffers() const {
  return commandBuffers;
};

const uint32_t Engine::Buffer::getFrameIndex() { return frameIndex; }
void Engine::Buffer::setFrameIndex(uint32_t index) { frameIndex = index; }

void Engine::Buffer::createCommandPool() {
  vk::CommandPoolCreateInfo poolInfo{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = pDeivce->getQueueIndex()};

  commandPool = vk::raii::CommandPool(pDeivce->getDevice(), poolInfo);
}

void Engine::Buffer::createVertexBuffer() {
  vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  auto [stagingBuffer, stagingBufferMemory] =
      createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                   vk::MemoryPropertyFlagBits::eHostVisible |
                       vk::MemoryPropertyFlagBits::eHostCoherent);

  void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
  memcpy(dataStaging, vertices.data(), bufferSize);
  stagingBufferMemory.unmapMemory();

  std::tie(vertexBuffer, vertexBufferMemory) =
      createBuffer(bufferSize,
                   vk::BufferUsageFlagBits::eVertexBuffer |
                       vk::BufferUsageFlagBits::eTransferDst,
                   vk::MemoryPropertyFlagBits::eDeviceLocal);

  copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
};

void Engine::Buffer::createIndexBuffer() {
  vk::DeviceSize bufferSize = sizeof(indicies[0]) * indicies.size();

  auto [stagingBuffer, stagingBufferMemory] =
      createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                   vk::MemoryPropertyFlagBits::eHostVisible |
                       vk::MemoryPropertyFlagBits::eHostCoherent);

  void *data = stagingBufferMemory.mapMemory(0, bufferSize);
  memcpy(data, indicies.data(), (size_t)bufferSize);
  stagingBufferMemory.unmapMemory();

  std::tie(indexBuffer, indexBufferMemory) =
      createBuffer(bufferSize,
                   vk::BufferUsageFlagBits::eIndexBuffer |
                       vk::BufferUsageFlagBits::eTransferDst,
                   vk::MemoryPropertyFlagBits::eDeviceLocal);

  copyBuffer(stagingBuffer, indexBuffer, bufferSize);
};

void Engine::Buffer::createUniformBuffer() {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DeviceSize bufferSize = sizeof(Engine::Mesh::UniformBufferObject);
    auto [buffer, bufferMem] =
        createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
                     vk::MemoryPropertyFlagBits::eHostVisible |
                         vk::MemoryPropertyFlagBits::eHostCoherent);
    uniformBuffers.emplace_back(std::move(buffer));
    uniformBuffersMemory.emplace_back(std::move(bufferMem));
    uniformBuffersMapped.emplace_back(
        uniformBuffersMemory.back().mapMemory(0, bufferSize));
  }
};

void Engine::Buffer::updateUniformBuffer(uint32_t currentImage) {
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(
                   currentTime - startTime)
                   .count();

  Engine::Mesh::UniformBufferObject ubo{};
  ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                     glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(
      glm::radians(45.0f),
      static_cast<float>(pSwapChain->getSwapChainExtent().width) /
          static_cast<float>(pSwapChain->getSwapChainExtent().height),
      0.1f, 10.0f);
  memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
};

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
Engine::Buffer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                             vk::MemoryPropertyFlags properties) {
  vk::BufferCreateInfo bufferInfo{
      .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};
  vk::raii::Buffer buffer = vk::raii::Buffer(pDeivce->getDevice(), bufferInfo);
  vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

  vk::MemoryAllocateInfo allocInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits, properties)};

  vk::raii::DeviceMemory bufferMemory =
      vk::raii::DeviceMemory(pDeivce->getDevice(), allocInfo);
  buffer.bindMemory(*bufferMemory, 0);

  return {std::move(buffer), std::move(bufferMemory)};
};

uint32_t Engine::Buffer::findMemoryType(uint32_t typeFilter,
                                        vk::MemoryPropertyFlags properties) {
  vk::PhysicalDeviceMemoryProperties memProperties =
      pDeivce->getPhysicalDevice().getMemoryProperties();

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

void Engine::Buffer::copyBuffer(vk::raii::Buffer &srcBuffer,
                                vk::raii::Buffer &dstBuffer,
                                vk::DeviceSize size) {
  vk::CommandBufferAllocateInfo allocInfo{.commandPool = commandPool,
                                          .level =
                                              vk::CommandBufferLevel::ePrimary,
                                          .commandBufferCount = 1};
  vk::raii::CommandBuffer commandCopyBuffer =
      std::move(pDeivce->getDevice().allocateCommandBuffers(allocInfo).front());
  commandCopyBuffer.begin(
      {.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer,
                               vk::BufferCopy(0, 0, size));
  commandCopyBuffer.end();

  pDeivce->getQueue().submit(
      vk::SubmitInfo{.commandBufferCount = 1,
                     .pCommandBuffers = &*commandCopyBuffer},
      nullptr);
  pDeivce->getQueue().waitIdle();
};

void Engine::Buffer::createCommandBuffers() {
  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
  commandBuffers =
      std::move(vk::raii::CommandBuffers(pDeivce->getDevice(), allocInfo));
};

void Engine::Buffer::recordCommandBuffer(uint32_t imageIndex) {
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
  commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
  commandBuffer.bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint32);
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

void Engine::Buffer::transition_image_layout(
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
