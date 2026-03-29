cmake_minimum_required(VERSION 3.25)
include(cmake/projectVersion.cmake)
project(minimacore
        LANGUAGES C CXX
        VERSION ${minimacore_VERSION}
)
set(CMAKE_CXX_STANDARD 23)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(ClangFormat)
include(ClangTidy)
add_subdirectory(src)
add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)
