#ifndef APPLICATION_H
#define APPLICATION_H

#include "CommandBuffer.hpp"
#include "DepthBuffer.hpp"
#include "DescriptorLayout.hpp"
#include "DescriptorManager.hpp"
#include "Device.hpp"
#include "Instance.hpp"
#include "Pipeline.hpp"
#include "Sufrace.hpp"
#include "Swapchain.hpp"
#include "SyncObjects.hpp"
#include "Texture.hpp"
#include "Window.hpp"
#include <numbers>
#include <optional>

namespace Engine {
class Application {
public:
  void run();

private:
  Engine::Window window;
  Engine::Instance instance;
  Engine::Surface surface;
  Engine::Device device;
  Engine::Swapchain swapchain;
  Engine::DescriptorLayout descriptorLayout;
  Engine::Pipeline pipeline;

  vk::raii::CommandPool commandPool = nullptr;

  Engine::DepthBuffer depthBuffer;

  Engine::Texture texture;
  std::vector<Engine::Buffer> vertexBuffer;
  std::optional<Engine::Buffer> indexBuffer;
  std::vector<Engine::Buffer> uniformBuffer;
  std::optional<Engine::CommandBuffer> commandBuffer;

  Engine::DescriptorManager descriptor;
  Engine::SyncObejcst syncObjects;

  void createObject();

  void updateUniformBuffer(uint32_t currentImage);
  void updateVertexBuffer(uint32_t currentImage);

  void drawFrame();
  void mainLoop();
};
} // namespace Engine

#endif // !APPLICATION_H
