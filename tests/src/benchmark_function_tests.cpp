
#include <gtest/gtest.h>
#include <concepts>
#include <Eigen/Dense>
#include "test_functions.h"

template<floating_point_type T>
class benchmark_functions_test : public ::testing::Test {
public:
};

using floating_point_types = ::testing::Types<long double, double, float>;
TYPED_TEST_SUITE(benchmark_functions_test, floating_point_types);

TYPED_TEST(benchmark_functions_test, rastrigin2)
{
  ASSERT_NEAR(rastrigin<TypeParam>(Eigen::Vector<TypeParam, 2>({0., 0.})), 0., 1E-8);
}

TYPED_TEST(benchmark_functions_test, rastrigin3)
{
  ASSERT_NEAR(rastrigin<TypeParam>(Eigen::Vector<TypeParam, 3>({0., 0., 0.})), 0., 1E-8);
}

TYPED_TEST(benchmark_functions_test, sphere2)
{
  ASSERT_NEAR(sphere<TypeParam>(Eigen::Vector<TypeParam, 2>({0., 0.})), 0., 1E-8);
}

TYPED_TEST(benchmark_functions_test, sphere3)
{
  ASSERT_NEAR(sphere<TypeParam>(Eigen::Vector<TypeParam, 3>({0., 0., 0.})), 0., 1E-8);
}

TYPED_TEST(benchmark_functions_test, rosenbrock3)
{
  ASSERT_NEAR(rosenbrock<TypeParam>(Eigen::Vector<TypeParam, 3>({1., 1., 1.})), 0., 1E-8);
}
