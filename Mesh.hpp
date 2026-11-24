#ifndef MESH_H

#define MESH_H
#include <array>
#include <cstddef>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace Engine::Mesh {
struct Vertex {
  glm::vec3 pos;
  glm::vec4 color;
  glm::vec2 texCoord;
  uint32_t texIndex;

  static vk::VertexInputBindingDescription getBindingDescription();
  static std::array<vk::VertexInputAttributeDescription, 4>
  getAttributeDescriptions();
};

struct MeshPushConstants {
  glm::mat4 transform;
};

struct UniformBufferObject {
  glm::mat4 view;
  glm::mat4 proj;
};
} // namespace Engine::Mesh

const std::vector<Engine::Mesh::Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.5f}, {1.0f, 0.0f}, 0},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.5f}, {0.0f, 0.0f}, 0},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.5f}, {0.0f, 1.0f}, 0},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.5f}, {1.0f, 1.0f}, 0},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, 1},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 1},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, 1},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, 1}};
const std::vector<uint32_t> indicies = {6, 5, 4, 4, 7, 6, 2, 1, 0, 0, 3, 2};
#endif // !VERTEX_H VERTEX_H
