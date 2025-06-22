cmake_minimum_required(VERSION 3.15)
project(Engine3D LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders for IDE organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Add subdirectories
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(demo)
