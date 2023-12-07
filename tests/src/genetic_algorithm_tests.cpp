
#include <gtest/gtest.h>
#include <source_location>
#include "genetic_algorithm_base.h"

using namespace minimacore::genetic_algorithm;

template<floating_point_type F>
class minimacore_genetic_algorithm_tests : public ::testing::Test {
protected:
  
  using individual_impl = individual<F>;
  
  void SetUp() override
  {
    for (size_t i = 0; i < 10; i++) {
      auto& ind = _population.emplace_back(std::make_unique<individual_impl>(Eigen::VectorX<F>(3)));
      ind->append_fitness_value(_fitness_values[0][i]);
      ind->append_fitness_value(_fitness_values[1][i]);
    }
  }
  
  vector<individual_ptr<F>> _population;
  
  vector<vector<F>> _fitness_values{
      {
          {
              1.,
              1.2,
              0.2,
              0.3,
              1.4,
              3.,
              2.3,
              0.4,
              1.1,
              2.1
          },
          {
              0.6,
              1.3,
              0.5,
              0.4,
              0.2,
              1.,
              0.3,
              1.4,
              1.2,
              0.9
          },
      }
  };
  
  vector<size_t> ranks{
      {
          1,
          3,
          0,
          0,
          0,
          3,
          1,
          1,
          2,
          2
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
  // TODO: write comprehensive tests for this
}

TYPED_TEST(minimacore_genetic_algorithm_tests, tournament_selection_for_reproduction)
{
  size_t selection_size = 5;
  size_t tournament_size = 3;
  tournament_selection_for_reproduction<TypeParam> selection(tournament_size, selection_size);
  vector selected_individuals = selection(this->_population);
  ASSERT_EQ(selected_individuals.size(), selection_size);
  for (auto& i : selected_individuals) {
    ASSERT_NE(i, this->_population[5].get());
  }
}

TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_reproduction)
{
  // todo
}