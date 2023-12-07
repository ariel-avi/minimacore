add_library(minimacore_utils SHARED logger.cpp)
add_library(minimacore::utils ALIAS minimacore_utils)
target_include_directories(minimacore_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})