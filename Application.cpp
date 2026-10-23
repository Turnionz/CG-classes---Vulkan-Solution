#include "Application.hpp"

void Engine::Application::run() {
  window.init();
  instance.init();

  surface.init(instance, window);
  device.init(instance, surface);

  swapchain.init(device, surface, window);

  descriptorLayout.init(device);

  vk::PipelineRasterizationStateCreateInfo rasterizer{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eCounterClockwise,
      .depthBiasEnable = vk::False,
      .lineWidth = 1.0f};

  pipeline.init(device, swapchain, descriptorLayout, "vertMain", "fragMain",
                rasterizer);
  buffer.init(pipeline, descriptorLayout, swapchain, device);
  descriptor.init(device, descriptorLayout, buffer);
  buffer.initCommandBuffers();
  syncObjects.init(swapchain, device);

  mainLoop();
  window.cleanup();
}

void Engine::Application::drawFrame() {
  // NOTE: inFlightFence, presentCompleteSemaphores and commandBufferCount are
  // indexed by frameIndex, while renderFinishedSemaphores is indexed by
  // imageIndex
  uint32_t frameIndex = buffer.getFrameIndex();
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

  buffer.getCommandBuffers()[frameIndex].reset();
  buffer.recordCommandBuffer(imageIndex);

  buffer.updateUniformBuffer(frameIndex);

  vk::PipelineStageFlags waitDestinationStageMask(
      vk::PipelineStageFlagBits::eColorAttachmentOutput);
  const vk::SubmitInfo submitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          &*syncObjects.getPresentCompleteSemaphores()[frameIndex],
      .pWaitDstStageMask = &waitDestinationStageMask,
      .commandBufferCount = 1,
      .pCommandBuffers = &*buffer.getCommandBuffers()[frameIndex],
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
  buffer.setFrameIndex((frameIndex + 1) % MAX_FRAMES_IN_FLIGHT);
}

void Engine::Application::mainLoop() {
  while (!(glfwWindowShouldClose(window.getWindow()))) {
    glfwPollEvents();
    drawFrame();
  }

  device.getDevice().waitIdle();
}
