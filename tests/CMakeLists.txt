enable_testing()
include(FetchContent)

fetchcontent_declare(
        GTest
        GIT_REPO https://github.com/google/googletest.git
        GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571
        FIND_PACKAGE_ARGS
)
fetchcontent_makeavailable(GTest)

if (GTest_FOUND)
    # TODO: Add tests
endif ()