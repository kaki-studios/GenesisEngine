cmake_minimum_required(VERSION 3.15)
project(Engine3D LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders for IDE organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)



# Wayland detection and prioritization
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    
    # Check if we're in a Wayland session
    execute_process(
        # COMMAND sh -c "[ -n \"$WAYLAND_DISPLAY\" ] && echo 1 || echo 0"
        COMMAND sh -c "echo 0"
        OUTPUT_VARIABLE IS_WAYLAND_SESSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    if(IS_WAYLAND_SESSION EQUAL "1" OR FORCE_WAYLAND)
  find_package(ECM REQUIRED)
  set(CMAKE_MODULE_PATH ${ECM_FIND_MODULE_DIR})
  include(${ECM_FIND_MODULE_DIR}/FindWayland.cmake)
  find_package(Wayland REQUIRED)

        message(STATUS "Building with Wayland support")
        set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
        # set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)
        add_definitions(-D__WAYLAND__=1)
    else()
        message(STATUS "Building with X11 support")
        set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
        add_definitions(-D__X11__=1)
    endif()
endif()

# Add subdirectories
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(demo)
