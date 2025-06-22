# Find all source files
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.h")

# Create engine library
add_library(EngineLib STATIC ${ENGINE_SOURCES})

# Organize files in IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SOURCES})

# Include directories
target_include_directories(EngineLib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(EngineLib PRIVATE
    bgfx
    glfw
    glm
)

# Set output directory
set_target_properties(EngineLib PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
