#include "Pipeline.hpp"

void Engine::Pipeline::init(
    Engine::Device &device, Engine::Swapchain &swapchain,
    Engine::DescriptorLayout &descriptorLayout,
    const std::string vertShaderPName, const std::string fragShaderPName,
    vk::PipelineRasterizationStateCreateInfo customRasterizer) {
  pDevice = &device;

  createGraphicsPipeline(swapchain, descriptorLayout, vertShaderPName,
                         fragShaderPName, customRasterizer);
}

const vk::raii::PipelineLayout &Engine::Pipeline::getPipelineLayout() const {
  return pipelineLayout;
}

const vk::raii::Pipeline &Engine::Pipeline::getGraphicsPipeline() const {
  return graphicsPipeline;
}

void Engine::Pipeline::createGraphicsPipeline(
    Engine::Swapchain &swapchain, Engine::DescriptorLayout &descriptorLayout,
    const std::string vertShaderPName, const std::string fragShaderPName,
    vk::PipelineRasterizationStateCreateInfo customRasterizer) {
  vk::raii::ShaderModule shaderModule =
      createShaderModule(readFile("shaders/slang.spv"));

  // Info for vertex and fragment shader
  vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eVertex,
      .module = shaderModule,
      .pName = vertShaderPName.c_str()};
  vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .module = shaderModule,
      .pName = fragShaderPName.c_str()};
  vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

  // Since most of pipelines states do not have default states in Vulkan, we
  // have to implement them ourselves It is unfortunately a neccesary step, but
  // we can't avoid it

  auto bindingDescription = Engine::Mesh::Vertex::getBindingDescription();
  auto attributeDescriptions = Engine::Mesh::Vertex::getAttributeDescriptions();
  vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(attributeDescriptions.size()),
      .pVertexAttributeDescriptions = attributeDescriptions.data()};

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
      .topology = vk::PrimitiveTopology::eTriangleList};
  vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1,
                                                    .scissorCount = 1};

  vk::PipelineRasterizationStateCreateInfo rasterizer{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eCounterClockwise,
      .depthBiasEnable = vk::False,
      .lineWidth = 1.0f};

  vk::PipelineMultisampleStateCreateInfo multisampling{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = vk::False};

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = vk::False,
      .colorWriteMask =
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

  vk::PipelineColorBlendStateCreateInfo colorBlending{
      .logicOpEnable = vk::False,
      .logicOp = vk::LogicOp::eCopy,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = 1,
      .pSetLayouts = &*descriptorLayout.getDescriptorSetLayout(),
      .pushConstantRangeCount = 0};

  pipelineLayout =
      vk::raii::PipelineLayout(pDevice->getDevice(), pipelineLayoutInfo);

  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                 vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo dynamicState{
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data()};

  vk::Format swapchainFormat = swapchain.getSwapChainSurfaceFormat().format;
  vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
      .colorAttachmentCount = 1, .pColorAttachmentFormats = &swapchainFormat};

  vk::StructureChain<vk::GraphicsPipelineCreateInfo,
                     vk::PipelineRenderingCreateInfo>
      pipelineChainInfoCreate(
          vk::GraphicsPipelineCreateInfo{.stageCount = 2,
                                         .pStages = shaderStages,
                                         .pVertexInputState = &vertexInputInfo,
                                         .pInputAssemblyState = &inputAssembly,
                                         .pViewportState = &viewportState,
                                         .pRasterizationState =
                                             &customRasterizer,
                                         .pMultisampleState = &multisampling,
                                         .pColorBlendState = &colorBlending,
                                         .pDynamicState = &dynamicState,
                                         .layout = pipelineLayout,
                                         .renderPass = nullptr},
          vk::PipelineRenderingCreateInfo{.colorAttachmentCount = 1,
                                          .pColorAttachmentFormats =
                                              &swapchainFormat});

  graphicsPipeline = vk::raii::Pipeline(
      pDevice->getDevice(), nullptr,
      pipelineChainInfoCreate.get<vk::GraphicsPipelineCreateInfo>());
}

std::vector<char> Engine::Pipeline::readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }
  std::vector<char> buffer(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  file.close();
  return buffer;
}

vk::raii::ShaderModule
Engine::Pipeline::createShaderModule(const std::vector<char> &code) const {
  vk::ShaderModuleCreateInfo createInfo{
      .codeSize = code.size() * sizeof(char),
      .pCode = reinterpret_cast<const uint32_t *>(code.data())};
  return vk::raii::ShaderModule{pDevice->getDevice(), createInfo};
}
