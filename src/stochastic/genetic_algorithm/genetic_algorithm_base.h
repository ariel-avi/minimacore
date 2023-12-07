
#ifndef MINIMACORE_GENETIC_ALGORITHM_BASE_H
#define MINIMACORE_GENETIC_ALGORITHM_BASE_H

#include "selection_operators.h"
#include "genetic_operators.h"
#include "base_evaluation.h"
#include "base_individual_generator.h"
#include "termination_condition.h"

#include <minimacore_concepts.h>

namespace minimacore::genetic_algorithm {

template<floating_point_type F>
class setup {
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
    initialize_population();
    _statistics->register_statistic(_population);
    while (std::none_of(std::execution::par_unseq,
                        _termination_conditions.begin(),
                        _termination_conditions.end(),
                        [this](auto& condition) { return (*condition)(_statistics); })) {
      _statistics->register_statistic(_population);
      auto reproduction_set = (*_selection_for_reproduction)(_population);
      (*_selection_for_replacement)(_population);
      fill_population();
    }
  }

private:
  [[nodiscard]] size_t objective_count() const
  {
    return std::accumulate(
        _evaluations.begin(), _evaluations.end(), 0,
        [](size_t i, const evaluation_t& eval) { return i + eval->objective_count(); }
    );
  }
  
  void evaluate(const individual_ptr<F>& individual)
  {
    while (!individual->is_valid()) {
      size_t dummy = 0;
      for (auto& evaluation : _evaluations) dummy = (*evaluation)(*individual, dummy);
    }
  }
  
  void initialize_population()
  {
    while (_population.size() < _population_size) {
      auto& individual = _population.emplace_back(
          std::make_shared<base_individual<F>>(
              _genome_generator.initial_genome(),
              objective_count()
          )
      );
      evaluate(individual);
    }
  }
  
  void fill_population(population_t<F>& reproduction_set)
  {
    while (_population.size() < _population_size) {
      auto individual = (*_crossover)(reproduction_set);
      evaluate(individual);
      _population.push_back(individual);
    }
  }
  
  size_t _population_size;
  population_t<F> _population;
  selection_for_replacement_ptr _selection_for_replacement;
  selection_for_reproduction_ptr _selection_for_reproduction;
  crossover_ptr _crossover;
  mutation_ptr _mutation;
  population_generator_ptr _population_generator;
  evolution_statistics<F> _statistics;
  vector<termination_condition_ptr<F>> _termination_conditions;
  genome_generator<F> _genome_generator;
  evaluations_t _evaluations;
};

}
#endif //MINIMACORE_GENETIC_ALGORITHM_BASE_H
