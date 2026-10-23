#include "Instance.hpp"

void Engine::Instance::init() {
  try {
    Engine::Instance::createInstance();
    Engine::Instance::createDebugMessenger();
  } catch (std::exception &e) {
    throw std::runtime_error(
        "Failed to create instance and/or debugMessenger!");
  }
}

void Engine::Instance::createInstance() {
  // Set the app info
  constexpr vk::ApplicationInfo appInfo{
      .pApplicationName = "VulkanSoltuion",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = vk::ApiVersion14};

  // Get the required layers
  std::vector<char const *> requiredLayers;
  if (enableValidationLayers) {
    requiredLayers.assign(validationLayers.begin(), validationLayers.end());
  }

  // Check if the required layers are supported by Vulkan implemntation
  auto layerProperties = context.enumerateInstanceLayerProperties();
  auto unsupportedLayerIt = std::ranges::find_if(
      requiredLayers, [&layerProperties](auto const &requiredLayer) {
        return std::ranges::none_of(
            layerProperties, [requiredLayer](auto const &layerProperty) {
              return strcmp(layerProperty.layerName, requiredLayer) == 0;
            });
      });

  if (unsupportedLayerIt != requiredLayers.end()) {
    throw std::runtime_error("Required layer is not supported: " +
                             std::string(*unsupportedLayerIt));
  }

  // Get required instance extensions from GLFW
  auto requiredExtensions = getRequiredInstanceExtensions();

  // Check if required extensions are supported by Vulkan implemntation
  auto extensionProperties = context.enumerateInstanceExtensionProperties();
  auto unsupportedPropertyIt = std::ranges::find_if(
      requiredExtensions,
      [&extensionProperties](const auto &requiredExtension) {
        return std::ranges::none_of(
            extensionProperties,
            [requiredExtension](const auto &extensionProperty) {
              return strcmp(extensionProperty.extensionName,
                            requiredExtension) == 0;
            });
      });

  if (unsupportedPropertyIt != requiredExtensions.end()) {
    throw std::runtime_error("Required extension not supported: " +
                             std::string(*unsupportedPropertyIt));
  }

  vk::InstanceCreateInfo createInfo{
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
      .ppEnabledLayerNames = requiredLayers.data(),
      .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
      .ppEnabledExtensionNames = requiredExtensions.data()};
  instance = vk::raii::Instance(context, createInfo);
}

void Engine::Instance::createDebugMessenger() {
  if (!enableValidationLayers)
    return;

  // Config of what type of messages we want to see
  vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
  vk::DebugUtilsMessageTypeFlagsEXT messageFlags(
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
  vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
      .messageSeverity = severityFlags,
      .messageType = messageFlags,
      .pfnUserCallback = &debugCallback};
  debugMessenger =
      instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL Engine::Instance::debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning ||
      severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
    std::cerr << "validation layer: type " << to_string(type)
              << " msg: " << pCallbackData->pMessage << std::endl;
  }

  return vk::False;
}

const vk::raii::Instance &Engine::Instance::getInstance() const {
  return instance;
}

std::vector<const char *> Engine::Instance::getRequiredInstanceExtensions() {
  uint32_t glfwExtensionCount = 0;
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (enableValidationLayers) {
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
  }

  return extensions;
}
