
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

public:
  enum exit_code {
    successful_exit = 0,
    failed_exit
  };
  
  void initialize_population()
  {
  
  }
  
  exit_code run()
  {
    initialize_population();
    _statistics->register_statistic(_population);
    while (std::none_of(std::execution::par_unseq,
                        _termination_conditions.begin(),
                        _termination_conditions.end(),
                        [this](auto& condition) { return (*condition)(_statistics); })) {
      _statistics->register_statistic(_population);
    }
  }


private:
  selection_for_replacement_ptr _selection_for_replacement;
  selection_for_reproduction_ptr _selection_for_reproduction;
  crossover_ptr _crossover;
  mutation_ptr _mutation;
  population_generator_ptr _population_generator;
  population_t<F> _population;
  evolution_statistics<F> _statistics;
  vector<termination_condition_ptr<F>> _termination_conditions;
};

}
#endif //MINIMACORE_GENETIC_ALGORITHM_BASE_H
