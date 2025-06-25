set(BGFX_CONFIG_DEBUG ${CMAKE_BUILD_TYPE} STREQUAL "Debug"})
set(GLFW_BUILD_DEBUG ${CMAKE_BUILD_TYPE} STREQUAL "Debug"})

# BGFX (assuming it has its own CMakeLists)
set(BGFX_WITH_WAYLAND ON CACHE BOOL "Enable wayland support")
add_subdirectory(bgfx.cmake)

# GLFW (built from source)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "Enable Wayland support")
set(GLFW_BUILD_X11 ON CACHE BOOL "Enable X11 fallback")


add_subdirectory(glfw)
message(STATUS "GLFW Wayland support: ${GLFW_BUILD_WAYLAND}")
message(STATUS "GLFW X11 support: ${GLFW_BUILD_X11}")

# GLM (built from source)
set(GLM_BUILD_TESTS OFF CACHE BOOL "Build the GLM test programs")
set(GLM_BUILD_SAMPLES OFF CACHE BOOL "Build the GLM sample programs")
add_subdirectory(glm)
