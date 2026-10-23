#ifndef APPLICATION_H
#define APPLICATION_H

#include "Buffer.hpp"
#include "DescriptorLayout.hpp"
#include "DescriptorManager.hpp"
#include "Device.hpp"
#include "Instance.hpp"
#include "Pipeline.hpp"
#include "Sufrace.hpp"
#include "Swapchain.hpp"
#include "SyncObjects.hpp"
#include "Window.hpp"

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
  Engine::Buffer buffer;
  Engine::DescriptorManager descriptor;
  Engine::SyncObejcst syncObjects;

  void drawFrame();
  void mainLoop();
};
} // namespace Engine

#endif // !APPLICATION_H
