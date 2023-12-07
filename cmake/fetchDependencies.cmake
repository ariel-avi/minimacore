include(FetchContent)
fetchcontent_declare(Eigen3
        GIT_REPO https://gitlab.com/libeigen/eigen.git
        GIT_TAG e7248b26a1ed53fa030c5c459f7ea095dfd276ac
        FIND_PACKAGE_ARGS)
fetchcontent_makeavailable(Eigen3)