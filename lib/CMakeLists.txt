set(BGFX_BUILD_TOOLS ON CACHE BOOL "" FORCE)  # Keeps shaderc, geometryc, etc.

# Nuclear option to kill example-common
set(BGFX_CONFIG_UTILS OFF CACHE BOOL "" FORCE) 
set(BGFX_BUILD_EXAMPLE_COMMON OFF CACHE BOOL "Disable bgfx from building imgui")
set(BGFX_CONFIG_UTILS OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "Disable bgfx from building imgui")
set(BGFX_INSTALL_EXAMPLES OFF CACHE BOOL "Disable bgfx from building imgui" FORCE)
set(BGFX_CONFIG_DEBUG ${CMAKE_BUILD_TYPE} STREQUAL "Debug"})
set(BGFX_BUILD_TOOLS_SHADER ON CACHE BOOL "Enable shader compilation")

# BGFX (assuming it has its own CMakeLists)
set(BGFX_WITH_WAYLAND ON CACHE BOOL "Enable wayland support")
add_subdirectory(bgfx.cmake)

if(TARGET example-common)
    set_target_properties(example-common PROPERTIES EXCLUDE_FROM_ALL TRUE)
    message(WARNING "Force-disabled example-common despite config")
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable shared libs")
add_subdirectory(SDL)

# GLM (built from source)
set(GLM_BUILD_TESTS OFF CACHE BOOL "Build the GLM test programs")
set(GLM_BUILD_SAMPLES OFF CACHE BOOL "Build the GLM sample programs")
add_subdirectory(glm)


set(IMGUI_WITH_BACKEND OFF CACHE BOOL "")
set(IMGUI_BACKEND_DX9 OFF CACHE BOOL "")
set(IMGUI_BACKEND_DX10 OFF CACHE BOOL "")
set(IMGUI_BACKEND_DX11 OFF CACHE BOOL "")
set(IMGUI_BACKEND_DX12 OFF CACHE BOOL "")
add_subdirectory(imgui.cmake)
