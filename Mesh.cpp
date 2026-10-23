#include "Mesh.hpp"

vk::VertexInputBindingDescription
Engine::Mesh::Vertex::getBindingDescription() {
  return {.binding = 0,
          .stride = sizeof(Vertex),
          .inputRate = vk::VertexInputRate::eVertex};
}

std::array<vk::VertexInputAttributeDescription, 2>
Engine::Mesh::Vertex::getAttributeDescriptions() {
  return {{{.location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(Vertex, pos)},
           {.location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(Vertex, color)}}};
}
