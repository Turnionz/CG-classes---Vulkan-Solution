#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

void Engine::Texture::init(Engine::Device &device,
                           vk::raii::CommandPool &commandPool,
                           Engine::Swapchain &swapchain) {
  this->pDevice = &device;
  this->pCommandPool = &commandPool;
  this->pSwapchain = &swapchain;

  createTextureImage();
  createTextureImageView();
  createTextureSampler();
}

const vk::raii::Sampler &Engine::Texture::getTextureSampler() const {
  return textureSampler;
}

const vk::raii::ImageView &Engine::Texture::getTextureImageView() const {
  return textureImageView;
}

void Engine::Texture::createTextureImage() {
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load("textures/chriss_pratt.png", &texWidth,
                              &texHeight, &texChannels, STBI_rgb_alpha);
  vk::DeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }

  Engine::Buffer staging(*pDevice, *pCommandPool, imageSize,
                         vk::BufferUsageFlagBits::eTransferSrc,
                         vk::MemoryPropertyFlagBits::eHostVisible |
                             vk::MemoryPropertyFlagBits::eHostCoherent);

  staging.map();
  staging.writeToBuffer(pixels, imageSize);
  staging.unmap();

  stbi_image_free(pixels);

  std::tie(textureImage, textureImageMemory) = createImage(
      texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();
  transitionImageLayout(commandBuffer, textureImage,
                        vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eTransferDstOptimal);
  copyBufferToImage(commandBuffer, staging.getBuffer(), textureImage,
                    static_cast<uint32_t>(texWidth),
                    static_cast<uint32_t>(texHeight));
  transitionImageLayout(commandBuffer, textureImage,
                        vk::ImageLayout::eTransferDstOptimal,
                        vk::ImageLayout::eShaderReadOnlyOptimal);
  endSingleTimeCommands(std::move(commandBuffer));
}

void Engine::Texture::createTextureImageView() {
  textureImageView = createImageView(*textureImage, vk::Format::eR8G8B8A8Srgb,
                                     vk::ImageAspectFlagBits::eColor);
}

vk::raii::ImageView
Engine::Texture::createImageView(vk::Image const &image, vk::Format format,
                                 vk::ImageAspectFlags aspectFlags) {
  vk::ImageViewCreateInfo viewInfo{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = format,
      .subresourceRange = {.aspectMask = aspectFlags,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};
  return vk::raii::ImageView(pDevice->getDevice(), viewInfo);
}

void Engine::Texture::createTextureSampler() {
  vk::PhysicalDeviceProperties properties =
      pDevice->getPhysicalDevice().getProperties();
  vk::SamplerCreateInfo samplerInfo{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      .addressModeU = vk::SamplerAddressMode::eRepeat,
      .addressModeV = vk::SamplerAddressMode::eRepeat,
      .addressModeW = vk::SamplerAddressMode::eRepeat,
      .mipLodBias = 0.0f,
      .anisotropyEnable = vk::False,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways};

  textureSampler = vk::raii::Sampler(pDevice->getDevice(), samplerInfo);
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory>
Engine::Texture::createImage(uint32_t width, uint32_t height, vk::Format format,
                             vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                             vk::MemoryPropertyFlags properties) {
  vk::ImageCreateInfo imageInfo{.imageType = vk::ImageType::e2D,
                                .format = format,
                                .extent = {width, height, 1},
                                .mipLevels = 1,
                                .arrayLayers = 1,
                                .samples = vk::SampleCountFlagBits::e1,
                                .tiling = tiling,
                                .usage = usage,
                                .sharingMode = vk::SharingMode::eExclusive};
  vk::raii::Image image = vk::raii::Image(pDevice->getDevice(), imageInfo);

  vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits, properties)};
  vk::raii::DeviceMemory imageMemory =
      vk::raii::DeviceMemory(pDevice->getDevice(), allocInfo);
  image.bindMemory(imageMemory, 0);

  return {std::move(image), std::move(imageMemory)};
}

void Engine::Texture::transitionImageLayout(
    vk::raii::CommandBuffer &commandBuffer, const vk::raii::Image &image,
    vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
  vk::ImageMemoryBarrier barrier{
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
      .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
      .image = image,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .levelCount = 1,
                           .layerCount = 1}};

  vk::PipelineStageFlags sourceStage;
  vk::PipelineStageFlags destinationStage;

  if (oldLayout == vk::ImageLayout::eUndefined &&
      newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = {};
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
             newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    sourceStage = vk::PipelineStageFlagBits::eTransfer;
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    throw std::invalid_argument("unsuported layer transition!");
  }

  commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr,
                                barrier);
}

void Engine::Texture::copyBufferToImage(vk::raii::CommandBuffer &commandBuffer,
                                        const vk::raii::Buffer &buffer,
                                        vk::raii::Image &image, uint32_t width,
                                        uint32_t height) {
  vk::BufferImageCopy region{
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .mipLevel = 0,
                           .baseArrayLayer = 0,
                           .layerCount = 1},
      .imageOffset = {0, 0, 0},
      .imageExtent = {width, height, 1}};

  commandBuffer.copyBufferToImage(buffer, image,
                                  vk::ImageLayout::eTransferDstOptimal, region);
}
