#ifndef TEXTURE_H
#define TEXTURE_H

#include "Buffer.hpp"
#include "MemoryManager.hpp"
#include "Swapchain.hpp"

namespace Engine {
class Texture : public MemoryManager {
public:
  void init(Engine::Device &device, vk::raii::CommandPool &commandPool,
            Engine::Swapchain &swapchain, std::string filename);
  std::pair<vk::raii::Image, vk::raii::DeviceMemory>
  createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
              vk::Format format, vk::ImageTiling tiling,
              vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
  vk::raii::ImageView createImageView(vk::Image const &image, vk::Format format,
                                      vk::ImageAspectFlags aspectFlags,
                                      uint32_t mipLevels);

  [[nodiscard]] const vk::raii::Sampler &getTextureSampler() const;
  [[nodiscard]] const vk::raii::ImageView &getTextureImageView() const;

private:
  Engine::Swapchain *pSwapchain;
  uint32_t mipLevels = 0;
  vk::raii::Image textureImage = nullptr;
  vk::raii::DeviceMemory textureImageMemory = nullptr;
  vk::raii::ImageView textureImageView = nullptr;
  vk::raii::Sampler textureSampler = nullptr;

  void createTextureImage(std::string filename);
  void createTextureImageView();
  void createTextureSampler();
  void createImageViews();

  void generateMipmaps(vk::raii::CommandBuffer &commandBuffer,
                       vk::raii::Image &image, vk::Format imageFormat,
                       int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

  void transitionImageLayout(vk::raii::CommandBuffer &commandBuffer,
                             const vk::raii::Image &image,
                             vk::ImageLayout oldLayout,
                             vk::ImageLayout newLayout, uint32_t mipLevels);
  void copyBufferToImage(vk::raii::CommandBuffer &commandBuffer,
                         const vk::raii::Buffer &buffer, vk::raii::Image &image,
                         uint32_t width, uint32_t height);
};
} // namespace Engine

#endif // !TEXTURE_H
