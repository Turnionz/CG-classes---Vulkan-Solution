#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "Device.hpp"
#include <limits>
#include <vector>

namespace Engine {

// Forward declaration of DepthBuffer, we do it since ine recreateSwapChain we
// need to recreateDepthResource from DepthBuffer, but DepthBuffer objects need
// swapchain beforehand. So to avoid the closed dependency loop we use forward
// declaration
class DepthBuffer;

class Swapchain {
public:
  Swapchain() = default;
  void init(Engine::Device &device, Engine::Surface &surface,
            Engine::Window &window);
  [[nodiscard]] const vk::raii::SwapchainKHR &getSwapchain() const;
  [[nodiscard]] const vk::Extent2D &getSwapChainExtent() const;
  [[nodiscard]] const std::vector<vk::raii::ImageView> &
  getSwapChainImageViews() const;
  [[nodiscard]] const std::vector<vk::Image> &getSwapChainImages() const;
  [[nodiscard]] const vk::SurfaceFormatKHR &getSwapChainSurfaceFormat() const;
  void recreateSwapChain();
  void setDepthBuffer(DepthBuffer *depthBuffer) { pDepthBuffer = depthBuffer; }

private:
  vk::raii::SwapchainKHR swapChain = nullptr;
  vk::Extent2D swapChainExtent;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  std::vector<vk::Image> swapChainImages;
  std::vector<vk::raii::ImageView> swapChainImageViews;

  Engine::Device *pDevice = nullptr;
  Engine::Surface *pSurface = nullptr;
  Engine::Window *pWindow = nullptr;
  DepthBuffer *pDepthBuffer = nullptr;

  void createSwapChain();
  void createImageViews();
  void cleanupSwapChain();

  vk::raii::ImageView createImageView(vk::Image const &image, vk::Format format,
                                      vk::ImageAspectFlags aspectFlags,
                                      uint32_t mipLevels);

  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR> &availableFormats);
  vk::PresentModeKHR chooseSwapPresentMode(
      std::vector<vk::PresentModeKHR> const &availablePresentMods);
  vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
  static uint32_t chooseSwapMinImageCount(
      vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);
};
} // namespace Engine

#endif // !SWAPCHAIN_H
