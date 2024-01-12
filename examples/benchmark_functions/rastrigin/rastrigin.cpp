
#include <minimacore_concepts.h>
#include <selection_operators.h>
#include <setup.h>
#include <runner.h>


template<floating_point_type T>
inline T rastrigin(const Eigen::VectorX<T>& input)
{
  T a = 10.;
  T result = a * input.size();
  for (std::size_t i = 0; i < input.size(); i++) result += (square(input(i)) - a * std::cos(2. * EIGEN_PI * input(i)));
  return result;
}

int main(int argc, char** argv) {
  return 0;
}