#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "Device.hpp"
#include <limits>
#include <vector>

namespace Engine {
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

private:
  vk::raii::SwapchainKHR swapChain = nullptr;
  vk::Extent2D swapChainExtent;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  std::vector<vk::Image> swapChainImages;
  std::vector<vk::raii::ImageView> swapChainImageViews;

  Engine::Device *pDevice = nullptr;
  Engine::Surface *pSurface = nullptr;
  Engine::Window *pWindow = nullptr;

  void createSwapChain();
  void createImageViews();
  void cleanupSwapChain();

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
