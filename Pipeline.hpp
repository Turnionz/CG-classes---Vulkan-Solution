#ifndef PIPELINE_H
#define PIPELINE_H

#include "DescriptorLayout.hpp"
#include "Device.hpp"
#include "Mesh.hpp"
#include "Swapchain.hpp"
#include <fstream>

namespace Engine {
class Pipeline {
public:
  Pipeline() = default;
  void init(Engine::Device &device, Engine::Swapchain &swapchain,
            Engine::DescriptorLayout &descriptorLayout,
            const std::string vertShaderPName,
            const std::string fragShaderPName,
            vk::PipelineRasterizationStateCreateInfo customRasterizer);

  [[nodiscard]] const vk::raii::PipelineLayout &getPipelineLayout() const;
  [[nodiscard]] const vk::raii::Pipeline &getGraphicsPipeline() const;

private:
  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::Pipeline graphicsPipeline = nullptr;

  Engine::Device *pDevice;

  void createGraphicsPipeline(
      Engine::Swapchain &swapchain, Engine::DescriptorLayout &descriptorLayout,
      const std::string vertShaderPName, const std::string fragShaderPName,
      vk::PipelineRasterizationStateCreateInfo customRasterizer);
  std::vector<char> readFile(const std::string &filename);
  vk::raii::ShaderModule
  createShaderModule(const std::vector<char> &code) const;
};
} // namespace Engine

#endif // !PIPELINE_H
