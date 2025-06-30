cmake_minimum_required(VERSION 3.15)
project(Engine3D LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders for IDE organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose build type" FORCE)
endif()

# Force all subprojects to use the same build type
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Build type" FORCE)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  # For bgfx.cmake specifically
  set(BGFX_CONFIG_DEBUG OFF)
  set(NDEBUG ON)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(NDEBUG OFF)
endif()


# Linux display server support - build with both X11 and Wayland for runtime detection
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(ECM REQUIRED)
    set(CMAKE_MODULE_PATH ${ECM_FIND_MODULE_DIR})
    include(${ECM_FIND_MODULE_DIR}/FindWayland.cmake)
    find_package(Wayland REQUIRED)

    message(STATUS "Building with both X11 and Wayland support for runtime detection")
    #test
    # set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
    # set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
endif()

# Add subdirectories
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(demo)
