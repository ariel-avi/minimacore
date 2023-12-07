cmake_minimum_required(VERSION 3.25)
include(cmake/projectVersion.cmake)
project(minimacore
        LANGUAGES C CXX
        VERSION ${minimacore_VERSION}
)
set(CMAKE_CXX_STANDARD 20)
include(cmake/fetchDependencies.cmake)
add_subdirectory(src)
add_subdirectory(tests)