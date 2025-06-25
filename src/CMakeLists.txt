# Find all source files (c++, headers and shaders)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h" "*.hpp" "*.sc")
file(GLOB PUBLIC_HEADERS CONFIGURE_DEPENDS "include/*.h")

# Create library
add_library(EngineLib STATIC ${SOURCES} ${PUBLIC_HEADERS})

target_include_directories(EngineLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)



# Set include directories
target_include_directories(EngineLib PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<TARGET_PROPERTY:bgfx,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bx,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bimg,INTERFACE_INCLUDE_DIRECTORIES>
    # $<TARGET_PROPERTY:SDL3,INTERFACE_INCLUDE_DIRECTORIES>
)
#wayland fuckery
if(UNIX AND NOT APPLE)
  find_package(ECM REQUIRED)
  set(CMAKE_MODULE_PATH ${ECM_FIND_MODULE_DIR})
  include(${ECM_FIND_MODULE_DIR}/FindWayland.cmake)
  find_package(Wayland REQUIRED)
  target_link_libraries(EngineLib PRIVATE Wayland::Client)
endif()

# Link dependencies
target_link_libraries(EngineLib PUBLIC 
    bgfx
    bx
    bimg
    SDL3
    glm
)



get_target_property(BGFX_INCLUDES bgfx INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(EngineLib PUBLIC ${BGFX_INCLUDES})

bgfx_compile_shaders(
  TYPE VERTEX
  SHADERS rendering/v_simple.sc
  VARYING_DEF varying.def.sc
  OUTPUT_DIR ${CMAKE_BINARY_DIR}
  # AS_HEADERS
)
bgfx_compile_shaders(
  TYPE FRAGMENT
  SHADERS f_simple.sc
  VARYING_DEF ${CMAKE_SOURCE_DIR}/varying.def.sc
  OUTPUT_DIR ${CMAKE_BINARY_DIR}
  # AS_HEADERS
)

target_include_directories(EngineLib PRIVATE ${CMAKE_BINARY_DIR})


# Organize files in IDE
source_group("Source Files" FILES ${SOURCES})
source_group("Public Headers" FILES ${PUBLIC_HEADERS})

# Set output directory
set_target_properties(EngineLib PROPERTIES 
    PUBLIC_HEADER "include/*.h"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
