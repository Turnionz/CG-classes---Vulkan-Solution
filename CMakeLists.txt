cmake_minimum_required(VERSION 3.32)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_MODULE_STD 1)

project(VulkanSolution LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_MODULE_STD 1)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(VulkanSolution ${SOURCES})

find_package(Vulkan REQUIRED)

# set up Vulkan module
add_library(VulkanCppModule)
add_library(Vulkan::cppm ALIAS VulkanCppModule)

target_compile_definitions(VulkanCppModule PUBLIC
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)

target_include_directories(VulkanCppModule PRIVATE "${Vulkan_INCLUDE_DIR}")

target_link_libraries(VulkanCppModule PUBLIC Vulkan::Vulkan)

set_target_properties(VulkanCppModule PROPERTIES CXX_STANDARD 23)

target_sources(VulkanCppModule
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS "${Vulkan_INCLUDE_DIR}"
        FILES "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
)

# set up the GLFW library
find_package(glfw3 REQUIRED)

# set up GLM library
find_package(glm CONFIG REQUIRED)

# find slangc for CMake to use
find_program(SLANGC_EXECUTABLE slangc REQUIRED)

# slang shader target so we don't have to repeatedly compile the shaders ourselves
function(add_slang_shader_target TARGET)
  cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
  set(SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)
  set(ENTRY_POINT -entry vertMain -entry fragMain)
  message(STATUS "Slang compiler path: ${SLANGC_EXECUTABLE}")
  add_custom_command(
    OUTPUT ${SHADERS_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
  )
  add_custom_command(
    OUTPUT ${SHADERS_DIR}/slang.spv
    COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINT} -o slang.spv
    WORKING_DIRECTORY ${SHADERS_DIR}
    DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
    COMMENT "Compiling Slang Shaders"
    VERBATIM
  )
  add_custom_target(${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

set(SHADER_SLANG_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.slang"
)

add_slang_shader_target(SlangTarget SOURCES ${SHADER_SLANG_SOURCES})
add_dependencies(VulkanSolution SlangTarget)

# link the libraries to the app
target_link_libraries(VulkanSolution PRIVATE Vulkan::cppm)
target_link_libraries(VulkanSolution PRIVATE glm::glm)
target_link_libraries(VulkanSolution PRIVATE glfw)
