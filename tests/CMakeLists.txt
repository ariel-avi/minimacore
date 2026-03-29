find_package(GTest)

if (GTest_FOUND)
    include(CTest)
    set(CTEST_OUTPUT_ON_FAILURE ON)
    include(GoogleTest)
    find_package(Eigen3 REQUIRED)
    set(_TESTS_TARGET ${PROJECT_NAME}_tests)
    set(_TESTS_SRC
            ${CMAKE_CURRENT_SOURCE_DIR}/src/genetic_algorithm_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmark_function_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/utils_tests.cpp
    )
    add_executable(${_TESTS_TARGET}
            ${_TESTS_SRC}
    )

    target_link_libraries(${_TESTS_TARGET} GTest::gtest_main GTest::gtest Eigen3::Eigen minimacore::genetic_algorithm)
    add_clang_format_for(${_TESTS_TARGET} ${_TESTS_SRC})
    add_clang_tidy_for(${_TESTS_TARGET} ${_TESTS_SRC})

    gtest_discover_tests(${_TESTS_TARGET}
            DISCOVERY_TIMEOUT 30
    )

    if (UNIX AND "${CMAKE_BUILD_TYPE}" MATCHES "^Debug")
        include(CodeCoverage OPTIONAL)
        setup_target_for_coverage_lcov(NAME ${PROJECT_NAME}_coverage
                EXECUTABLE ${PROJECT_NAME}_tests
                EXCLUDE "/usr/*" "include/*")
    elseif (UNIX AND "${CMAKE_BUILD_TYPE}" MATCHES "^Coverage")
        set(CMAKE_CXX_FLAGS "-O0 --coverage -fno-inline -fno-inline-small-functions -fno-default-inline -fprofile-arcs -ftest-coverage")
    elseif (WIN32)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
            message(STATUS "Compiling with flags: ${CMAKE_CXX_FLAGS}")
        endif ()
    endif ()

else ()
    message(WARNING "GTest not found. Not compiling tests.")
endif ()