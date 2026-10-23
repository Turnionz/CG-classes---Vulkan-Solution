#ifndef MESH_H

#define MESH_H
#include <array>
#include <cstddef>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace Engine::Mesh {
struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;

  static vk::VertexInputBindingDescription getBindingDescription();
  static std::array<vk::VertexInputAttributeDescription, 2>
  getAttributeDescriptions();
};

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};
} // namespace Engine::Mesh

const std::vector<Engine::Mesh::Vertex> vertices = {
    {{-0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 1.0f}}};

const std::vector<uint32_t> indicies = {0, 1, 2, 2, 3, 0, 0, 3, 4};

#endif // !VERTEX_H VERTEX_H
