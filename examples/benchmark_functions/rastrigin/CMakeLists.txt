add_executable(minimacore_example_rastrigin src/rastrigin.cpp)
target_link_libraries(minimacore_example_rastrigin PRIVATE minimacore::genetic_algorithm)
set_target_properties(minimacore_example_rastrigin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_custom_command(TARGET minimacore_example_rastrigin
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/rastrigin_plot_analysis.py
        ${CMAKE_BINARY_DIR}/bin
        COMMENT "Copying Rastrigin Python script to binary directory")