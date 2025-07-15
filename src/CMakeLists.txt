# Find all source files (c++, headers and shaders)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h" "*.hpp" "*.sc")
file(GLOB PUBLIC_HEADERS CONFIGURE_DEPENDS "include/*.h")

# Create library
add_library(EngineLib STATIC ${SOURCES} ${PUBLIC_HEADERS})




target_include_directories(EngineLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)



target_include_directories(EngineLib PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<TARGET_PROPERTY:bgfx,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bx,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bimg,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:imgui,INTERFACE_INCLUDE_DIRECTORIES>
    # $<TARGET_PROPERTY:imgui_headers,INTERFACE_INCLUDE_DIRECTORIES>
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
    SDL3::SDL3
    glm
    imgui
)




get_target_property(BGFX_INCLUDES bgfx INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(EngineLib PUBLIC ${BGFX_INCLUDES})

bgfx_compile_shaders(
  TYPE VERTEX
  SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/v_simple.sc
  VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/varying.def.sc
  OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/generated
  AS_HEADERS
)
bgfx_compile_shaders(
  TYPE FRAGMENT
  SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/f_simple.sc
  VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/varying.def.sc
  OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/generated
  AS_HEADERS
)

bgfx_compile_shaders(
  TYPE VERTEX
  SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/v_lighting.sc
  VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/varying.def.sc
  OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/generated
  AS_HEADERS
)
bgfx_compile_shaders(
  TYPE FRAGMENT
  SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/f_lighting.sc
  VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/varying.def.sc
  OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/generated
  AS_HEADERS
)


# Fix MinGW locale issues for BGFX
if(WIN32)
    target_compile_definitions(bgfx PRIVATE 
        _LOCALE_T_DEFINED
        locale_t=_locale_t
        LC_CTYPE_MASK=LC_CTYPE
        newlocale=_create_locale
        strtof_l=_strtof_l
        freelocale=_free_locale
    )
    
    # Apply same fixes to GLSL optimizer if it's a separate target
    if(TARGET glsl-optimizer)
        target_compile_definitions(glsl-optimizer PRIVATE 
            _LOCALE_T_DEFINED
            locale_t=_locale_t
            LC_CTYPE_MASK=LC_CTYPE
            newlocale=_create_locale
            strtof_l=_strtof_l
            freelocale=_free_locale
        )
    endif()
endif()


# Organize files in IDE
source_group("Source Files" FILES ${SOURCES})
source_group("Public Headers" FILES ${PUBLIC_HEADERS})

# Set output directory
set_target_properties(EngineLib PROPERTIES 
    PUBLIC_HEADER "include/*.h"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
