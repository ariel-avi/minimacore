enable_testing()
include(FetchContent)

fetchcontent_declare(
  GTest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571
  FIND_PACKAGE_ARGS
)
fetchcontent_makeavailable(GTest)

if (GTest_FOUND)
  include(GoogleTest)
  set(_TESTS_TARGET ${PROJECT_NAME}_tests)

  add_executable(${_TESTS_TARGET}
    src/genetic_algorithm_tests.cpp
    src/benchmark_function_tests.cpp src/utils_tests.cpp)

  target_link_libraries(${_TESTS_TARGET} GTest::gtest_main GTest::gtest Eigen3::Eigen minimacore::genetic_algorithm)

  gtest_discover_tests(${_TESTS_TARGET})

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