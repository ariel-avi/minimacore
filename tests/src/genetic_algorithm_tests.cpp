
#include <gtest/gtest.h>
#include "genetic_algorithm_base.h"

using namespace minimacore::genetic_algorithm;

struct mock_data {
  [[nodiscard]] mock_data* clone() const
  {
    return new mock_data(*this);
  }
};

template<floating_point_type F>
class minimacore_genetic_algorithm_tests : public ::testing::Test {
protected:
  
  using individual_impl = individual<F>;
  
  void SetUp() override
  {
    for (size_t i = 0; i < 10; i++) {
      auto& ind = _population.emplace_back(std::make_unique<individual_impl>(Eigen::VectorX<F>::Ones(3)));
      ind->append_fitness_value(_fitness_values[i]);
    }
  }
  
  vector<individual_ptr<F>> _population;
  
  vector<F> _fitness_values{
      {
          1.,  // 4
          1.2,
          0.2, // 1
          0.3, // 2
          2.,
          3.,
          2.3,
          0.4, // 3
          1.1, // 5
          2.1
      }
  };
  
};

using floating_point_types = ::testing::Types<long double, double, float>;
TYPED_TEST_SUITE(minimacore_genetic_algorithm_tests, floating_point_types);

TYPED_TEST(minimacore_genetic_algorithm_tests, truncation_selection_for_reproduction)
{
  size_t selection_size = 5;
  truncation_selection_for_reproduction<TypeParam> selection(selection_size);
  auto selected_individuals = selection(this->_population);
  vector<TypeParam> cp(this->_fitness_values);
  std::sort(cp.begin(), cp.end());
  for (size_t i = 0; i < selection_size; i++) EXPECT_EQ(selected_individuals[i]->overall_fitness(), cp[i]);
}