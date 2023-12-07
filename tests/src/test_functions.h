
#ifndef MINIMACORE_TEST_FUNCTIONS_H
#define MINIMACORE_TEST_FUNCTIONS_H

#include <concepts>
#include <type_traits>
#include <Eigen/Dense>

template<typename T>
concept floating_point_type = std::is_floating_point_v<T>;

template<floating_point_type T>
inline T square(T value)
{
  return value * value;
}


template<floating_point_type T, std::size_t Dim>
inline T rastrigin(const Eigen::VectorBlock<T, Dim>& input)
{
  T a = 10.;
  static_assert(Dim > 0);
  T result = a * Dim;
  for (std::size_t i = 0; i < Dim; i++) result += (square(input(i)) - a * std::cos(2. * EIGEN_PI * input(i)));
  return result;
}

template<floating_point_type T>
inline T auckley(const Eigen::VectorBlock<T, 2>& input)
{
  T x = input(0), y = input(1);
  return -20 * std::exp(-0.2 * std::sqrt(0.2 * (square(x) + square(y))))
         - std::exp(0.5 * (std::cos(2. * EIGEN_PI * x) + std::cos(2. * M_PI_2 * y))) + std::exp<T> + 20.;
}

template<floating_point_type T, std::size_t Dim>
inline T sphere(const Eigen::VectorBlock<T, Dim>& input)
{
  T result = 0.;
  for (std::size_t i = 0; i < Dim; i++) result += square(input(i));
  return result;
}

template<floating_point_type T, std::size_t Dim>
inline T rosenbrock(const Eigen::VectorBlock<T, Dim>& input)
{
  static_assert(Dim > 2);
  T result = 0;
  for (std::size_t i = 0; i < (Dim - 1); i++)
    result += 100 * square(input(i + 1) - square(input(i))) +
              square(1 - square(input(i)));
  return result;
}

#endif //MINIMACORE_TEST_FUNCTIONS_H
