
#ifndef MINIMACORE_TEST_FUNCTIONS_H
#define MINIMACORE_TEST_FUNCTIONS_H

#include <concepts>
#include <type_traits>
#include <Eigen/Dense>
#include <iostream>
#include <minimacore_concepts.h>

template<floating_point_type T>
inline T square(T value)
{
  return value * value;
}


template<floating_point_type T>
inline T rastrigin(const Eigen::VectorX<T>& input)
{
  T a = 10.;
  T result = a * input.size();
  for (std::size_t i = 0; i < input.size(); i++) result += (square(input(i)) - a * std::cos(2. * EIGEN_PI * input(i)));
  return result;
}

template<floating_point_type T>
inline T auckley(const Eigen::Vector<T, 2>& input)
{
  T x = input(0);
  T y = input(1);
  return -20. * std::exp<T>(-0.2 * std::sqrt<T>(0.2 * (square(x) + square(y))))
         - std::exp<T>(0.5 * (std::cos<T>(2. * EIGEN_PI * x) + std::cos<T>(2. * M_PI_2 * y))) + std::exp<T>() + 20.;
}

template<floating_point_type T>
inline T sphere(const Eigen::VectorX<T>& input)
{
  T result = 0.;
  for (size_t i = 0; i < input.size(); i++) result += square(input(i));
  return result;
}

template<floating_point_type T>
inline T rosenbrock(const Eigen::VectorX<T>& input)
{
  T result = 0;
  for (size_t i = 0; i < (input.size() - 1); i++)
    result += 100 * square(input(i + 1) - square(input(i))) +
              square(1 - square(input(i)));
  return result;
}

#endif //MINIMACORE_TEST_FUNCTIONS_H
