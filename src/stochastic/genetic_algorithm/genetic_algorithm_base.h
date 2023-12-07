
#ifndef MINIMACORE_GENETIC_ALGORITHM_BASE_H
#define MINIMACORE_GENETIC_ALGORITHM_BASE_H

#include "selection_operators.h"
#include "genetic_operators.h"
#include "base_evaluation.h"
#include "base_individual_generator.h"

#include <minimacore_concepts.h>

namespace minimacore::genetic_algorithm {

template<floating_point_type F>
class setup {
  using selection_for_replacement_ptr = unique_ptr<base_selection_for_replacement<F>>;
  using selection_for_reproduction_ptr = unique_ptr<base_selection_for_reproduction<F>>;
  using mutation_ptr = unique_ptr<mutation<F>>;
  using crossover_ptr = unique_ptr<base_crossover<F>>;
  using population_generator_ptr = unique_ptr<population_generator<F>>;
public:
  
  enum exit_code {
    successful_exit = 0,
    failed_exit
  };
  
  exit_code run()
  {
  
  }

private:
  selection_for_replacement_ptr _selection_for_replacement;
  selection_for_reproduction_ptr _selection_for_reproduction;
  crossover_ptr _crossover;
  mutation_ptr _mutation;
  population_generator_ptr _population_generator;
  population_t<F> _population;
};

}
#endif //MINIMACORE_GENETIC_ALGORITHM_BASE_H
