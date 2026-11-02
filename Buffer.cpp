#include "Buffer.hpp"

// Initializing a buffer
// This class doesn't create multiple buffers inside of it, so for real time
// updates of a buffer, create an array with size of MAX_FRAMES_IN_FLIGHT and
// update each object by a seperate method
Engine::Buffer::Buffer(Engine::Device &device,
                       vk::raii::CommandPool &commandPool, vk::DeviceSize size,
                       vk::BufferUsageFlags usage,
                       vk::MemoryPropertyFlags properties) {
  this->pDevice = &device;
  this->pCommandPool = &commandPool;

  // Tie the bufer and bufferMemory to the class at creation
  std::tie(buffer, bufferMemory) = createBuffer(size, usage, properties);
  bufferSize = size;
}

void Engine::Buffer::map() {
  bufferMapped = bufferMemory.mapMemory(0, bufferSize);
}

void Engine::Buffer::unmap() {
  bufferMemory.unmapMemory();
  bufferMapped = nullptr;
}

// Either load up the data to the buffer or update it
void Engine::Buffer::writeToBuffer(const void *data, size_t size) {
  if (bufferMapped) {
    memcpy(bufferMapped, data, size);
  }
}

const vk::raii::Buffer &Engine::Buffer::getBuffer() const { return buffer; }

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
Engine::Buffer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                             vk::MemoryPropertyFlags properties) {
  vk::BufferCreateInfo bufferInfo{
      .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};
  vk::raii::Buffer createBuf =
      vk::raii::Buffer(pDevice->getDevice(), bufferInfo);
  vk::MemoryRequirements memRequirements = createBuf.getMemoryRequirements();

  vk::MemoryAllocateInfo allocInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits, properties)};

  vk::raii::DeviceMemory createBufferMemory =
      vk::raii::DeviceMemory(pDevice->getDevice(), allocInfo);
  createBuf.bindMemory(*createBufferMemory, 0);

  return {std::move(createBuf), std::move(createBufferMemory)};
};

void Engine::Buffer::copyBuffer(const vk::raii::Buffer &srcBuffer,
                                vk::DeviceSize size) {
  vk::raii::CommandBuffer commandCopyBuffer = beginSingleTimeCommands();
  commandCopyBuffer.copyBuffer(*srcBuffer, this->buffer,
                               vk::BufferCopy{.size = size});
  endSingleTimeCommands(std::move(commandCopyBuffer));
}
