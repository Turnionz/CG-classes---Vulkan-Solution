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

inline std::vector<Engine::Mesh::Vertex> vertices;
inline std::vector<uint32_t> indicies;
#endif // !VERTEX_H VERTEX_H
