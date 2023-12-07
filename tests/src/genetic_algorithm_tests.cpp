
#include <gtest/gtest.h>
#include <ranges>
#include <genetic_algorithm_base.h>

using namespace minimacore::genetic_algorithm;

template<floating_point_type F>
class minimacore_genetic_algorithm_tests : public ::testing::Test {
protected:
  
  using individual_impl = base_individual<F>;
  
  void SetUp() override
  {
    _unique_sorted_ranks = this->_ranks;
    std::sort(_unique_sorted_ranks.begin(), _unique_sorted_ranks.end());
    auto end = std::unique(_unique_sorted_ranks.begin(), _unique_sorted_ranks.end());
    _unique_sorted_ranks.erase(end, _unique_sorted_ranks.end());
    _unique_sorted_ranks.shrink_to_fit();
    for (size_t i = 0; i < 10; i++) {
      auto& ind = _population.emplace_back(std::make_shared<individual_impl>(Eigen::VectorX<F>(3), 2));
      ind->set_objective_fitness(0, _fitness_values[0][i]);
      ind->set_objective_fitness(1, _fitness_values[1][i]);
    }
  }
  
  void test_ranked_selection_for_reproduction_by_ranks(size_t rank_count)
  {
    reproduction_selection_t<F> test_set;
    for (auto& rank_i : _unique_sorted_ranks) {
      if (rank_i < rank_count) {
        for (size_t i{0}; i < _ranks.size(); i++) {
          if (_ranks[i] == rank_i) test_set.push_back(_population[i]);
        }
      }
    }
    
    ranked_selection_for_reproduction<F> selection_for_reproduction(
        rank_count, ranked_selection::select_by_ranks);
    reproduction_selection_t<F> top_rank = selection_for_reproduction(_population);
    ASSERT_EQ(top_rank.size(), test_set.size()) << "Rank count: " << rank_count;
    
    std::ranges::for_each(top_rank,
                          [test_set](auto& individual) {
                            EXPECT_TRUE(std::find(test_set.begin(), test_set.end(), individual) != test_set.end())
                                      << "Couldn't find " << individual << " in test set.";
                          });
  }
  
  void test_ranked_selection_for_replacement_by_ranks(size_t rank_count)
  {
    reproduction_selection_t<F> test_set;
    size_t count{0};
    for (auto& rank_i : std::ranges::reverse_view(_unique_sorted_ranks)) {
      if (count < rank_count) {
        for (size_t i{_ranks.size()}; i != 0; i--) {
          if (_ranks[i - 1] == rank_i) test_set.push_back(_population[i - 1]);
        }
        count++;
      }
    }
    ranked_selection_for_replacement<F> selection(
        rank_count, ranked_selection::select_by_ranks);
    selection(_population);
    ASSERT_EQ(_population.size() + test_set.size(), _ranks.size())
                  << "Rank count: " << rank_count
                  << "\nTest Size: " << test_set.size()
                  << "\nPopulation size: " << _population.size();
    
    std::ranges::for_each(test_set,
                          [&](auto& individual) {
                            EXPECT_TRUE(
                                std::find(_population.begin(), _population.end(), individual) == _population.end())
                                      << "Found " << individual << " in population.";
                          });
  }
  
  void test_ranked_selection_for_reproduction_by_individuals(size_t individual_count)
  {
    reproduction_selection_t<F> test_set;
    for (auto& rank_i : _unique_sorted_ranks) {
      if (rank_i < individual_count) {
        for (size_t i{0}; i < _ranks.size(); i++) {
          if (_ranks[i] == rank_i) test_set.push_back(_population[i]);
        }
      }
    }
    
    ranked_selection_for_reproduction<F> selection_for_reproduction(
        individual_count, ranked_selection::select_by_individuals);
    reproduction_selection_t<F> top_rank = selection_for_reproduction(_population);
    ASSERT_NE(top_rank.size(), test_set.size());
    
    std::ranges::for_each(top_rank,
                          [test_set](auto& individual) {
                            EXPECT_TRUE(std::find(test_set.begin(), test_set.end(), individual) != test_set.end())
                                      << "Couldn't find " << individual << " in test set.";
                          });
  }
  
  void test_ranked_selection_for_replacement_by_individuals(size_t individual_count)
  {
    ranked_selection_for_replacement<F> selection(
        individual_count, ranked_selection::select_by_individuals);
    selection(_population);
    ASSERT_EQ(_population.size(), 10 - individual_count);
  }
  
  population_t<F> _population;
  
  vector<size_t> _unique_sorted_ranks;
  
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
  vector<size_t> _ranks{
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
  ASSERT_EQ(selected_individuals.size(), selection_size);
  for (size_t i = selection_size; i < this->_population.size(); i++) {
    EXPECT_TRUE(std::all_of(selected_individuals.begin(), selected_individuals.end(), [&](auto& individual) {
      return individual->overall_fitness() < this->_population[i]->overall_fitness();
    }));
  }
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

TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_reproduction_by_ranks)
{
  this->test_ranked_selection_for_reproduction_by_ranks(1);
  this->test_ranked_selection_for_reproduction_by_ranks(2);
  this->test_ranked_selection_for_reproduction_by_ranks(3);
  this->test_ranked_selection_for_reproduction_by_ranks(4);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_reproduction_by_individuals)
{
  this->test_ranked_selection_for_reproduction_by_individuals(2);
  this->test_ranked_selection_for_reproduction_by_individuals(4);
  this->test_ranked_selection_for_reproduction_by_individuals(7);
  this->test_ranked_selection_for_reproduction_by_individuals(9);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, generational_selection_for_replacement)
{
  generational_selection_for_replacement<TypeParam> replacement;
  replacement(this->_population);
  ASSERT_TRUE(this->_population.empty());
}

TYPED_TEST(minimacore_genetic_algorithm_tests, truncation_selection_for_replacement)
{
  truncation_selection_for_replacement<TypeParam> replacement(5);
  replacement(this->_population);
  ASSERT_EQ(this->_population.size(), 5);
}

/**
 * @brief The following tests must be crated separately because they modify the population, so the population needs to
 * be initialized again after every test.
 */
TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_replacement_by_ranks_1)
{
  this->test_ranked_selection_for_replacement_by_ranks(1);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_replacement_by_ranks_2)
{
  this->test_ranked_selection_for_replacement_by_ranks(2);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_replacement_by_ranks_3)
{
  this->test_ranked_selection_for_replacement_by_ranks(3);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_replacement_by_ranks_4)
{
  this->test_ranked_selection_for_replacement_by_ranks(4);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_replacement_by_individuals_1)
{
  this->test_ranked_selection_for_replacement_by_individuals(1);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_replacement_by_individuals_2)
{
  this->test_ranked_selection_for_replacement_by_individuals(2);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_replacement_by_individuals_4)
{
  this->test_ranked_selection_for_replacement_by_individuals(4);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_replacement_by_individuals_8)
{
  this->test_ranked_selection_for_replacement_by_individuals(8);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, ranked_selection_for_replacement_by_individuals_10)
{
  this->test_ranked_selection_for_replacement_by_individuals(10);
}

