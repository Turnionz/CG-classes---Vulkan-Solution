#include "Swapchain.hpp"

const vk::raii::SwapchainKHR &Engine::Swapchain::getSwapchain() const {
  return swapChain;
}

const vk::Extent2D &Engine::Swapchain::getSwapChainExtent() const {
  return swapChainExtent;
}

const std::vector<vk::raii::ImageView> &
Engine::Swapchain::getSwapChainImageViews() const {
  return swapChainImageViews;
}

const std::vector<vk::Image> &Engine::Swapchain::getSwapChainImages() const {
  return swapChainImages;
}

const vk::SurfaceFormatKHR &
Engine::Swapchain::getSwapChainSurfaceFormat() const {
  return swapChainSurfaceFormat;
}

void Engine::Swapchain::init(Engine::Device &device, Engine::Surface &surface,
                             Engine::Window &window) {
  try {
    pDevice = &device;
    pSurface = &surface;
    pWindow = &window;
    createSwapChain();
    createImageViews();
  } catch (std::exception &e) {
    throw std::runtime_error("Failed to create Swapchain and/or image views!");
  }
}

void Engine::Swapchain::createSwapChain() {
  VkSurfaceKHR rawSurface = *pSurface->getSurface();
  GLFWwindow *window = pWindow->getWindow();
  vk::raii::PhysicalDevice physicalDevice = pDevice->getPhysicalDevice();
  auto surfaceCapabilities =
      physicalDevice.getSurfaceCapabilitiesKHR(rawSurface);
  swapChainExtent = chooseSwapExtent(surfaceCapabilities);
  uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

  std::vector<vk::SurfaceFormatKHR> availableFormats =
      physicalDevice.getSurfaceFormatsKHR(rawSurface);
  swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

  std::vector<vk::PresentModeKHR> availablePresentMode =
      physicalDevice.getSurfacePresentModesKHR(rawSurface);
  vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentMode);

  vk::SwapchainCreateInfoKHR swapChainCreateInfo{
      .surface = rawSurface,
      .minImageCount = minImageCount,
      .imageFormat = swapChainSurfaceFormat.format,
      .imageColorSpace = swapChainSurfaceFormat.colorSpace,
      .imageExtent = swapChainExtent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = chooseSwapPresentMode(availablePresentMode),
      .clipped = true};

  swapChain = vk::raii::SwapchainKHR(pDevice->getDevice(), swapChainCreateInfo);
  swapChainImages = swapChain.getImages();
}

void Engine::Swapchain::createImageViews() {
  assert(swapChainImageViews.empty());

  vk::ImageViewCreateInfo imageViewCreateInfo{
      .viewType = vk::ImageViewType::e2D,
      .format = swapChainSurfaceFormat.format,
      .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

  imageViewCreateInfo.components = {
      vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
      vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};

  imageViewCreateInfo.subresourceRange = {.aspectMask =
                                              vk::ImageAspectFlagBits::eColor,
                                          .levelCount = 1,
                                          .layerCount = 1};

  for (auto &image : swapChainImages) {
    imageViewCreateInfo.image = image;
    swapChainImageViews.emplace_back(pDevice->getDevice(), imageViewCreateInfo);
  }
}

void Engine::Swapchain::recreateSwapChain() {
  int width = 0, height = 0;
  GLFWwindow *window = pWindow->getWindow();
  // Have a const reference to the created device here
  const vk::raii::Device &device = pDevice->getDevice();
  glfwGetFramebufferSize(window, &width, &height);
  while ((width == 0 || height == 0) && !glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
  if (glfwWindowShouldClose(window)) {
    return;
  }
  device.waitIdle();

  cleanupSwapChain();
  createSwapChain();
  createImageViews();
}

void Engine::Swapchain::cleanupSwapChain() {
  swapChainImageViews.clear();
  swapChain = nullptr;
}

// Choose swapSurfaceFormat depending if it supports SRGB color space and color
// format
vk::SurfaceFormatKHR Engine::Swapchain::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  const auto formatIt =
      std::ranges::find_if(availableFormats, [](const auto &format) {
        return format.format == vk::Format::eB8G8R8A8Srgb &&
               format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
      });
  return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

// Choose Presentation mode for our SwapChain
// By default we will be using Fifo, but if Mailbox mode is available we will be
// using it instead
vk::PresentModeKHR Engine::Swapchain::chooseSwapPresentMode(
    std::vector<vk::PresentModeKHR> const &availablePresentMods) {
  assert(std::ranges::any_of(availablePresentMods, [](auto presentMode) {
    return presentMode == vk::PresentModeKHR::eFifo;
  }));

  return std::ranges::any_of(availablePresentMods,
                             [](const vk::PresentModeKHR value) {
                               return vk::PresentModeKHR::eMailbox == value;
                             })
             ? vk::PresentModeKHR::eMailbox
             : vk::PresentModeKHR::eFifo;
}

// Chose swapExtent for the SwapChain aka the resolution of SwapChain images
vk::Extent2D Engine::Swapchain::chooseSwapExtent(
    vk::SurfaceCapabilitiesKHR const &capabilities) {
  // Set currentExtent to "undefined" only when the window lets us choose the
  // extent ourselves, for any other value the surface dictates a fixed extent
  // that we must as-is
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  int width, height;
  glfwGetFramebufferSize(pWindow->getWindow(), &width, &height);

  // std::clamp for bounding width and height between allowed max and min
  // extents that are supported
  return {std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                               capabilities.maxImageExtent.width),
          std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height)};
}

uint32_t Engine::Swapchain::chooseSwapMinImageCount(
    const vk::SurfaceCapabilitiesKHR &surfaceCapabilities) {
  auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) &&
      (surfaceCapabilities.maxImageCount < minImageCount)) {
    minImageCount = surfaceCapabilities.maxImageCount;
  }

  return minImageCount;
}
