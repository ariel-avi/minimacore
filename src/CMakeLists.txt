set(CONCEPTS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/minimacore_concepts.h)
add_library(minimacore_base_concepts INTERFACE)
add_library(minimacore::base_concepts ALIAS minimacore_base_concepts)
target_include_directories(minimacore_base_concepts INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_clang_format_for(minimacore_base_concepts ${CONCEPTS_SRC})
add_clang_tidy_for(minimacore_base_concepts ${CONCEPTS_SRC})

add_subdirectory(utils)
add_subdirectory(stochastic)