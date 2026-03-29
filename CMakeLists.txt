cmake_minimum_required(VERSION 3.25)
include(cmake/projectVersion.cmake)
project(minimacore
        LANGUAGES C CXX
        VERSION ${minimacore_VERSION}
)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Map custom build types to their Conan-installed counterparts so that
# imported targets (Eigen, GTest, …) resolve their include dirs and libs.
# Conan only installs Debug/Release; Coverage is built on top of Debug.
set(CMAKE_MAP_IMPORTED_CONFIG_COVERAGE Debug)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(ClangFormat)
include(ClangTidy)
add_subdirectory(src)
add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)
