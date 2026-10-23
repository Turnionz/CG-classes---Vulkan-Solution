#ifndef INSTANCE_H
#define INSTANCE_H

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "Window.hpp"

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif // NDEBUG

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<char const *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#include <ranges>

namespace Engine {
class Instance {
public:
  vk::raii::DebugUtilsMessengerEXT
  createDebugMessenger(const vk::DebugUtilsMessengerCreateInfoEXT &createInfo);
  [[nodiscard]] const vk::raii::Instance &getInstance() const;

  void init();
  Instance() = default;

  static std::vector<const char *> getRequiredInstanceExtensions();

private:
  vk::raii::Instance instance = nullptr;
  vk::raii::Context context;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

  void createInstance();
  void createDebugMessenger();
  static VKAPI_ATTR vk::Bool32 VKAPI_CALL
  debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                vk::DebugUtilsMessageTypeFlagsEXT type,
                const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData);
};
} // namespace Engine

#endif // !INSTANCE_H
