
#ifndef MINIMACORE_GENETIC_ALGORITHM_BASE_H
#define MINIMACORE_GENETIC_ALGORITHM_BASE_H

#include "selection_operators.h"
#include "genetic_operators.h"
#include "base_evaluation.h"
#include "base_individual_generator.h"
#include "termination_condition.h"
#include "setup.h"

#include <minimacore_concepts.h>

namespace minimacore::genetic_algorithm {

template<floating_point_type F>
class runner {
  using selection_for_replacement_ptr = unique_ptr<base_selection_for_replacement<F>>;
  using selection_for_reproduction_ptr = unique_ptr<base_selection_for_reproduction<F>>;
  using mutation_ptr = unique_ptr<mutation<F>>;
  using crossover_ptr = unique_ptr<base_crossover<F>>;
  using population_generator_ptr = unique_ptr<genome_generator<F>>;
  using evaluation_t = unique_ptr<base_evaluation<F>>;
  using evaluations_t = vector<evaluation_t>;

public:
  enum exit_code {
    successful_exit = 0,
    failed_exit
  };
  
  exit_code run()
  {
    std::cout << "initializing population\n";
    initialize_population();
    _statistics.register_statistic(_population);
    std::cout << "current generation: " << _statistics.current_generation() << '\n';
    std::cout << "current fitness: " << _statistics.current_value(statistics_requests_factory<F>::best_fitness_stat)
              << '\n';
    std::cout << "average fitness: " << _statistics.current_value(statistics_requests_factory<F>::average_fitness_stat)
              << '\n';
    while (std::none_of(std::execution::par_unseq,
                        _setup->termination_conditions().begin(),
                        _setup->termination_conditions().end(),
                        [this](auto& condition) { return (*condition)(_statistics); })) {
      std::cout << "selecting reproduction set...\n";
      auto reproduction_set = _setup->selection_for_reproduction()(_population);
      std::cout << "selecting replacement set...\n";
      _setup->selection_for_replacement()(_population);
      fill_population(reproduction_set);
      _statistics.register_statistic(_population);
      std::cout << "current generation: " << _statistics.current_generation() << '\n';
      std::cout << "current fitness: " << _statistics.current_value(statistics_requests_factory<F>::best_fitness_stat)
                << '\n';
      std::cout << "average fitness: "
                << _statistics.current_value(statistics_requests_factory<F>::average_fitness_stat)
                << '\n';
    }
    return successful_exit;
  }
  
  const shared_ptr<base_individual<F>>& get_best_individual() const
  {
    return _best_individual;
  }
  
  explicit runner(setup<F>* setup)
      : _setup(setup),
        _statistics(setup->generations())
  {}

private:
  [[nodiscard]] size_t objective_count() const
  {
    return std::accumulate(
        _setup->evaluations().begin(),
        _setup->evaluations().end(),
        0,
        [](size_t i, const evaluation_t& eval) { return i + eval->objective_count(); }
    );
  }
  
  void evaluate(const individual_ptr<F>& individual)
  {
    while (!individual->is_valid()) {
      size_t dummy = 0;
      for (auto& evaluation : _setup->evaluations()) dummy = (*evaluation)(*individual, dummy);
    }
  }
  
  void initialize_population()
  {
    while (_population.size() < _setup->population_size()) {
      auto& individual = _population.emplace_back(
          std::make_shared<base_individual<F>>(
              _setup->get_genome_generator().initial_genome(),
              objective_count()
          )
      );
      _setup->get_genome_generator()(individual);
      evaluate(individual);
    }
  }
  
  void fill_population(population_t<F>& reproduction_set)
  {
    std::random_device device{};
    std::mt19937_64 generator{device()};
    std::uniform_real_distribution<F> distribution(0., 1.);
    while (_population.size() < _setup->population_size()) {
      F chance = distribution(generator);
      individual_ptr<F> individual;
      if (_setup->get_mutation().should_mutate()) {
        individual = std::make_shared<base_individual<F>>(
            _setup->get_mutation()(*random_pick(reproduction_set)),
            objective_count());
      } else {
        individual = std::make_shared<base_individual<F>>(
            _setup->crossover()(*random_pick(reproduction_set), *random_pick(reproduction_set)),
            objective_count());
      }
      evaluate(individual);
      _population.push_back(individual);
    }
  }
  
  population_t<F> _population;
  individual_ptr<F> _best_individual;
  evolution_statistics<F> _statistics;
  setup<F>* _setup;
};
  
}
#endif //MINIMACORE_GENETIC_ALGORITHM_BASE_H
