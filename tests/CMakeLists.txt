# Headless contact-solver checks. No window or renderer needed at runtime.
add_executable(TestContacts test_contacts.cpp)

target_include_directories(TestContacts PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/src/ecs
)

target_link_libraries(TestContacts PRIVATE EngineLib)

set_target_properties(TestContacts PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
