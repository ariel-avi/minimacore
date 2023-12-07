
#include <gtest/gtest.h>
#include <ranges>
#include "test_functions.h"
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
      auto& ind = _population.emplace_back(
          std::make_shared<individual_impl>(Eigen::VectorX<F>::Random(3), _functions.size()));
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
    reproduction_selection_t<F> selected = selection_for_reproduction(_population);
    ASSERT_EQ(selected.size(), test_set.size()) << "Rank count: " << rank_count;
    
    std::ranges::for_each(selected,
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
  
  vector<F (*)(const Eigen::VectorX<F>&)> _functions{{&rastrigin, &rosenbrock}};
  
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

TYPED_TEST(minimacore_genetic_algorithm_tests, is_dominant)
{
  EXPECT_FALSE(ranked_selection::is_dominant(this->_population[0], this->_population));
  EXPECT_FALSE(ranked_selection::is_dominant(this->_population[1], this->_population));
  EXPECT_TRUE(ranked_selection::is_dominant(this->_population[2], this->_population));
  EXPECT_TRUE(ranked_selection::is_dominant(this->_population[3], this->_population));
  EXPECT_TRUE(ranked_selection::is_dominant(this->_population[4], this->_population));
  EXPECT_FALSE(ranked_selection::is_dominant(this->_population[5], this->_population));
  EXPECT_FALSE(ranked_selection::is_dominant(this->_population[6], this->_population));
  EXPECT_FALSE(ranked_selection::is_dominant(this->_population[7], this->_population));
  EXPECT_FALSE(ranked_selection::is_dominant(this->_population[8], this->_population));
  EXPECT_FALSE(ranked_selection::is_dominant(this->_population[9], this->_population));
}

TYPED_TEST(minimacore_genetic_algorithm_tests, rank_population)
{
  ranked_selection_t<TypeParam> ranks = ranked_selection::rank_population(this->_population);
  ASSERT_EQ(ranks.size(), 4);
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

template<floating_point_type F>
static constexpr F tolerance()
{
  switch (sizeof(F)) {
    case sizeof(float):
      return 1E-4;
    case sizeof(double):
      return 1E-8;
    case sizeof(long double):
      return 1E-16;
  }
  return 1E-6;
}

TYPED_TEST(minimacore_genetic_algorithm_tests, uniform_linear_crossover)
{
  for (size_t i{0}; i < this->_population.size() / 2; i++) {
    const auto& individual_a = this->_population[i];
    const auto& individual_b = this->_population[i + this->_population.size() / 2];
    uniform_linear_crossover<TypeParam> crossover(1.);
    auto genome = crossover(*individual_a, *individual_b);
    auto midpoint = (individual_b->genome() + individual_a->genome()) / 2;
    
    auto genome_diff = genome - midpoint;
    
    auto diff_a = individual_a->genome() - midpoint;
    auto ratio_a = diff_a.cwiseQuotient(genome_diff);
    ASSERT_GE(diff_a.norm(), genome_diff.norm());
    
    for (long j{1}; j < genome.size(); j++) EXPECT_NEAR(ratio_a(j) / ratio_a(0), 1., tolerance<TypeParam>());
    
    auto diff_b = individual_b->genome() - midpoint;
    auto ratio_b = diff_b.cwiseQuotient(genome_diff);
    ASSERT_GE(diff_b.norm(), genome_diff.norm());
    for (long j{1}; j < genome.size(); j++) EXPECT_NEAR(ratio_b(j) / ratio_b(0), 1., tolerance<TypeParam>());
  }
}

/**
 * @brief This test has a very small chance of failing.
 */
TYPED_TEST(minimacore_genetic_algorithm_tests, uniform_voluminal_crossover)
{
  for (size_t i{0}; i < this->_population.size() / 2; i++) {
    const auto& individual_a = this->_population[i];
    const auto& individual_b = this->_population[i + this->_population.size() / 2];
    uniform_voluminal_crossover<TypeParam> crossover(1.);
    auto genome = crossover(*individual_a, *individual_b);
    auto midpoint = (individual_b->genome() + individual_a->genome()) / 2;
    
    auto genome_diff = genome - midpoint;
    
    auto diff_a = individual_a->genome() - midpoint;
    auto ratio_a = diff_a.cwiseQuotient(genome_diff);
    ASSERT_GE(diff_a.norm(), genome_diff.norm());
    for (long j{1}; j < genome.size(); j++)
      EXPECT_TRUE(std::abs(std::abs(ratio_a(j) / ratio_a(0)) - 1.) > 1E-5)
                << "ratio_a(" << j << "): " << ratio_a(j)
                << "\nratio_a(0): " << ratio_a(0);
    
    auto diff_b = individual_b->genome() - midpoint;
    auto ratio_b = diff_b.cwiseQuotient(genome_diff);
    ASSERT_GE(diff_b.norm(), genome_diff.norm());
    for (long j{1}; j < genome.size(); j++)
      EXPECT_TRUE(std::abs(std::abs(ratio_b(j) / ratio_b(0)) - 1.) > 1E-5)
                << "ratio_b(" << j << "): " << ratio_b(j)
                << "\nratio_b(0): " << ratio_b(0);
  }
}

TYPED_TEST(minimacore_genetic_algorithm_tests, gaussian_mutation)
{
  const size_t repetitions{1'000};
  for (auto& individual : this->_population) {
    gaussian_mutation<TypeParam> mutation(0.05, 1E-2);
    genome_t<TypeParam> genome = genome_t<TypeParam>::Zero(individual->genome().size());
    for (size_t i{0}; i < repetitions; i++) genome += mutation(*individual);
    genome /= TypeParam(repetitions);
    EXPECT_TRUE(genome.isApprox(individual->genome(), 1E-2))
              << genome.transpose() << '\n' << individual->genome().transpose();
  }
}

TYPED_TEST(minimacore_genetic_algorithm_tests, uniform_mutation)
{
  vector<TypeParam> factors{
      {1., 2., 3., 4., 5., 6.}
  };
  for (auto& factor : factors) {
    for (auto& individual : this->_population) {
      uniform_mutation<TypeParam> mutation(0.05, factor);
      genome_t<TypeParam> genome = mutation(*individual);
      auto diff = individual->genome() - genome;
      for (size_t i{0}; i < genome.size(); i++) EXPECT_LE(diff(i), factor);
    }
  }
}

template<floating_point_type F>
class genome_generator : public base_genome_generator<F> {
public:
  void operator()(const individual_ptr<F>& individual) const override
  {
    std::random_device device;
    std::mt19937_64 generator(device());
    std::uniform_real_distribution<F> dist(lower_limit, upper_limit);
    for (auto i{0}; i < individual->genome().size(); i++) individual->genome()(i) = dist(generator);
  }
  
  genome_generator(F lower_limit, F upper_limit)
      : lower_limit(lower_limit), upper_limit(upper_limit)
  {}

private:
  F lower_limit, upper_limit;
};

TYPED_TEST(minimacore_genetic_algorithm_tests, population_generator)
{
  auto& population = this->_population;
  population_generator<TypeParam> generator;
  generator.append_genome_generator(std::make_unique<genome_generator<TypeParam>>(-5.28, 5.28));
  vector<Eigen::VectorX<TypeParam>> original_genomes;
  for (auto& individual : population) original_genomes.push_back(individual->genome());
  ASSERT_EQ(original_genomes.size(), population.size());
  generator(population);
  for (size_t i{0UL}; i < original_genomes.size(); i++) {
    auto& genome = population[i]->genome();
    EXPECT_FALSE(original_genomes[i].isApprox(genome));
    for (auto j{0}; j < genome.size(); j++) {
      EXPECT_GE(genome(j), -5.28);
      EXPECT_LE(genome(j), 5.28);
    }
  }
}

template<floating_point_type F>
class benchmark_function_evaluation : base_evaluation<F> {
public:
  size_t operator()(base_individual<F>& individual, size_t objective_index) const override
  {
    for (auto& f : _f_ptr) {
      individual.set_objective_fitness(objective_index, std::abs(f(individual.genome())));
      objective_index++;
    }
    return objective_index;
  }
  
  explicit benchmark_function_evaluation(const vector<F (*)(const Eigen::VectorX<F>&)>& f_ptr) : _f_ptr(f_ptr)
  {}

private:
  vector<F (*)(const Eigen::VectorX<F>&)> _f_ptr;
};

TYPED_TEST(minimacore_genetic_algorithm_tests, benchmark_function_evaluation)
{
  benchmark_function_evaluation<TypeParam> evaluation(this->_functions);
  for (auto& individual : this->_population) {
    EXPECT_EQ(evaluation(*individual, 0), this->_functions.size());
    EXPECT_FALSE(individual->overall_fitness() < 1E-6);
  }
}

