# BGFX (assuming it has its own CMakeLists)
add_subdirectory(bgfx.cmake)

# GLFW (built from source)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")
add_subdirectory(glfw)

# GLM (built from source)
set(GLM_BUILD_TESTS OFF CACHE BOOL "Build the GLM test programs")
set(GLM_BUILD_SAMPLES OFF CACHE BOOL "Build the GLM sample programs")
add_subdirectory(glm)
