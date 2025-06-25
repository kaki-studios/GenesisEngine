set(BGFX_CONFIG_DEBUG ${CMAKE_BUILD_TYPE} STREQUAL "Debug"})

# BGFX (assuming it has its own CMakeLists)
set(BGFX_WITH_WAYLAND ON CACHE BOOL "Enable wayland support")
add_subdirectory(bgfx.cmake)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable shared libs")
add_subdirectory(SDL)

# GLM (built from source)
set(GLM_BUILD_TESTS OFF CACHE BOOL "Build the GLM test programs")
set(GLM_BUILD_SAMPLES OFF CACHE BOOL "Build the GLM sample programs")
add_subdirectory(glm)
