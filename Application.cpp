#include "Application.hpp"

void Engine::Application::run() {
  window.init();
  instance.init();

  surface.init(instance, window);
  device.init(instance, surface);

  swapchain.init(device, surface, window);

  descriptorLayout.init(device);

  depthBuffer.init(swapchain, device);

  swapchain.setDepthBuffer(&depthBuffer);

  vk::PipelineRasterizationStateCreateInfo rasterizer{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eClockwise,
      .depthBiasEnable = vk::False,
      .lineWidth = 1.0f};

  pipeline.init(device, swapchain, descriptorLayout, depthBuffer, "vertMain",
                "fragMain", rasterizer);

  vk::CommandPoolCreateInfo poolInfo{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = device.getQueueIndex()};
  commandPool = vk::raii::CommandPool(device.getDevice(), poolInfo);

  // createObject();

  texture.init(device, commandPool, swapchain);

  vk::DeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vertexBuffer.emplace_back(device, commandPool, vertexBufferSize,
                              vk::BufferUsageFlagBits::eVertexBuffer,
                              vk::MemoryPropertyFlagBits::eHostVisible |
                                  vk::MemoryPropertyFlagBits::eHostCoherent);

    // Map each buffer because we will be updating it every frame
    vertexBuffer.back().map();
  }

  vk::DeviceSize indexBufferSize = sizeof(indicies[0]) * indicies.size();
  Engine::Buffer staging(device, commandPool, indexBufferSize,
                         vk::BufferUsageFlagBits::eTransferSrc,
                         vk::MemoryPropertyFlagBits::eHostVisible |
                             vk::MemoryPropertyFlagBits::eHostCoherent);
  indexBuffer.emplace(device, commandPool, indexBufferSize,
                      vk::BufferUsageFlagBits::eIndexBuffer |
                          vk::BufferUsageFlagBits::eTransferDst,
                      vk::MemoryPropertyFlagBits::eDeviceLocal);

  staging.map();
  staging.writeToBuffer(indicies.data(), indexBufferSize);
  staging.unmap();

  indexBuffer->copyBuffer(staging.getBuffer(), indexBufferSize);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    uniformBuffer.emplace_back(device, commandPool,
                               sizeof(Engine::Mesh::UniformBufferObject),
                               vk::BufferUsageFlagBits::eUniformBuffer,
                               vk::MemoryPropertyFlagBits::eHostVisible |
                                   vk::MemoryPropertyFlagBits::eHostCoherent);

    uniformBuffer.back().map();
  }

  descriptor.init(device, descriptorLayout, uniformBuffer, texture);
  this->commandBuffer.emplace(pipeline, descriptorLayout, swapchain, device,
                              commandPool);
  syncObjects.init(swapchain, device);

  mainLoop();
  window.cleanup();
}

void Engine::Application::drawFrame() {
  // NOTE: inFlightFence, presentCompleteSemaphores and commandBufferCount are
  // indexed by frameIndex, while renderFinishedSemaphores is indexed by
  // imageIndex
  uint32_t frameIndex = commandBuffer->getFrameIndex();
  auto fenceResult = device.getDevice().waitForFences(
      *syncObjects.getInFlightFence()[frameIndex], vk::True, UINT64_MAX);
  if (fenceResult != vk::Result::eSuccess) {
    throw std::runtime_error("failed to wait for fence!");
  }

  auto [result, imageIndex] = swapchain.getSwapchain().acquireNextImage(
      UINT64_MAX, *syncObjects.getPresentCompleteSemaphores()[frameIndex],
      nullptr);

  if (result == vk::Result::eErrorOutOfDateKHR) {
    swapchain.recreateSwapChain();
    return;
  }
  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  device.getDevice().resetFences(*syncObjects.getInFlightFence()[frameIndex]);

  commandBuffer->getCommandBuffers()[frameIndex].reset();
  commandBuffer->recordCommandBuffers(imageIndex,
                                      vertexBuffer[frameIndex].getBuffer(),
                                      indexBuffer->getBuffer(), depthBuffer);

  updateUniformBuffer(frameIndex);
  vertexBuffer[frameIndex].writeToBuffer(vertices.data(),
                                         sizeof(vertices[0]) * vertices.size());
  // updateVertexBuffer(frameIndex);

  vk::PipelineStageFlags waitDestinationStageMask(
      vk::PipelineStageFlagBits::eColorAttachmentOutput);
  const vk::SubmitInfo submitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          &*syncObjects.getPresentCompleteSemaphores()[frameIndex],
      .pWaitDstStageMask = &waitDestinationStageMask,
      .commandBufferCount = 1,
      .pCommandBuffers = &*commandBuffer->getCommandBuffers()[frameIndex],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores =
          &*syncObjects.getRenderFinishedSemaphores()[imageIndex]};

  device.getQueue().submit(submitInfo,
                           *syncObjects.getInFlightFence()[frameIndex]);

  const vk::PresentInfoKHR presentInfoKHR{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          &*syncObjects.getRenderFinishedSemaphores()[imageIndex],
      .swapchainCount = 1,
      .pSwapchains = &*swapchain.getSwapchain(),
      .pImageIndices = &imageIndex};

  result = device.getQueue().presentKHR(presentInfoKHR);
  if ((result == vk::Result::eSuboptimalKHR) ||
      (result == vk::Result::eErrorOutOfDateKHR) ||
      window.isFramebufferResized()) {
    window.setFramebufferRezised(false);
    swapchain.recreateSwapChain();
  } else {
    assert(result == vk::Result::eSuccess);
  }
  commandBuffer->setFrameIndex((frameIndex + 1) % MAX_FRAMES_IN_FLIGHT);
}
/*
void Engine::Application::createObject() {
  float h = 0.5f;
  glm::vec3 color = {1.0f, 0.0f, 0.0f};
  vertices.push_back({{h, h, h}, color});   // A
  vertices.push_back({{h, -h, h}, color});  // A1
  vertices.push_back({{-h, h, h}, color});  // B
  vertices.push_back({{-h, -h, h}, color}); // B1

  color = {1.0f, 1.0f, 0.0f};
  vertices.push_back({{-h, h, -h}, color});  // C
  vertices.push_back({{-h, -h, -h}, color}); // C1
  vertices.push_back({{h, h, -h}, color});   // D
  vertices.push_back({{h, -h, -h}, color});  // D1

  // For the love of god don't do it like this, this is just cursed and tedious
  indicies.insert(indicies.end(),
                  {0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 7, 1, 0, 0, 6, 7,
                   4, 6, 0, 0, 2, 4, 1, 7, 5, 5, 3, 1, 3, 5, 4, 4, 2, 3});
}*/

void Engine::Application::updateUniformBuffer(uint32_t currentImage) {
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(
                   currentTime - startTime)
                   .count();

  Engine::Mesh::UniformBufferObject ubo{};
  ubo.model = rotate(glm::mat4(3.0f), time * glm::radians(90.0f),
                     glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(
      glm::radians(45.0f),
      static_cast<float>(swapchain.getSwapChainExtent().width) /
          static_cast<float>(swapchain.getSwapChainExtent().height),
      0.1f, 10.0f);
  uniformBuffer[currentImage].writeToBuffer(&ubo, sizeof(ubo));
}
/*
void Engine::Application::updateVertexBuffer(uint32_t currentImage) {
  static float colorA = 0.0f;
  static float colorB = 0.0f;
  static float colorC = 0.0f;
  static auto lastTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(
                        currentTime - lastTime)
                        .count();
  colorA = std::sin(deltaTime * 2.0f) + 1.0f;
  colorB = std::sin(deltaTime * 2.0f + 1.0f) + 1.0f;
  colorC = std::sin(deltaTime * 2.0f + 3.14f) + 1.0f;

  vertices[0].color = {colorB, 0.0f, colorA};
  vertices[1].color = {colorB, colorA, 0.0f};
  vertices[2].color = {colorC, 0.0f, colorB};
  vertices[3].color = {colorC, 0.0f, colorC};
  vertices[4].color = {colorA, colorB, 0.0f};
  vertices[5].color = {0.0f, colorC, colorB};
  vertices[6].color = {colorC, colorA, colorB};
  vertices[7].color = {colorB, colorC, colorA};
  vertexBuffer[currentImage].writeToBuffer(
      vertices.data(), sizeof(vertices[0]) * vertices.size());
}*/

void Engine::Application::mainLoop() {
  while (!(glfwWindowShouldClose(window.getWindow()))) {
    glfwPollEvents();
    drawFrame();
  }

  device.getDevice().waitIdle();
}
