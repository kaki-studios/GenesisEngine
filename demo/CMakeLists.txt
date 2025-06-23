# Find all demo source files
file(GLOB DEMO_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.h")

# Create demo executable
add_executable(EngineDemo ${DEMO_SOURCES})

# Organize files in IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${DEMO_SOURCES})

# Include directories
target_include_directories(EngineDemo PRIVATE
    ${CMAKE_SOURCE_DIR}/src/include
)

# Link dependencies
target_link_libraries(EngineDemo PRIVATE
    EngineLib
)

# Set output directory
set_target_properties(EngineDemo PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
