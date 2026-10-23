#include "Device.hpp"

const vk::raii::PhysicalDevice &Engine::Device::getPhysicalDevice() const {
  return physicalDevice;
}
const vk::raii::Device &Engine::Device::getDevice() const { return device; }
const vk::raii::Queue &Engine::Device::getQueue() const { return queue; }
uint32_t Engine::Device::getQueueIndex() { return queueIndex; }

void Engine::Device::init(Engine::Instance &instance,
                          Engine::Surface &surface) {
  pickPhysicalDevice(instance);
  isDeviceSuitable(physicalDevice);
  createLogicalDevice(surface);
}

void Engine::Device::pickPhysicalDevice(Engine::Instance &instance) {
  // Let the Vulkan list all the GPUs available for us to use
  std::vector<vk::raii::PhysicalDevice> physicalDevices =
      instance.getInstance().enumeratePhysicalDevices();
  auto const devIter =
      std::ranges::find_if(physicalDevices, [&](auto const &physicalDevice) {
        return isDeviceSuitable(physicalDevice);
      });
  if (devIter == physicalDevices.end()) {
    throw std::runtime_error("failed to find suitable a GPU!");
  }
  physicalDevice = *devIter;
}

bool Engine::Device::isDeviceSuitable(
    const vk::raii::PhysicalDevice &physicalDevice) {
  // Self explanatory
  bool supportsVulkan1_4 =
      physicalDevice.getProperties().apiVersion >= vk::ApiVersion14;

  // Check if any of the queue famiies support graphics operations
  auto queueFamily = physicalDevice.getQueueFamilyProperties();
  bool supportGraphics = std::ranges::any_of(queueFamily, [](auto const &qfp) {
    return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
  });

  // Check if all required physicalDevice extensions are available
  auto availableDeviceExtensions =
      physicalDevice.enumerateDeviceExtensionProperties();
  bool supportAllRequiredExtensions = std::ranges::all_of(
      requiredDeviceExtension,
      [&availableDeviceExtensions](auto const &requireDeviceExtension) {
        return std::ranges::any_of(
            availableDeviceExtensions,
            [requireDeviceExtension](const auto &availableDeviceExtension) {
              return strcmp(availableDeviceExtension.extensionName,
                            requireDeviceExtension) == 0;
            });
      });

  // Check if physicalDevice supports required features
  auto features = physicalDevice.template getFeatures2<
      vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
      vk::PhysicalDeviceVulkan13Features,
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

  bool supportsRequiredFeatures =
      features.template get<vk::PhysicalDeviceVulkan11Features>()
          .shaderDrawParameters &&
      features.template get<vk::PhysicalDeviceVulkan13Features>()
          .dynamicRendering &&
      features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
          .extendedDynamicState;

  // Return true if all criterias are satisfied
  return supportAllRequiredExtensions && supportsVulkan1_4 &&
         supportsRequiredFeatures && supportGraphics;
}

void Engine::Device::createLogicalDevice(Engine::Surface &surface) {
  // Find the index of first queue family which supports graphics
  std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
      physicalDevice.getQueueFamilyProperties();

  for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size();
       qfpIndex++) {
    // Get raw VkSurfaceKHR pointer
    VkSurfaceKHR rawSurface = *surface.getSurface();
    if ((queueFamilyProperties[qfpIndex].queueFlags &
         vk::QueueFlagBits::eGraphics) &&
        physicalDevice.getSurfaceSupportKHR(qfpIndex, rawSurface)) {
      // Find a queue family that supports both graphics and present
      queueIndex = qfpIndex;
      break;
    }
  }
  if (queueIndex == ~0) {
    std::runtime_error(
        "  Could not fina a queue for graphics and present - terminating");
  }

  // get the first index into queueFamilyProperties which supports graphics
  auto graphicsQueueFamilyProperty =
      std::ranges::find_if(queueFamilyProperties, [](const auto &qfp) {
        return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) !=
               static_cast<vk::QueueFlags>(0);
      });

  assert(graphicsQueueFamilyProperty != queueFamilyProperties.end() &&
         "No graphics queue family found!");

  auto graphicsIndex = static_cast<uint32_t>(std::distance(
      queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

  // Chain of feature structures
  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      featureChain = {
          {}, // vk::PhysicalDeviceFeatures2 (empty for now)
          {.shaderDrawParameters = true}, // Enable shader draw from Vulkan 1.1
          {.synchronization2 = true,
           .dynamicRendering =
               true}, // Enable dynamic rendering from Vulkan 1.3
          {.extendedDynamicState =
               true} // Enable extended dynamic state from the extension
  };

  // Creaating the device
  float queuePriority = 0.5f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
      .queueFamilyIndex = graphicsIndex,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};
  vk::DeviceCreateInfo deviceCreateInfo{
      .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &deviceQueueCreateInfo,
      .enabledExtensionCount =
          static_cast<uint32_t>(requiredDeviceExtension.size()),
      .ppEnabledExtensionNames = requiredDeviceExtension.data()};

  device = vk::raii::Device(physicalDevice, deviceCreateInfo);
  queue = vk::raii::Queue(device, queueIndex, 0);
}
