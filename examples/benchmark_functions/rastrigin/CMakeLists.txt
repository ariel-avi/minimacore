add_executable(minimacore_example_rastrigin src/rastrigin.cpp)
target_link_libraries(minimacore_example_rastrigin PRIVATE minimacore::genetic_algorithm)
set_target_properties(minimacore_example_rastrigin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)