
#include <gtest/gtest.h>
#include <concepts>
#include <Eigen/Dense>
#include "test_functions.h"

template<floating_point_type T, std::size_t Dim = 0UL>
struct io {
  T (* function)(const Eigen::Vector<T, Dim>&);
  
  Eigen::Vector<T, Dim> input;
  T expected;
};

template<floating_point_type T, std::size_t Dim = 0UL>
class benchmark_functions_test : public ::testing::TestWithParam<io<T, Dim>> {
protected:
};

using mytypes = ::testing::Types<double>;
typedef benchmark_functions_test<double, 2> rastrigin_tests;

TEST_P(rastrigin_tests, _double)
{
  ASSERT_NEAR(GetParam().function(GetParam().input), GetParam().expected, 1E-8);
}

INSTANTIATE_TEST_SUITE_P(value_tests, rastrigin_tests,
                         ::testing::Values(
                             io<double, 2>(&rastrigin<double, 2>,
                                           Eigen::Vector<double, 2>({0., 0.}),
                                           0.),
                             io<double, 2>(&sphere<double, 2>,
                                           Eigen::Vector<double, 2>({0., 0.}),
                                           0.)
                         )
);

typedef benchmark_functions_test<double, 3> tests3;

TEST_P(tests3, _double)
{
  ASSERT_NEAR(GetParam().function(GetParam().input), GetParam().expected, 1E-8);
}

INSTANTIATE_TEST_SUITE_P(value_tests, tests3,
                         ::testing::Values(
                             io<double, 3>(&rastrigin<double, 3>,
                                           Eigen::Vector<double, 3>({0., 0., 0.}),
                                           0.),
                             io<double, 3>(&sphere<double, 3>,
                                           Eigen::Vector<double, 3>({0., 0., 0.}),
                                           0.),
                             io<double, 3>(&rosenbrock<double, 3>,
                                           Eigen::Vector<double, 3>({1., 1., 1.}),
                                           0.)
                         )
);
