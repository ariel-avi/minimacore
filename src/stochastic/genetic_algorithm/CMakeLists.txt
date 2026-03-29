set(GA_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/runner.h
        ${CMAKE_CURRENT_SOURCE_DIR}/base_individual.h
        ${CMAKE_CURRENT_SOURCE_DIR}/selection_operators.h
)
find_package(Eigen3 REQUIRED)
add_library(minimacore_genetic_algorithm INTERFACE ${GA_HEADERS})
add_library(minimacore::genetic_algorithm ALIAS minimacore_genetic_algorithm)
target_include_directories(minimacore_genetic_algorithm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(minimacore_genetic_algorithm INTERFACE Eigen3::Eigen minimacore::base_concepts minimacore::utils)
add_clang_format_for(minimacore_genetic_algorithm ${GA_HEADERS})
add_clang_tidy_for(minimacore_genetic_algorithm ${GA_HEADERS})
