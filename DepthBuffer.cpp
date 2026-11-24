#include "DepthBuffer.hpp"

void Engine::DepthBuffer::init(Engine::Swapchain &swapchain,
                               Engine::Device &device) {
  this->pSwapchain = &swapchain;
  this->pDevice = &device;
  createDepthResource();
}

void Engine::DepthBuffer::createDepthResource() {
  vk::Format depthFormat = findDepthFormat();

  // Creating depth image
  std::tie(depthImage, depthImageMemory) =
      createImage(pSwapchain->getSwapChainExtent().width,
                  pSwapchain->getSwapChainExtent().height, 1, depthFormat,
                  vk::ImageTiling::eOptimal,
                  vk::ImageUsageFlagBits::eDepthStencilAttachment,
                  vk::MemoryPropertyFlagBits::eDeviceLocal);

  // Creating image view with aspect specifying that it is a depth image, not a
  // regular color image
  depthImageView = createImageView(depthImage, depthFormat,
                                   vk::ImageAspectFlagBits::eDepth, 1);
}

// Returns a pointer to depthImageView
//
// So far it returns a pointer to the obejct, mainly because we don't need to be
// a reference
vk::ImageView Engine::DepthBuffer::getDepthImageView() const {
  return *depthImageView;
}

const vk::raii::Image &Engine::DepthBuffer::getDepthImage() const {
  return depthImage;
}

// Going through each format candidate to see if it satisfies our needs
vk::Format Engine::DepthBuffer::findSupportedFormat(
    const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
    vk::FormatFeatureFlags featues) {
  for (const auto format : candidates) {
    vk::FormatProperties props =
        pDevice->getPhysicalDevice().getFormatProperties(format);

    if (((tiling == vk::ImageTiling::eLinear) &&
         ((props.linearTilingFeatures & featues) == featues)) ||
        ((tiling == vk::ImageTiling::eOptimal) &&
         ((props.optimalTilingFeatures & featues) == featues))) {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported formats");
}

void Engine::DepthBuffer::cleanupDepthResource() {
  // Order matters!
  depthImageView = nullptr;
  depthImage = nullptr;
  depthImageMemory = nullptr;
}

void Engine::DepthBuffer::recreateDepthResource() {
  pDevice->getDevice().waitIdle();
  cleanupDepthResource();
  createDepthResource();
}

vk::Format Engine::DepthBuffer::findDepthFormat() {
  return findSupportedFormat(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
       vk::Format::eD24UnormS8Uint},
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}
