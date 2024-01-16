
#include <gtest/gtest.h>
#include <ranges>
#include "test_functions.h"
#include <runner.h>
#include <future>
#include <atomic>

using namespace minimacore::genetic_algorithm;

template<floating_point_type F>
class chromosome_generator_impl : public base_chromosome_generator<F> {
public:
  void generate_chromosome(const individual_ptr<F>& individual) const override
  {
    std::random_device device;
    std::mt19937_64 generator(device());
    std::uniform_real_distribution<F> dist(lower_limit, upper_limit);
    for (auto i{0}; i < individual->genome().size(); i++) individual->genome()(i) = dist(generator);
  }

  chromosome_generator_impl(F lower_limit, F upper_limit)
          :lower_limit(lower_limit), upper_limit(upper_limit) { }

private:
  F lower_limit;
  F upper_limit;
};

template<floating_point_type F>
class minimacore_genetic_algorithm_tests : public ::testing::Test {
protected:

  using individual_impl = base_individual<F>;

  void SetUp() override
  {
    _genome_generator = std::make_unique<genome_generator<F>>(Eigen::VectorX<F>::Random(3));
    _genome_generator->append_chromosome_generator(std::make_unique<chromosome_generator_impl<F>>(-5.28, 5.28));

    _unique_sorted_ranks = this->_ranks;
    std::sort(_unique_sorted_ranks.begin(), _unique_sorted_ranks.end());
    auto end = std::unique(_unique_sorted_ranks.begin(), _unique_sorted_ranks.end());
    _unique_sorted_ranks.erase(end, _unique_sorted_ranks.end());
    _unique_sorted_ranks.shrink_to_fit();
    for (size_t i = 0; i < 10; i++) {
      auto& ind = _population.emplace_back(
              std::make_shared<individual_impl>(
                      _genome_generator->initial_genome(),
                      _functions.size()));
      ind->set_objective_fitness(0, _fitness_values[0][i]);
      ind->set_objective_fitness(1, _fitness_values[1][i]);
      (*_genome_generator)(ind);
      auto& genome = ind->genome();
      EXPECT_FALSE(_genome_generator->initial_genome().isApprox(genome));
      for (auto j{0}; j < genome.size(); j++) {
        EXPECT_GE(genome(j), -5.28);
        EXPECT_LE(genome(j), 5.28);
      }
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
            rank_count, ranked_selection::select_by_ranks
    );
    reproduction_selection_t<F> selected = selection_for_reproduction(_population);
    ASSERT_EQ(selected.size(), test_set.size()) << "Rank count: " << rank_count;

    std::ranges::for_each(
            selected,
            [test_set](auto& individual) {
              EXPECT_TRUE(std::find(test_set.begin(), test_set.end(), individual) != test_set.end())
                          << "Couldn't find " << individual << " in test add_evaluation.";
            }
    );
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
            rank_count, ranked_selection::select_by_ranks
    );
    selection(_population);
    ASSERT_EQ(_population.size() + test_set.size(), _ranks.size())
                        << "Rank count: " << rank_count
                        << "\nTest Size: " << test_set.size()
                        << "\nPopulation size: " << _population.size();

    std::ranges::for_each(
            test_set,
            [&](auto& individual) {
              EXPECT_TRUE(
                      std::find(_population.begin(), _population.end(), individual) == _population.end())
                          << "Found " << individual << " in population.";
            }
    );
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
            individual_count, ranked_selection::select_by_individuals
    );
    reproduction_selection_t<F> top_rank = selection_for_reproduction(_population);
    ASSERT_NE(top_rank.size(), test_set.size());

    std::ranges::for_each(
            top_rank,
            [test_set](auto& individual) {
              EXPECT_TRUE(std::find(test_set.begin(), test_set.end(), individual) != test_set.end())
                          << "Couldn't find " << individual << " in test add_evaluation.";
            }
    );
  }

  void test_ranked_selection_for_replacement_by_individuals(size_t individual_count)
  {
    ranked_selection_for_replacement<F> selection(
            individual_count, ranked_selection::select_by_individuals
    );
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

  evolution_statistics<F> _statistics{
          2,
          std::initializer_list<int>(
                  {
                          (int) statistics_requests_factory<F>::stat_requests::best_fitness_stat,
                          (int) statistics_requests_factory<F>::stat_requests::average_fitness_stat,
                          (int) statistics_requests_factory<F>::stat_requests::selection_pressure_stat
                  }
          )
  };

  unique_ptr<genome_generator<F>> _genome_generator;

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
    EXPECT_TRUE(std::all_of(
            selected_individuals.begin(), selected_individuals.end(), [&](auto& individual) {
              return individual->overall_fitness() < this->_population[i]->overall_fitness();
            }
    ));
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
  default:
    return 1E-16;
  }
}

TYPED_TEST(minimacore_genetic_algorithm_tests, uniform_linear_crossover)
{
  for (size_t i{0}; i < this->_population.size() / 2; i++) {
    const auto& individual_a = this->_population[i];
    const auto& individual_b = this->_population[i + this->_population.size() / 2];
    uniform_linear_crossover<TypeParam> crossover(1., 2);
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
    uniform_voluminal_crossover<TypeParam> crossover(1., 2);
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
    gaussian_mutation<TypeParam> mutation(0.05, 1E-2, 2);
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
      uniform_mutation<TypeParam> mutation(0.05, factor, 2);
      genome_t<TypeParam> genome = mutation(*individual);
      auto diff = individual->genome() - genome;
      for (size_t i{0}; i < genome.size(); i++) EXPECT_LE(diff(i), factor);
    }
  }
}

TYPED_TEST(minimacore_genetic_algorithm_tests, population_initialization)
{
  auto& population = this->_population;
  auto& initial_genome = this->_genome_generator->initial_genome();
  for (auto& individual : population) {
    (*this->_genome_generator)(individual);
    auto& genome = individual->genome();
    EXPECT_FALSE(initial_genome.isApprox(genome));
    for (auto j{0}; j < genome.size(); j++) {
      EXPECT_GE(genome(j), -5.28);
      EXPECT_LE(genome(j), 5.28);
    }
  }
}

template<floating_point_type F>
class benchmark_function_evaluation : public base_evaluation<F> {
public:
  size_t operator()(base_individual<F>& individual, size_t objective_index) const override
  {
    for (auto& f : _f_ptr) {
      individual.set_objective_fitness(objective_index, std::abs(f(individual.genome())));
      objective_index++;
    }
    return objective_index;
  }

  [[nodiscard]] size_t objective_count() const override
  {
    return _f_ptr.size();
  }

  explicit benchmark_function_evaluation(const vector<F (*)(const Eigen::VectorX<F>&)>& f_ptr)
          :_f_ptr(f_ptr) { }

private:
  vector<F (*)(const Eigen::VectorX<F>&)> _f_ptr;
};

TYPED_TEST(minimacore_genetic_algorithm_tests, benchmark_function_evaluation)
{
  benchmark_function_evaluation<TypeParam> evaluation(this->_functions);
  for (auto& individual : this->_population) {
    EXPECT_EQ(evaluation(*individual, 0), this->_functions.size());
    EXPECT_FALSE(individual->overall_fitness() < 1E-6);
    EXPECT_TRUE(individual->is_valid());
    individual->set_objective_fitness(0, NAN);
    EXPECT_FALSE(individual->is_valid());
  }
}

TYPED_TEST(minimacore_genetic_algorithm_tests, best_fitness_request)
{
  best_fitness_request<TypeParam> calculator;
  ASSERT_NEAR(calculator(this->_population), .7, 1E-6);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, average_fitness_request)
{
  average_fitness_request<TypeParam> calculator;
  ASSERT_NEAR(calculator(this->_population), 2.08, 1E-6);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, selection_pressure_request)
{
  selection_pressure_request<TypeParam> calculator;
  ASSERT_NEAR(calculator(this->_population), 3.36538462E-1, 1E-6);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, evolution_statistics)
{
  ASSERT_EQ(this->_statistics.current_generation(), 0);
  this->_statistics.register_statistic(this->_population);
  {
    auto best_fitness_stat = this->_statistics[(int) statistics_requests_factory<TypeParam>::stat_requests::best_fitness_stat];
    EXPECT_NEAR(best_fitness_stat(this->_statistics.current_generation() - 1, 0), 0.7, 1E-6);
    EXPECT_EQ(best_fitness_stat.rows(), 1);
  }
  ASSERT_EQ(this->_statistics.current_generation(), 1);
  this->_statistics.register_statistic(this->_population);
  {
    auto best_fitness_stat = this->_statistics[(int) statistics_requests_factory<TypeParam>::stat_requests::best_fitness_stat];
    EXPECT_NEAR(best_fitness_stat(this->_statistics.current_generation() - 1, 0), 0.7, 1E-6);
    EXPECT_EQ(best_fitness_stat.rows(), 2);
  }
}

TYPED_TEST(minimacore_genetic_algorithm_tests, generation_termination)
{
  this->_statistics.register_statistic(this->_population);
  {
    generation_termination<TypeParam> termination(2);
    ASSERT_FALSE(termination(this->_statistics));
  }
  this->_statistics.register_statistic(this->_population);
  {
    generation_termination<TypeParam> termination(2);
    ASSERT_TRUE(termination(this->_statistics));
  }
}

TYPED_TEST(minimacore_genetic_algorithm_tests, best_fitness_termination_positive)
{
  this->_statistics.register_statistic(this->_population);
  best_fitness_termination<TypeParam> termination(1.);
  ASSERT_TRUE(termination(this->_statistics));
}

TYPED_TEST(minimacore_genetic_algorithm_tests, best_fitness_termination_negative)
{
  this->_statistics.register_statistic(this->_population);
  best_fitness_termination<TypeParam> termination(0.);
  ASSERT_FALSE(termination(this->_statistics));
}

TYPED_TEST(minimacore_genetic_algorithm_tests, average_fitness_termination_positive)
{
  this->_statistics.register_statistic(this->_population);
  average_fitness_termination<TypeParam> termination(2.1);
  ASSERT_TRUE(termination(this->_statistics));
}

TYPED_TEST(minimacore_genetic_algorithm_tests, average_fitness_termination_negative)
{
  this->_statistics.register_statistic(this->_population);
  average_fitness_termination<TypeParam> termination(0.);
  ASSERT_FALSE(termination(this->_statistics));
}

TYPED_TEST(minimacore_genetic_algorithm_tests, selection_pressure_termination_positive)
{
  this->_statistics.register_statistic(this->_population);
  selection_pressure_termination<TypeParam> termination(0.1);
  ASSERT_TRUE(termination(this->_statistics))
                      << "Selection pressure: " << selection_pressure_request<TypeParam>{}(this->_population)
                      << "\nAverage fitness: " << average_fitness_request<TypeParam>{}(this->_population)
                      << "\nBest fitness: " << best_fitness_request<TypeParam>{}(this->_population)
                      << "\nStatistics Selection pressure: "
                      << this->_statistics.current_value(
                              (int) statistics_requests_factory<TypeParam>::stat_requests::selection_pressure_stat
                      );
}

TYPED_TEST(minimacore_genetic_algorithm_tests, selection_pressure_termination_negative)
{
  this->_statistics.register_statistic(this->_population);
  selection_pressure_termination<TypeParam> termination(1.);
  ASSERT_FALSE(termination(this->_statistics));
}

template<floating_point_type F>
class sphere_evaluation_function : public base_evaluation<F> {
public:
  size_t operator()(base_individual<F>& individual, size_t objective_index) const override
  {
    individual.set_objective_fitness(objective_index, std::abs(sphere(individual.genome())));
    return ++objective_index;
  }

  [[nodiscard]] size_t objective_count() const override
  {
    return 1;
  }

  explicit sphere_evaluation_function() = default;

};

TYPED_TEST(minimacore_genetic_algorithm_tests, setup_run)
{
  vector f = this->_functions;
  Eigen::VectorX<TypeParam> initial_genome = Eigen::VectorX<TypeParam>::Constant(3, 5.);
  auto genome_gen = std::make_unique<genome_generator<TypeParam>>(initial_genome);
  genome_gen->append_chromosome_generator(std::make_unique<chromosome_generator_impl<TypeParam>>(-5., 5.));
  setup<TypeParam> s;
  s.set_population_size(10)
          .set_generations(20)
          .set_selection_for_reproduction(std::make_unique<truncation_selection_for_reproduction<TypeParam>>(4))
          .set_selection_for_replacement(std::make_unique<truncation_selection_for_replacement<TypeParam>>(6))
          .set_crossover(std::make_unique<uniform_linear_crossover<TypeParam>>(1.))
          .set_mutation(std::make_unique<uniform_mutation<TypeParam>>(.05, 1.))
          .set_genome_generator(std::move(genome_gen))
          .add_evaluation(std::make_unique<sphere_evaluation_function<TypeParam>>());
  runner<TypeParam> r(std::move(s));
  r.add_log_stream(std::cout);
  ASSERT_EQ(r.run(), runner<TypeParam>::success);
  ASSERT_LT(r.get_best_individual()->overall_fitness(), r.get_individual_zero()->overall_fitness());
  r.export_statistics("statistics.csv", ',');
}

template<floating_point_type F>
class basic_wait_function : public base_evaluation<F> {
public:
  size_t operator()(base_individual<F>& individual, size_t objective_index) const override
  {
    individual.set_objective_fitness(objective_index, 1.);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return ++objective_index;
  }

  [[nodiscard]] size_t objective_count() const override
  {
    return 1;
  }

  explicit basic_wait_function() = default;
};

TYPED_TEST(minimacore_genetic_algorithm_tests, setup_run_pause_resume)
{
  vector f = this->_functions;
  Eigen::VectorX<TypeParam> initial_genome = Eigen::VectorX<TypeParam>::Constant(3, 5.);
  auto genome_gen = std::make_unique<genome_generator<TypeParam>>(initial_genome);
  genome_gen->append_chromosome_generator(std::make_unique<chromosome_generator_impl<TypeParam>>(-5., 5.));
  setup<TypeParam> s;
  s.set_population_size(10)
          .set_generations(20)
          .set_selection_for_reproduction(std::make_unique<truncation_selection_for_reproduction<TypeParam>>(4))
          .set_selection_for_replacement(std::make_unique<truncation_selection_for_replacement<TypeParam>>(6))
          .set_crossover(std::make_unique<uniform_linear_crossover<TypeParam>>(1.))
          .set_mutation(std::make_unique<uniform_mutation<TypeParam>>(.05, 1.))
          .set_genome_generator(std::move(genome_gen))
          .add_evaluation(std::make_unique<basic_wait_function<TypeParam>>());
  runner<TypeParam> r(std::move(s));
  r.add_log_stream(std::cout);
  auto fut = std::async(&runner<TypeParam>::run, &r);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  r.pause();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  r.resume();
  ASSERT_EQ(fut.get(), runner<TypeParam>::success);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, setup_run_pause_stop)
{
  vector f = this->_functions;
  Eigen::VectorX<TypeParam> initial_genome = Eigen::VectorX<TypeParam>::Constant(3, 5.);
  auto genome_gen = std::make_unique<genome_generator<TypeParam>>(initial_genome);
  genome_gen->append_chromosome_generator(std::make_unique<chromosome_generator_impl<TypeParam>>(-5., 5.));
  setup<TypeParam> s;
  s.set_population_size(10)
          .set_generations(20)
          .set_selection_for_reproduction(std::make_unique<truncation_selection_for_reproduction<TypeParam>>(4))
          .set_selection_for_replacement(std::make_unique<truncation_selection_for_replacement<TypeParam>>(6))
          .set_crossover(std::make_unique<uniform_linear_crossover<TypeParam>>(1.))
          .set_mutation(std::make_unique<uniform_mutation<TypeParam>>(.05, 1.))
          .set_genome_generator(std::move(genome_gen))
          .add_evaluation(std::make_unique<basic_wait_function<TypeParam>>());
  runner<TypeParam> r(std::move(s));
  r.add_log_stream(std::cout);
  auto fut = std::async(&runner<TypeParam>::run, &r);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  r.pause();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  r.stop();
  ASSERT_EQ(fut.get(), runner<TypeParam>::success);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, setup_run_stop)
{
  vector f = this->_functions;
  Eigen::VectorX<TypeParam> initial_genome = Eigen::VectorX<TypeParam>::Constant(3, 5.);
  auto genome_gen = std::make_unique<genome_generator<TypeParam>>(initial_genome);
  genome_gen->append_chromosome_generator(std::make_unique<chromosome_generator_impl<TypeParam>>(-5., 5.));
  setup<TypeParam> s;
  s.set_population_size(10)
          .set_generations(20)
          .set_selection_for_reproduction(std::make_unique<truncation_selection_for_reproduction<TypeParam>>(4))
          .set_selection_for_replacement(std::make_unique<truncation_selection_for_replacement<TypeParam>>(6))
          .set_crossover(std::make_unique<uniform_linear_crossover<TypeParam>>(1.))
          .set_mutation(std::make_unique<uniform_mutation<TypeParam>>(.05, 1.))
          .set_genome_generator(std::move(genome_gen))
          .add_evaluation(std::make_unique<basic_wait_function<TypeParam>>());
  runner<TypeParam> r(std::move(s));
  r.add_log_stream(std::cout);
  auto fut = std::async(&runner<TypeParam>::run, &r);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  r.stop();
  ASSERT_EQ(fut.get(), runner<TypeParam>::success);
}

template<floating_point_type F>
class population_initialization_fail_mock : public base_evaluation<F> {
  const size_t _max_failures = 0;
  mutable std::atomic<size_t> _fail_count{0};
public:
  size_t operator()(base_individual<F>& individual, size_t objective_index) const override
  {
    if (_fail_count.load() < _max_failures) {
      individual.set_objective_fitness(objective_index, NAN);
      _fail_count++;
    }
    else {
      individual.set_objective_fitness(objective_index, 1.);
    }
    return ++objective_index;
  }

  [[nodiscard]] size_t objective_count() const override
  {
    return 1;
  }

  explicit population_initialization_fail_mock(size_t max_failures)
          :_max_failures(max_failures) { }
};

TYPED_TEST(minimacore_genetic_algorithm_tests, exit_on_population_initialization_failure_normal)
{
  vector f = this->_functions;
  Eigen::VectorX<TypeParam> initial_genome = Eigen::VectorX<TypeParam>::Constant(3, 5.);
  auto genome_gen = std::make_unique<genome_generator<TypeParam>>(initial_genome);
  genome_gen->append_chromosome_generator(std::make_unique<chromosome_generator_impl<TypeParam>>(-5., 5.));
  setup<TypeParam> s;
  s.set_population_size(10)
          .set_generations(20)
          .set_selection_for_reproduction(std::make_unique<truncation_selection_for_reproduction<TypeParam>>(4))
          .set_selection_for_replacement(std::make_unique<truncation_selection_for_replacement<TypeParam>>(6))
          .set_crossover(std::make_unique<uniform_linear_crossover<TypeParam>>(1.))
          .set_mutation(std::make_unique<uniform_mutation<TypeParam>>(.05, 1.))
          .set_genome_generator(std::move(genome_gen))
          .add_evaluation(std::make_unique<population_initialization_fail_mock<TypeParam>>(290U));
  runner<TypeParam> r(std::move(s));
  r.add_log_stream(std::cout);
  auto fut = std::async(&runner<TypeParam>::run, &r);
  ASSERT_EQ(fut.get(), runner<TypeParam>::success);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, exit_on_population_initialization_failure_fail)
{
  vector f = this->_functions;
  Eigen::VectorX<TypeParam> initial_genome = Eigen::VectorX<TypeParam>::Constant(3, 5.);
  auto genome_gen = std::make_unique<genome_generator<TypeParam>>(initial_genome);
  genome_gen->append_chromosome_generator(std::make_unique<chromosome_generator_impl<TypeParam>>(-5., 5.));
  setup<TypeParam> s;
  s.set_population_size(10)
          .set_generations(20)
          .set_selection_for_reproduction(std::make_unique<truncation_selection_for_reproduction<TypeParam>>(4))
          .set_selection_for_replacement(std::make_unique<truncation_selection_for_replacement<TypeParam>>(6))
          .set_crossover(std::make_unique<uniform_linear_crossover<TypeParam>>(1.))
          .set_mutation(std::make_unique<uniform_mutation<TypeParam>>(.05, 1.))
          .set_genome_generator(std::move(genome_gen))
          // 20 * 301U guarantees that at least one individual fails 301 times.
          // Since the algorithm can run in parallel,
          .add_evaluation(std::make_unique<population_initialization_fail_mock<TypeParam>>(20*301U));
  runner<TypeParam> r(std::move(s));
  r.add_log_stream(std::cout);
  auto fut = std::async(&runner<TypeParam>::run, &r);
  ASSERT_EQ(fut.get(), runner<TypeParam>::failure);
}

TYPED_TEST(minimacore_genetic_algorithm_tests, iteration_callback_count)
{
  vector f = this->_functions;
  Eigen::VectorX<TypeParam> initial_genome = Eigen::VectorX<TypeParam>::Constant(3, 5.);
  auto genome_gen = std::make_unique<genome_generator<TypeParam>>(initial_genome);
  genome_gen->append_chromosome_generator(std::make_unique<chromosome_generator_impl<TypeParam>>(-5., 5.));
  setup<TypeParam> s{};
  std::atomic<size_t> callback_count = 0;
  s.set_population_size(10)
          .set_generations(20)
          .set_selection_for_reproduction(std::make_unique<truncation_selection_for_reproduction<TypeParam>>(4))
          .set_selection_for_replacement(std::make_unique<truncation_selection_for_replacement<TypeParam>>(6))
          .set_crossover(std::make_unique<uniform_linear_crossover<TypeParam>>(1.))
          .set_mutation(std::make_unique<uniform_mutation<TypeParam>>(.05, 1.))
          .set_genome_generator(std::move(genome_gen))
          .add_evaluation(std::make_unique<sphere_evaluation_function<TypeParam>>())
          .add_callback([&callback_count]() { callback_count++; });
  runner<TypeParam> r(std::move(s));
  r.add_log_stream(std::cout);
  auto fut = std::async(&runner<TypeParam>::run, &r);
  ASSERT_EQ(fut.get(), runner<TypeParam>::exit_flag::success);
  ASSERT_EQ(callback_count, r.get_setup().generations());
}


