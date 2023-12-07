
#ifndef MINIMACORE_TERMINATION_CONDITION_H
#define MINIMACORE_TERMINATION_CONDITION_H

#include "evolution_statistics.h"

namespace minimacore::genetic_algorithm {

template<floating_point_type F>
class termination_condition_base {
public:
  virtual bool operator()(const evolution_statistics<F>& statistics) const = 0;
};

template<floating_point_type F>
using termination_condition_ptr = unique_ptr<termination_condition_base<F>>;

template<floating_point_type F>
class generation_termination : public termination_condition_base<F> {
public:
  bool operator()(const evolution_statistics<F>& statistics) const override
  {
    return statistics.current_generation() >= _maximum_generations;
  }
  
  explicit generation_termination(size_t maximum_generations) : _maximum_generations(maximum_generations)
  {}

private:
  size_t _maximum_generations{0};
};

template<floating_point_type F>
class average_fitness_termination : public termination_condition_base<F> {
public:
  bool operator()(const evolution_statistics<F>& statistics) const override
  {
    return statistics.current_value((size_t) statistics_requests_factory<F>::best_fitness_stat) < _max_avg_fitness;
  }
  
  explicit average_fitness_termination(F max_avg_fitness) : _max_avg_fitness(max_avg_fitness)
  {}

private:
  F _max_avg_fitness{0.};
};

template<floating_point_type F>
class best_fitness_termination : public termination_condition_base<F> {
public:
  bool operator()(const evolution_statistics<F>& statistics) const override
  {
    return statistics.current_value((size_t) statistics_requests_factory<F>::best_fitness_stat) < _max_best_fitness;
  }
  
  explicit best_fitness_termination(F max_best_fitness) : _max_best_fitness(max_best_fitness)
  {}

private:
  F _max_best_fitness{0.};
};

template<floating_point_type F>
class selection_pressure_termination : public termination_condition_base<F> {
public:
  bool operator()(const evolution_statistics<F>& statistics) const override
  {
    return statistics.current_value((size_t) statistics_requests_factory<F>::selection_pressure_stat) >
           _min_selection_pressure;
  }
  
  explicit selection_pressure_termination(F min_selection_pressure) : _min_selection_pressure(min_selection_pressure)
  {}

private:
  F _min_selection_pressure{0.};
};


}
#endif //MINIMACORE_TERMINATION_CONDITION_H
