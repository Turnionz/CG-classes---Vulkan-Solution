#ifndef DEPTHBUFFER_H
#define DEPTHBUFFER_H

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture.hpp"

namespace Engine {
// Inheriting Texture class for the sole abbility to create images and well work
// with them
class DepthBuffer : public Engine::Texture {
public:
  void init(Engine::Swapchain &swapchain, Engine::Device &device);
  vk::Format findDepthFormat();
  [[nodiscard]] vk::ImageView getDepthImageView() const;
  [[nodiscard]] const vk::raii::Image &getDepthImage() const;

  void recreateDepthResource();

private:
  Engine::Swapchain *pSwapchain;
  vk::raii::Image depthImage = nullptr;
  vk::raii::DeviceMemory depthImageMemory = nullptr;
  vk::raii::ImageView depthImageView = nullptr;

  void createDepthResource();
  void cleanupDepthResource();
  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates,
                                 vk::ImageTiling tiling,
                                 vk::FormatFeatureFlags featues);
};
} // namespace Engine

#endif // !DEPTHBUFFER_H
