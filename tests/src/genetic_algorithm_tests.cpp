
#include <gtest/gtest.h>
#include <source_location>
#include <ranges>
#include "genetic_algorithm_base.h"

using namespace minimacore::genetic_algorithm;

template<floating_point_type F>
class minimacore_genetic_algorithm_tests : public ::testing::Test {
protected:
  
  using individual_impl = individual<F>;
  
  void SetUp() override
  {
    unique_sorted_ranks = this->ranks;
    std::sort(unique_sorted_ranks.begin(), unique_sorted_ranks.end());
    auto end = std::unique(unique_sorted_ranks.begin(), unique_sorted_ranks.end());
    unique_sorted_ranks.erase(end, unique_sorted_ranks.end());
    unique_sorted_ranks.shrink_to_fit();
    for (size_t i = 0; i < 10; i++) {
      auto& ind = _population.emplace_back(std::make_shared<individual_impl>(Eigen::VectorX<F>(3), 2));
      ind->set_fitness_value(0, _fitness_values[0][i]);
      ind->set_fitness_value(1, _fitness_values[1][i]);
    }
  }
  
  void test_ranked_selection_by_rank(size_t rank_count)
  {
    reproduction_selection_t<F> test_set;
    for (auto& rank_i : unique_sorted_ranks) {
      if (rank_i < rank_count) {
        for (size_t i{0}; i < ranks.size(); i++) {
          if (ranks[i] == rank_i) test_set.push_back(_population[i]);
        }
      }
    }
    
    ranked_selection_for_reproduction<F> selection_for_reproduction(
        rank_count, ranked_selection_for_reproduction<F>::select_by_ranks);
    reproduction_selection_t<F> top_rank = selection_for_reproduction(_population);
    ASSERT_EQ(top_rank.size(), test_set.size()) << "Rank count: " << rank_count;
    
    std::ranges::for_each(top_rank,
                          [test_set](auto& individual) {
                            EXPECT_TRUE(std::find(test_set.begin(), test_set.end(), individual) != test_set.end())
                                      << "Couldn't find " << individual << " in test set.";
                          });
  }
  
  population_t<F> _population;
  
  // Two objectives (fitness values) for each individual in the population, to constitute multi-objectiveness
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
  
  // These are the ranks of the objective functions defined above - taken manually for check against unit test
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
  
  vector<size_t> unique_sorted_ranks;
  
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
  for (auto& i : selected_individuals) ASSERT_NE(i, this->_population[5]);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_reproduction)
{
  this->test_ranked_selection_by_rank(1);
  this->test_ranked_selection_by_rank(2);
  this->test_ranked_selection_by_rank(3);
  this->test_ranked_selection_by_rank(4);
}
